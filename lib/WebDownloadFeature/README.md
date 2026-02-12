# Web Download Feature

HTTP file download endpoint feature for CrossPoint Reader Vietnamese fork web server.

## Overview

This feature adds a download button to the web file browser, allowing users to download EPUB files and other content from the device's SD card to their computer via WiFi.

## Features

- **HTTP GET `/download` endpoint** - File download with proper headers
- **Download button UI** - 📥 icon in web file browser
- **Security** - Blocks hidden files and protected systemdirectories
- **EPUB support** - Proper `application/epub+zip` MIME type
- **Generic fallback** - `application/octet-stream` for other files

## Usage

### Accessing Web Interface

1. Connect device to WiFi (Settings → WiFi)
2. Note the device IP address from the screen
3. Open browser: `http://<device-ip>/files`

### Downloading Files

1. Navigate to file browser
2. Click download button (📥) next to any file
3. Browser will download the file

## Build Configuration

This feature is controlled by the `ENABLE_WEB_DOWNLOAD_FEATURE` preprocessor flag:

```ini
# In platformio.ini
build_flags = -DENABLE_WEB_DOWNLOAD_FEATURE=1  # Enabled (default)
build_flags = -DENABLE_WEB_DOWNLOAD_FEATURE=0  # Disabled
```

### When Disabled

- Binary size: -4KB
- `/download` endpoint returns 404
- Download button hidden from web UI
- File listing still works normally

## Implementation Details

### HTTP Endpoint

**Route:** `GET /download?path=<file_path>`

**Headers:**
```
Content-Type: application/epub+zip (for EPUB files)
Content-Type: application/octet-stream (for others)
Content-Disposition: attachment; filename="<filename>"
Content-Length: <file_size>
```

**Response:** File binary data streamed from SD card

### Security

Protected items (cannot download):
- Hidden files (starting with `.`)
- System directories from `HIDDEN_ITEMS` array

### Frontend Integration

**HTML Button:**
```html
<!-- FORK-FEATURE-BEGIN: WEBDOWNLOAD -->
#ifdef ENABLE_WEB_DOWNLOAD_FEATURE
<button onclick="downloadFile(path, name)" title="Download">📥</button>
#endif
<!-- FORK-FEATURE-END: WEBDOWNLOAD -->
```

**JavaScript:**
```javascript
function downloadFile(filePath, fileName) {
  const downloadUrl = `/download?path=${encodeURIComponent(filePath)}`;
  const link = document.createElement('a');
  link.href = downloadUrl;
  link.download = fileName;
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
}
```

## Implementation Files

**Backend** (inline with `#ifdef` guards):
- `src/network/CrossPointWebServer.h` - Method declaration
- `src/network/CrossPointWebServer.cpp` - Endpoint implementation and route registration

**Frontend** (preprocessed by `scripts/build_html.py`):
- `src/network/html/FilesPage.html` - Download button UI and JavaScript

**Build system:**
- `scripts/build_html.py` - HTML preprocessor for conditional UI

## HTML Preprocessing

The build system includes preprocessing to conditionally include/exclude HTML and JavaScript based on feature flags:

```python
# In build_html.py
def preprocess_html(content, defines):
    """Process #ifdef directives in HTML files"""
    # Removes sections not matching enabled features
```

This ensures the download button only appears in the web UI when the feature is enabled at compile time.

## API Documentation

### GET /download

Downloads a file from the SD card.

**Query Parameters:**
- `path` (required): File path on SD card (e.g., `/books/mybook.epub`)

**Success Response (200):**
- Headers: Content-Type, Content-Disposition, Content-Length
- Body: File binary data

**Error Responses:**
- `400 Bad Request`: Missing or invalid path
- `403 Forbidden`: Attempting to access hidden/protected items
- `404 Not Found`: File does not exist
- `500 Internal Server Error`: Failed to open file

**Example:**
```bash
curl -O "http://192.168.1.100/download?path=/books/mybook.epub"
```

## Testing

### Build testing
```bash
# With feature enabled
pio run -e default

# With feature disabled
pio run -e minimal
```

### Runtime testing
1. Connect to device WiFi
2. Open browser to `http://<device-ip>/files`
3. Verify download button (📥) is visible
4. Click download button
5. Verify file downloads successfully
6. Check downloaded file integrity

### Disabled feature testing
```bash
# Build without feature
pio run -e minimal

# Access web UI
# Verify download button is NOT visible
# Verify /download endpoint returns 404
```

## Code Markers

All fork-specific code is marked:
```cpp
// FORK-FEATURE-BEGIN: WEBDOWNLOAD
// ... download-specific code ...
// FORK-FEATURE-END: WEBDOWNLOAD
```

## License

Part of CrossPoint Reader Vietnamese Fork. See main project LICENSE.

## Repository

https://github.com/baivong/crosspoint-reader-vi
