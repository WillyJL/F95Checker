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
  - Scroll to zoom then move mouse to pan
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
- Option to archive game when thread isn't found (by @WillyJL)
- Setting to disable Donor DDL extraction, in Settings > Manage > Extract downloads (by @WillyJL)

### Updated:
- Tex Compress transparently in background (by @WillyJL):
  - If Settings > Images > Tex Compress is enabled, images are now compressed in background
  - The uncompressed copy of an image is visible on screen while it is being / waiting to be compressed
  - Other images can be loaded while compression is running
  - Overall substancially improves usability of Tex Compress option
  - Only "downside" is all images will be compressed and in "random" order, instead of only compressing images when they are shown on screen
  - But once enabled and the backlog of existing images are compressed, future image additions/updates in your library will be compressed quicker and unobtrusively
- Animations (Video and GIF) and Comics (CG, Comics, Manga and Pinup) thread types are detected properly now, "Collection" and "SiteRip" prefixes are now ignored (by @WillyJL)
- Notes textbox now resizes with how many lines are in the notes, so it is not the textbox that scrolls but rather the info popup (by @WillyJL)
- Removing labels now asks for confirmation if Settings > Manage > Confirm when removing is enabled (by @WillyJL)
- Updated dependencies (by @WillyJL):
  - New cx-Freeze version skips unused PyQt6 .dll/.so/.dylib libraries, Windows/Linux install sizes reduced by 70-100MB
  - Possibly better Linux support with GLFW and desktop-notifier updates
- Make most remaining popups closable by clicking outside the popup (by @WillyJL)

### Fixed:
- Fix window show/hide from other threads and from tray icon (#286 by @cicklolwut & @WillyJL)
- Restrict RPC server CORS to the browser addon and f95zone.to (#287 by @cicklolwut)
- Redraw UI when popups are opened/closed (by @WillyJL)
- Donor DDL downloads can now be aborted correctly when stuck (by @WillyJL)
- Limit image downloads to 2 per second just in case (by @WillyJL)
- Fix image downloads in censorship regimes (by @WillyJL)
- Drastically improved stutters during AVIF image loading, most noticeable with 4K images (by @WillyJL)
- Drastically improved stutters during image compression, most noticeable in (what used to be) catastrophic cases with 5-10MB+ GIFs (by @WillyJL)
- Pending image loads are cancelled if Settings > Images > Unload off-screen is enabled and image is no longer visible, so visible images can load sooner (by @WillyJL)
- Fix missing icon on Wayland (#297 by @phubhunter)
- Handle Cloudflare capcthas as 'F95zone unavailable' warnings (by @WillyJL)
- Fix RPDL token expiry detection (by @WillyJL)
- Add link to RPDL browser register page due to captcha/verification seemingly being required now (by @WillyJL)
- Fix scrollbar not being clickable in multiline text fields (by @WillyJL)

### Removed:
- Collection and SiteRip thread types are gone, these are now detected as the appropriate animation/comic/game type
