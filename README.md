# CrossPoint Reader (Vietnamese Fork)

Firmware for the **Xteink X4** e-paper display reader (unaffiliated with Xteink).
Built using **PlatformIO** and targeting the **ESP32-C3** microcontroller.

**This is a Vietnamese language fork** of [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader) with:

- 🇻🇳 Full Vietnamese language support (NFC/NFD Unicode normalization)
- Vietnamese-optimized Pridi font family
- Simplified font system for reduced firmware size

![cover](./docs/images/cover.jpg)

## Motivation

E-paper devices are fantastic for reading, but most commercially available readers are closed systems with limited
customisation. The **Xteink X4** is an affordable, e-paper device, however the official firmware remains closed.
CrossPoint exists partly as a fun side-project and partly to open up the ecosystem and truely unlock the device's
potential.

CrossPoint Reader aims to:

- Provide a **fully open-source alternative** to the official firmware.
- Offer a **document reader** capable of handling EPUB content on constrained hardware.
- Support **customisable font, layout, and display** options.
- Run purely on the **Xteink X4 hardware**.

This project is **not affiliated with Xteink**; it's built as a community project.

## Features & Usage

- [x] EPUB parsing and rendering (EPUB 2 and EPUB 3)
- [ ] Image support within EPUB
- [x] Saved reading position
- [x] File explorer with file picker
  - [x] Basic EPUB picker from root directory
  - [x] Support nested folders
  - [ ] EPUB picker with cover art
- [x] Custom sleep screen
  - [x] Cover sleep screen
- [x] Wifi book upload
- [x] Wifi OTA updates
- [x] Configurable font, layout, and display options
  - [ ] User provided fonts
  - [ ] Full UTF support
- [x] Screen rotation

Multi-language support: Read EPUBs in various languages, including English, Spanish, French, German, Italian, Portuguese, Russian, Ukrainian, Polish, Swedish, Norwegian, [and more](./USER_GUIDE.md#supported-languages).

See [the user guide](./USER_GUIDE.md) for instructions on operating CrossPoint.

## Installing

### Web (specific firmware version)

1. Connect your Xteink X4 to your computer via USB-C
2. Download the `firmware.bin` file from the [releases page](https://github.com/lelinhtinh/crosspoint-reader-vi/releases)
3. Go to <https://xteink.dve.al/> and flash the firmware file using the "OTA fast flash controls" section

To revert back to the official firmware, you can flash the latest official firmware from <https://xteink.dve.al/>, or swap
back to the other partition using the "Swap boot partition" button here <https://xteink.dve.al/debug>.

### Manual

See [Development](#development) below.

## Development

### Prerequisites

- **PlatformIO Core** (`pio`) or **VS Code + PlatformIO IDE**
- Python 3.8+
- USB-C cable for flashing the ESP32-C3
- Xteink X4

### Checking out the code

CrossPoint uses PlatformIO for building and flashing the firmware. To get started, clone the repository:

```sh
git clone --recursive https://github.com/lelinhtinh/crosspoint-reader-vi

# Or, if you've already cloned without --recursive:
git submodule update --init --recursive
```

### Flashing your device

Connect your Xteink X4 to your computer via USB-C and run the following command.

```sh
pio run --target upload
```

## Internals

CrossPoint Reader is pretty aggressive about caching data down to the SD card to minimise RAM usage. The ESP32-C3 only
has ~380KB of usable RAM, so we have to be careful. A lot of the decisions made in the design of the firmware were based
on this constraint.

### Data caching

The first time chapters of a book are loaded, they are cached to the SD card. Subsequent loads are served from the
cache. This cache directory exists at `.crosspoint` on the SD card. The structure is as follows:

```tree
.crosspoint/
├── epub_12471232/       # Each EPUB is cached to a subdirectory named `epub_<hash>`
│   ├── progress.bin     # Stores reading progress (chapter, page, etc.)
│   ├── cover.bmp        # Book cover image (once generated)
│   ├── book.bin         # Book metadata (title, author, spine, table of contents, etc.)
│   └── sections/        # All chapter data is stored in the sections subdirectory
│       ├── 0.bin        # Chapter data (screen count, all text layout info, etc.)
│       ├── 1.bin        #     files are named by their index in the spine
│       └── ...
│
└── epub_189013891/
```

Deleting the `.crosspoint` directory will clear the entire cache.

Due the way it's currently implemented, the cache is not automatically cleared when a book is deleted and moving a book
file will use a new cache directory, resetting the reading progress.

For more details on the internal file structures, see the [file formats document](./docs/file-formats.md).

## Contributing

Contributions are very welcome!

For Vietnamese-specific issues, please open an issue on [this repository](https://github.com/lelinhtinh/crosspoint-reader-vi/issues).
For general CrossPoint features, see the [upstream project](https://github.com/crosspoint-reader/crosspoint-reader).

### To submit a contribution

1. Fork the repo
2. Create a branch (`feature/dithering-improvement`)
3. Make changes
4. Submit a PR

---

CrossPoint Reader is **not affiliated with Xteink or any manufacturer of the X4 hardware**.

This Vietnamese fork is maintained by [@lelinhtinh](https://github.com/lelinhtinh).

**Credits:**

- [CrossPoint Reader](https://github.com/crosspoint-reader/crosspoint-reader) by [@daveallie](https://github.com/daveallie) - Original project
- [diy-esp32-epub-reader](https://github.com/atomic14/diy-esp32-epub-reader) by atomic14 - Inspiration for the original project
- [Pridi Font](https://fonts.google.com/specimen/Pridi) - Thai/Vietnamese font family
