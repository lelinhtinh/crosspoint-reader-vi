#include "OtaUpdater.h"

#include <ArduinoJson.h>

#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_wifi.h"

namespace {
constexpr char latestReleaseUrl[] = "https://api.github.com/repos/lelinhtinh/crosspoint-reader-vi/releases/latest";

/* This is buffer and size holder to keep upcoming data from latestReleaseUrl */
char *local_buf;
int output_len;

/*
 * When esp_crt_bundle.h included, it is pointing wrong header file
 * which is something under WifiClientSecure because of our framework based on arduno platform.
 * To manage this obstacle, don't include anything, just extern and it will point correct one.
 */
extern "C" {
extern esp_err_t esp_crt_bundle_attach(void *conf);
}

esp_err_t http_client_set_header_cb(esp_http_client_handle_t http_client) {
  return esp_http_client_set_header(http_client, "User-Agent", "CrossPoint-ESP32-" CROSSPOINT_VERSION);
}

esp_err_t event_handler(esp_http_client_event_t *event) {
  /* We do interested in only HTTP_EVENT_ON_DATA event only */
  if (event->event_id != HTTP_EVENT_ON_DATA)
    return ESP_OK;

  if (!esp_http_client_is_chunked_response(event->client)) {
    int content_len = esp_http_client_get_content_length(event->client);
    int copy_len = 0;

    if (local_buf == NULL) {
      /* local_buf life span is tracked by caller checkForUpdate */
      local_buf = static_cast<char *>(calloc(content_len + 1, sizeof(char)));
      output_len = 0;
      if (local_buf == NULL) {
        Serial.printf("[%lu] [OTA] HTTP Client Out of Memory Failed, Allocation %d\n", millis(), content_len);
        return ESP_ERR_NO_MEM;
      }
    }
    copy_len = min(event->data_len, (content_len - output_len));
    if (copy_len) {
      memcpy(local_buf + output_len, event->data, copy_len);
    }
    output_len += copy_len;
  } else {
    /* Code might be hits here, It happened once (for version checking) but I need more logs to handle that */
    int chunked_len;
    esp_http_client_get_chunk_length(event->client, &chunked_len);
    Serial.printf("[%lu] [OTA] esp_http_client_is_chunked_response failed, chunked_len: %d\n", millis(), chunked_len);
  }

  return ESP_OK;
} /* event_handler */
} /* namespace */

// helper forward declaration
static void parseSemver(const std::string &ver, int &major, int &minor, int &patch);

OtaUpdater::OtaUpdaterError OtaUpdater::checkForUpdate() {
  JsonDocument filter;
  esp_err_t esp_err;
  JsonDocument doc;

  esp_http_client_config_t client_config = {
      .url = latestReleaseUrl,
      .event_handler = event_handler,
      /* Default HTTP client buffer size 512 byte only */
      .buffer_size = 8192,
      .buffer_size_tx = 8192,
      .skip_cert_common_name_check = true,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .keep_alive_enable = true,
  };

  /* To track life time of local_buf, dtor will be called on exit from that function */
  struct localBufCleaner {
    char **bufPtr;
    ~localBufCleaner() {
      if (*bufPtr) {
        free(*bufPtr);
        *bufPtr = NULL;
      }
    }
  } localBufCleaner = {&local_buf};

  esp_http_client_handle_t client_handle = esp_http_client_init(&client_config);
  if (!client_handle) {
    Serial.printf("[%lu] [OTA] HTTP Client Handle Failed\n", millis());
    return INTERNAL_UPDATE_ERROR;
  }

  esp_err = esp_http_client_set_header(client_handle, "User-Agent", "CrossPoint-ESP32-" CROSSPOINT_VERSION);
  if (esp_err != ESP_OK) {
    Serial.printf("[%lu] [OTA] esp_http_client_set_header Failed : %s\n", millis(), esp_err_to_name(esp_err));
    esp_http_client_cleanup(client_handle);
    return INTERNAL_UPDATE_ERROR;
  }

  esp_err = esp_http_client_perform(client_handle);
  if (esp_err != ESP_OK) {
    Serial.printf("[%lu] [OTA] esp_http_client_perform Failed : %s\n", millis(), esp_err_to_name(esp_err));
    esp_http_client_cleanup(client_handle);
    return HTTP_ERROR;
  }

  /* esp_http_client_close will be called inside cleanup as well*/
  esp_err = esp_http_client_cleanup(client_handle);
  if (esp_err != ESP_OK) {
    Serial.printf("[%lu] [OTA] esp_http_client_cleanupp Failed : %s\n", millis(), esp_err_to_name(esp_err));
    return INTERNAL_UPDATE_ERROR;
  }

  filter["tag_name"] = true;
  filter["assets"][0]["name"] = true;
  filter["assets"][0]["browser_download_url"] = true;
  filter["assets"][0]["size"] = true;
  const DeserializationError error = deserializeJson(doc, local_buf, DeserializationOption::Filter(filter));
  if (error) {
    Serial.printf("[%lu] [OTA] JSON parse failed: %s\n", millis(), error.c_str());
    return JSON_PARSE_ERROR;
  }

  if (!doc["tag_name"].is<std::string>()) {
    Serial.printf("[%lu] [OTA] No tag_name found\n", millis());
    return JSON_PARSE_ERROR;
  }

  if (!doc["assets"].is<JsonArray>()) {
    Serial.printf("[%lu] [OTA] No assets found\n", millis());
    return JSON_PARSE_ERROR;
  }

  latestVersion = doc["tag_name"].as<std::string>();

  bool foundFirmware = false;
  for (int i = 0; i < doc["assets"].size(); i++) {
    if (doc["assets"][i]["name"] == "firmware.bin") {
      otaUrl = doc["assets"][i]["browser_download_url"].as<std::string>();
      otaSize = doc["assets"][i]["size"].as<size_t>();
      totalSize = otaSize;
      foundFirmware = true;
      break;
    }
  }

  if (!foundFirmware) {
    Serial.printf("[%lu] [OTA] No firmware.bin asset found\n", millis());
    return NO_UPDATE;
  }

  // Parse semver for debug and to determine if it's actually newer
  int latestMajor = 0, latestMinor = 0, latestPatch = 0;
  int currentMajor = 0, currentMinor = 0, currentPatch = 0;
  parseSemver(latestVersion, latestMajor, latestMinor, latestPatch);
  parseSemver(std::string(CROSSPOINT_VERSION), currentMajor, currentMinor, currentPatch);

  Serial.printf("[%lu] [OTA] Found release: %s (parsed %d.%d.%d) | Current: %s (parsed %d.%d.%d)\n",
                millis(), latestVersion.c_str(), latestMajor, latestMinor, latestPatch, CROSSPOINT_VERSION,
                currentMajor, currentMinor, currentPatch);

  // Only mark updateAvailable if the remote version is actually newer
  if (latestMajor == 0 && latestMinor == 0 && latestPatch == 0 && currentMajor == 0 && currentMinor == 0 &&
      currentPatch == 0) {
    // fallback to string comparison
    updateAvailable = latestVersion > std::string(CROSSPOINT_VERSION);
  } else {
    if (latestMajor != currentMajor)
      updateAvailable = latestMajor > currentMajor;
    else if (latestMinor != currentMinor)
      updateAvailable = latestMinor > currentMinor;
    else
      updateAvailable = latestPatch > currentPatch;
  }

  if (!updateAvailable) {
    Serial.printf("[%lu] [OTA] Latest release is not newer; skipping update\n", millis());
    return NO_UPDATE;
  }

  Serial.printf("[%lu] [OTA] Update available: %s (size %zu)\n", millis(), latestVersion.c_str(), otaSize);
  return OK;
}

static void parseSemver(const std::string &ver, int &major, int &minor, int &patch) {
  // Initialize defaults
  major = minor = patch = 0;
  size_t i = 0;

  // Skip any leading non-digit characters (e.g., 'v')
  while (i < ver.size() && !isdigit(static_cast<unsigned char>(ver[i])))
    ++i;

  // Helper to parse next integer starting at i
  auto parseInt = [&](int &out) {
    out = 0;
    bool found = false;
    while (i < ver.size() && isdigit(static_cast<unsigned char>(ver[i]))) {
      found = true;
      out = out * 10 + (ver[i] - '0');
      ++i;
    }
    return found;
  };

  // Parse major
  parseInt(major);
  // Skip non-digit separators
  while (i < ver.size() && !isdigit(static_cast<unsigned char>(ver[i])))
    ++i;
  // Parse minor
  parseInt(minor);
  // Skip non-digit separators
  while (i < ver.size() && !isdigit(static_cast<unsigned char>(ver[i])))
    ++i;
  // Parse patch
  parseInt(patch);
}

bool OtaUpdater::isUpdateNewer() const {
  if (latestVersion.empty()) {
    return false;
  }

  // If versions are identical string-wise, no update
  if (latestVersion == CROSSPOINT_VERSION) {
    return false;
  }

  int currentMajor = 0, currentMinor = 0, currentPatch = 0;
  int latestMajor = 0, latestMinor = 0, latestPatch = 0;

  parseSemver(latestVersion, latestMajor, latestMinor, latestPatch);
  parseSemver(std::string(CROSSPOINT_VERSION), currentMajor, currentMinor, currentPatch);

  Serial.printf("[%lu] [OTA] Comparing parsed: latest=%d.%d.%d current=%d.%d.%d\n", millis(), latestMajor,
                latestMinor, latestPatch, currentMajor, currentMinor, currentPatch);

  // If latest parsed to all zeros and equals current parsed to all zeros, fall back to string compare
  if (latestMajor == 0 && latestMinor == 0 && latestPatch == 0 && currentMajor == 0 && currentMinor == 0 &&
      currentPatch == 0) {
    // basic string comparison (lexicographic) as last resort
    return latestVersion > std::string(CROSSPOINT_VERSION);
  }

  if (latestMajor != currentMajor)
    return latestMajor > currentMajor;
  if (latestMinor != currentMinor)
    return latestMinor > currentMinor;
  return latestPatch > currentPatch;
}

const std::string &OtaUpdater::getLatestVersion() const { return latestVersion; }

OtaUpdater::OtaUpdaterError OtaUpdater::installUpdate() {
  if (!isUpdateNewer()) {
    return UPDATE_OLDER_ERROR;
  }

  esp_https_ota_handle_t ota_handle = NULL;
  esp_err_t esp_err;
  /* Signal for OtaUpdateActivity */
  render = false;

  esp_http_client_config_t client_config = {
      .url = otaUrl.c_str(),
      .timeout_ms = 15000,
      /* Default HTTP client buffer size 512 byte only
       * not sufficent to handle URL redirection cases or
       * parsing of large HTTP headers.
       */
      .buffer_size = 8192,
      .buffer_size_tx = 8192,
      .skip_cert_common_name_check = true,
      .crt_bundle_attach = esp_crt_bundle_attach,
      .keep_alive_enable = true,
  };

  esp_https_ota_config_t ota_config = {
      .http_config = &client_config,
      .http_client_init_cb = http_client_set_header_cb,
  };

  /* For better timing and connectivity, we disable power saving for WiFi */
  esp_wifi_set_ps(WIFI_PS_NONE);

  esp_err = esp_https_ota_begin(&ota_config, &ota_handle);
  if (esp_err != ESP_OK) {
    Serial.printf("[%lu] [OTA] HTTP OTA Begin Failed: %s\n", millis(), esp_err_to_name(esp_err));
    return INTERNAL_UPDATE_ERROR;
  }

  do {
    esp_err = esp_https_ota_perform(ota_handle);
    processedSize = esp_https_ota_get_image_len_read(ota_handle);
    /* Sent signal to  OtaUpdateActivity */
    render = true;
    vTaskDelay(10 / portTICK_PERIOD_MS);
  } while (esp_err == ESP_ERR_HTTPS_OTA_IN_PROGRESS);

  /* Return back to default power saving for WiFi in case of failing */
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  if (esp_err != ESP_OK) {
    Serial.printf("[%lu] [OTA] esp_https_ota_perform Failed: %s\n", millis(), esp_err_to_name(esp_err));
    esp_https_ota_finish(ota_handle);
    return HTTP_ERROR;
  }

  if (!esp_https_ota_is_complete_data_received(ota_handle)) {
    Serial.printf("[%lu] [OTA] esp_https_ota_is_complete_data_received Failed: %s\n", millis(),
                  esp_err_to_name(esp_err));
    esp_https_ota_finish(ota_handle);
    return INTERNAL_UPDATE_ERROR;
  }

  esp_err = esp_https_ota_finish(ota_handle);
  if (esp_err != ESP_OK) {
    Serial.printf("[%lu] [OTA] esp_https_ota_finish Failed: %s\n", millis(), esp_err_to_name(esp_err));
    return INTERNAL_UPDATE_ERROR;
  }

  Serial.printf("[%lu] [OTA] Update completed\n", millis());
  return OK;
}
