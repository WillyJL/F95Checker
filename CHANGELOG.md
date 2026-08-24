### Added:
- Exe launch wrappers (#289 by @cicklolwut & @WillyJL):
  - Allows setting custom arguments and wrapper commands for launching
  - Support for Wine/Proton on Linux/macOS
- Allow selecting folders as executables (by @WillyJL):
  - Works great together with Exe launch wrappers, so you can:
    - specify all Video/GIF collections to be opened with a custom command for your media player
    - select the folder as executable for each collection
    - clicking Play will open all files in your media player
- Show launch state on the play button (#288 by @cicklolwut)
- Locally tracked playtime duration (#290 by @cicklolwut)
- Label reordering (#291 by @px-pole & WillyJL)

### Updated:
- Animations (Video and GIF) and Comics (CG, Comics, Manga and Pinup) thread types are detected properly now, "Collection" and "SiteRip" prefixes are now ignored (by @WillyJL)

### Fixed:
- Fix window show/hide from other threads and from tray icon (#286 by @cicklolwut & @WillyJL)
- Restrict RPC server CORS to the browser addon and f95zone.to (#287 by @cicklolwut)
- Redraw UI when popups are opened/closed (by @WillyJL)

### Removed:
- Collection and SiteRip thread types are gone, these are now detected as the appropriate animation/comic/game type
