### Added:
- Preview images in More Info popup (#285 by @Crykon & @WillyJL):
  - Opt-in from Settings > Images > Preview images
  - Downloads on-demand only while the game's More Info popup is open, resuming/interrupting as it's opened/closed
  - Caches to disk, which can take a lot of space, so off by default
- Fullscreen image viewer (by @WillyJL):
  - From More Info popup, click either on cover image or on a preview image
  - Image will stay in fullscreen until your next click
  - Use arrow keys to cycle this game's images
  - Can also press space to toggle the viewer instead of clicking with mouse
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
- Donor DDL downloads can now be retried when failed (by @WillyJL)

### Updated:
- Animations (Video and GIF) and Comics (CG, Comics, Manga and Pinup) thread types are detected properly now, "Collection" and "SiteRip" prefixes are now ignored (by @WillyJL)
- Notes textbox now resizes with how many lines are in the notes, so it is not the textbox that scrolls but rather the info popup (by @WillyJL)

### Fixed:
- Fix window show/hide from other threads and from tray icon (#286 by @cicklolwut & @WillyJL)
- Restrict RPC server CORS to the browser addon and f95zone.to (#287 by @cicklolwut)
- Redraw UI when popups are opened/closed (by @WillyJL)
- Donor DDL downloads can now be aborted correctly when stuck (by @WillyJL)
- Limit image downloads to 2 per second just in case (by @WillyJL)
- Fix image downloads in censorship regimes (by @WillyJL)

### Removed:
- Collection and SiteRip thread types are gone, these are now detected as the appropriate animation/comic/game type
