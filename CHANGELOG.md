### Added:
- Right click option on Refresh button to manually check app updates (by @WillyJL)

### Updated:
- Show download size and speed in Donor DDL progress bar (#271 by @FaceCrap & @WillyJL)

### Fixed:
- Fix updating on Windows when the app closes faster than Powershell starts (by @WillyJL)
- Detect unsupported configurations with app data and app install locations overlapping (by @WillyJL)

### Removed:
- Nothing

### Known Issues:
- MacOS webview in frozen binaries remains blank, run from source instead
- The new notification-daemon process on Linux can sometimes get stuck spinning and cause the app to crash
