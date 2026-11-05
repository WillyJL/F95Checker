## 11.2.2

### Added:

- Add arrows and a counter to image viewer (#327 by @zerebos)
- Preview images gallery can be resized by dragging the scroll bar vertically (by @WillyJL)

### Fixed:
- Slow responses from cache api are now handled gracefully (by @WillyJL)
- Fix libraries not being bundled on Linux (by @WillyJL)
- Fix height shifting during verification and extraction of Donor DDL files (by @WillyJL)
- Show empty progress bar when showing a Donor DDL error (by @WillyJL)
- Updated browser addon for new reviews page (by @WillyJL)
- Fix private mode option for LibreWolf, Floorp, Zen, Helium (by @WillyJL)
- Fix Donor DDL zip extraction of file permissions and symlinks (by @WillyJL)
- Fix path/link traversal during archive extraction (by @WillyJL)
- Ignore wine-based wrappers for non-windows exes (by @WillyJL)
- Don't suppress exe console output to help with toubleshooting (by @WillyJL)
- Workaround Renpy's questionable Wayland support (by @WillyJL)

## 11.2.1

### Updated:
- Incremental changelog system, showing changelogs for all versions between your current one and the new one (by @WillyJL)

### Fixed:
- Fix Windows notifications with PyWinRT 4.0 (#326 by @zerebos)
- Fix duplicate preview images (by @WillyJL)
- Release will not update to beta if beta is older than release (by @WillyJL)

## 11.2

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
- Parallelized image I/O and decoding (by @WillyJL):
  - Multiple threads can now be allocated for image loading
  - Customizable parameters In Settings > Images for how many threads to use for I/O, decoding and limit to concurrent GIF decoding
  - Means that next images in queue can start being fetched from disk while previous images are still decoding, and multiple of these can happen at once
  - Source images (with Tex compress option disabled) can now load much faster, like they should have to begin with
  - Compressed images (with Tex compress option enabled) can now load even faster, making loading truly unnoticeable, though the tradeoff might not be worth to most users
  - Due to this, Settings > Images > Unload off-screen is now on by default, and existing users who have it off should consider enabling it to save RAM and VRAM
  - The default (4 I/O threads, 3 Decode threads, 2 Decode GIFs) should be fine for most users, but tweaking for your hardware can help (if you go all out you might get stutters)
  - If you want the previous behavior (one image at a time) set all 3 parameters (I/O threads, Decode threads and Decode GIFs) to 1
  - Note that changing these parameters requires restarting the app
- Exe launch wrappers (#289 by @cicklolwut & @WillyJL):
  - Allows setting custom arguments and wrapper commands for launching
  - Support for Wine/Proton on Linux/macOS
- Allow selecting folders as executables (by @WillyJL):
  - Works great together with Exe launch wrappers, so you can:
    - specify all Video/GIF collections to be opened with a custom command for your media player
    - select the folder as executable for each collection
    - clicking Play will open all files in your media player
  - Video and Pinup collections and such, default to seleting a folder when adding an executable
- F95Checker Beta auto-update (by @WillyJL):
  - Enable in Settings > Manage > F95Checker Beta
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
- The label remove button now respects Settings > Manage > Show remove button, there is a new option to remove the label in right click menu over the text field (by @WillyJL)
- Updated dependencies (by @WillyJL):
  - New cx-Freeze version skips unused PyQt6 .dll/.so/.dylib libraries, Windows/Linux install sizes reduced by 70-100MB
  - Possibly better Linux support with GLFW and desktop-notifier updates
- Make most remaining popups closable by clicking outside the popup (by @WillyJL)
- Adding a duplicate game via the bottom add/search bar will show its More Info popup instead of a duplicate warning (by @WillyJL)
- Bulk game import actions will show a summary even if just 1 game was added instead of adding silently (by @WillyJL)

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
- Cache API errors and missing thread errors are now bunched together at end of refresh and are non-fatal (by @WillyJL)
  - Refresh can complete even if some games have cache API errors
  - Last available cached data is used in those cases
  - The errors are bunched into a single warning popup at end of refresh, summarizing what games had what API errors and explaining that their data might be slightly out of date
  - Missing threads are bunched into their own popup, and you can decide what to do to all of them at once (the usual remove, archive or convert to custom)
- Also use preview images from game thread, fix newline in game title (#298 by @xev-c & @WillyJL)
- Compatibility with F95zone XenForo 2.3 and new Watched Games system (by @WillyJL)

### Removed:
- Collection and SiteRip thread types are gone, these are now detected as the appropriate animation/comic/game type

## 11.1.3

### Fixed:
- Fix launching ELF binaries on Linux (by @WillyJL)
- Fix notification daemon spinning when main process dies (by @WillyJL)
- Fix images with 0ms durations freezing the program (by @WillyJL)

## 11.1.2

### Fixed:
- Don't draw continuously while focused unless necessary (by @WillyJL)
- Fix symbolic links in auto update (by @WillyJL)
- Pick correct macOS update file if on x64 (by @WillyJL)

### Removed:
- Removed "Interface > Render if unfocused" setting:
  - Doesn't make sense anymore
  - Previously it would always draw when focused, and always draw when unfocused if this setting was on
  - Now, both when focused and unfocused, it will draw only if necessary, eg mouse moved, GIF is playing...

### Known Issues:
- The new notification-daemon process on Linux can sometimes get stuck spinning and cause the app to crash

## 11.1.1

### Added:
- Right click option on Refresh button to manually check app updates (by @WillyJL)
- Add Cmd+W shortcut to close window on macOS (#268 by @cantunborn)

### Updated:
- Show download size and speed in Donor DDL progress bar (#271 by @FaceCrap & @WillyJL)

### Fixed:
- Fix updating on Windows when the app closes faster than Powershell starts (by @WillyJL)
- Detect unsupported configurations with app data and app install locations overlapping (by @WillyJL)
- Fix horizontal scrolling in more info popup (by @WillyJL)
- Fix webview blank page on macOS (#269 by @cantunborn)
- Fix bundled compressonator on Linux (by @WillyJL)
- Improve responsiveness when resuming drawing (by @cantunborn)
- Fix tray icon and menu theming on macOS (by @cantunborn & @WillyJL)

### Known Issues:
- The new notification-daemon process on Linux can sometimes get stuck spinning and cause the app to crash

## 11.1

### Added:
- Reviews Tab in More Info Popup (#224 by @WillyJL)
  - Labels moved next to executables
- Image Texture Compression (`(ASTC or BC7) + ZSTD`) option (#212 by @WillyJL):
  - Compresses images for instantaneous load times (after first compression which is slower)
  - Less VRAM usage, and potentially less disk usage, depending on configuration and GPU support
  - Check the hover info in settings section for details on trade-offs
- Unload Images Off-screen option (#212 by @WillyJL):
  - Saves a lot of VRAM usage by unloading images not currently shown
  - Works best together with Tex Compress, so image load times are less noticeable
- Preload Nearby Images option (by @WillyJL):
  - Starts loading images that aren't visible yet but are less than a window width/height scroll away
  - Works best together with Tex Compress, so image load times are completely unnoticeable
- Play GIFs and Play GIFs Unfocused options (#212 by @WillyJL):
  - Saves a lot of VRAM if completely disabled, no GIFs play and only first frame is loaded
  - Saves CPU/GPU usage by redrawing less if disabled when unfocused, but still uses same VRAM
- Tabs can now be reordered by dragging (by @WillyJL)
- macOS arm64 (Apple Silicon) binary builds (#258 by @rakleed)

### Updated:
- New notification system with buttons and better platform support, option to include banner image in update notifs (#220 by @WillyJL)
- Updates popup is now cumulative, new updates get grouped with any previous popups and moved to top (#220 by @WillyJL)
- Executable paths in More Info popup wrap after `/` and `\` characters for easier readability (by @WillyJL)

### Fixed:
- Fix switching view modes with "Table header outside list" disabled (by @WillyJL)
- Fix flashbang while interface is loading (#221 by @sodamouse)
- Fix GUI redraws not pausing when unfocused, hovered and not moving mouse (by @WillyJL)
- Fix missing `libbz2.so` on linux binary bundles (#222 by @WillyJL)
- Apply images more efficiently, eliminate stutters while scrolling, start showing GIFs before all frames are loaded (by @WillyJL)
- Improve images error handling and display (#212 by @WillyJL)
- Tags now sort alphabetically as expected (by @WillyJL)
- UTF-8 encoding is now forced (#230 by @WillyJL)
- Fix adding executables focusing the wrong folder if the game type is in only one of the folders (by @WillyJL)
- Fix marking as executable on Linux/MacOS with RenPy games including a dot in their name (by @WillyJL)
- Fix readability on some framed text with dark text (by @WillyJL)
- RPDL token is regenerated once it expires (by @WillyJL)
- Make Godot a recognizable engine type (#266 by @FaceCrap)
- Fix macOS .app bundle support for game launch (#259 by @cantunborn)
- Popup warning when Max Labels is reached (#231 by @FaceCrap)
- Fix crash when RPC proxy fails (by @WillyJL)
- Only import QtWebEngine when we use it, users can choose to delete QtWebEngine .dll/.so/.dylib if they don't want the integrated browser for login/downloads/browsing (by @WillyJL)
- Add unknown type/tag/status so API updates don't create errors (by @WillyJL)

### Removed:
- Excluded `libEGL.so` on linux binary bundles, fixes "Cannot find EGLConfig, returning null config" (by @WillyJL)

## 11.0.3

### Added:
- Table header visible in all view modes setting, allows changing sorting and shown elements in grid/kanban view(by @Willy-JL)

### Updated:
- DDL can extract 7zip and RAR archives too (by @Willy-JL)
- App icons are now rounded, and MacOS icon has an empty border to fit design guidelines (by @rakleed & @Willy-JL)

### Fixed:
- Added support for AVIF images, supports new F95zone attachment server conversion (by @Willy-JL)
- DDL files are deleted asynchronously now, avoids stutters on slow drives (by @Willy-JL)
- Simplify some error handling, correctly handles connection issues in some edge cases (by @Willy-JL)
- Fix some link icons not being recognized by the extension and missing the library icon (by @Willy-JL)
- Detect system SSL certificates on more Linux distros, include certifi as fallback (by @kalvisbuls & @Willy-JL)

### Known Issues:
- MacOS webview in frozen binaries remains blank, run from source instead

## 11.0.2

### Updated:
- More info popup has constant size, easier to read when cycling games as it no longer shifts based on scroll bar being visible or vertical content length (by @Willy-JL)
- Extension > Add in Background now enabled by default for new users (by @Willy-JL)

### Fixed:
- Fixed sorting corruption bug (#211 by @Willy-JL & @FaceCrap)
- Fixed incorrect tab for first frame that caused images from default tab to load (by @Willy-JL)
- Fixed GIF animation speed (by @Willy-JL)
- Shift+Scroll and Shift+Alt+Scroll when zooming banner images scales correctly with FPS and uses smooth scrolling (by @Willy-JL)
- Save new zoom area and times settings after Shift+Scroll and Shift+Alt+Scroll on banner images (by @Willy-JL)
- Optimize some text drawing by using ImGui wrapping instead of slow `wrap_text()` which is now fully gone (by @Willy-JL)
- Slightly improve some hover tooltips, fixed weighted score missing on hover in grid/kanban view (by @Willy-JL)
- Fix flicker when clicking arrows in more info popup (by @Willy-JL)
- Mitigate ratelimits on DDL file list check (by @Willy-JL)
- Fixed thread not found errors, better handling of F95zone database and DDoS-Guard errors (by @Willy-JL)
- Fix weird wrapping at edge of grid/kanban cells in some edge cases (by @Willy-JL)

### Known Issues:
- MacOS webview in frozen binaries remains blank, run from source instead

## 11.0.1

### Added:
- Insecure SSL option (by @Willy-JL)
- Archive/Unarchive button to more info popup (by @Willy-JL)
- Allow removing personal rating for selected games in context menu (by @Willy-JL)
- Show full note when hovering notes icon (#198 by @FaceCrap)

### Updated:
- Rename "Last Played" to "Last Launched" and add timeline event when manually setting launched date (by @Willy-JL)
- Save URIs and relative exe path correctly in Launched timeline event (by @Willy-JL)
- Add executable fuzzy matches subdirs in Default Exe Dir for game type, developer, name (by @Willy-JL)
- Add executable checks best partial match to account for versions/mods/other things in dir names (#163 by @MayhemSixx)
- Themed integrated browser right click menu (by @Willy-JL)

### Fixed:
- More efficient grid/kanban cell cluster text (#200 by @Willy-JL)
- Fix Windows start with system setting and quotes usage (#156 by @oneshoekid & @Willy-JL)
- Fix Extension mdi-webfont not loading from RPC (#205 by @FaceCrap)
- Fix Extension context menu missing after browser restart (#206 by @TheOnlyRealKat)
- Redraw screen when DDL is extracting to show when complete (by @Willy-JL)
- Improved Developer name sanitization for some characters like `()[]{}\` (by @Willy-JL)
- Catch font texture exceptions, set texture faster (by @Willy-JL)
- Respect scaling for rounded corners (by @Willy-JL)
- Detect new 502 error code format (by @Willy-JL)
- Fix latest updates search issues with dots around spaces (by @Willy-JL)
- Fix RPC private network CORS preflight (by @Willy-JL)
- Make sure GLFW logic happens in main thread (by @Willy-JL)

### Removed:
- Removed obsolete DDOS-GUARD bypass, no longer needed and never really worked

### Known Issues:
- Sorting can be sporadically break/change with some actions, seems to be memory corruption inside (py)imgui, re-launch to fix it or change sorting manually
- MacOS webview in frozen binaries remains blank, run from source instead

## 11.0

### Added:
- Tabs system, more ways to organize your library (by @Willy-JL)
- Tab customization and colored icons in extension (#112 #117 #118 #126 by @r37r05p3C7 & @Willy-JL)
- F95zone Donor DDL integration, download and extract in-app (#190 by @Willy-JL, #192 by @FaceCrap)
- Tag highlighting and extension settings (#115 by @r37r05p3C7)
- Timeline events system, keep track of what happened and when (#138 #152 by @r37r05p3C7)
- Show some game details above RPDL and Donor DDL downloads as reminder (#185 by @FaceCrap & @Willy-JL)
- Number of votes after forum score and weighted score option (#140 by @r37r05p3C7)
- Gracefully handle unrecognized tags (#148 by @r37r05p3C7)
- Support proxy for network calls (#171 by @disaster2395 & @Willy-JL)
- Filters persist between app restarts (#86 by @littleraisins)
- Option to not focus the app when adding games from extension (#116 by @r37r05p3C7)
- Extension icons on all thread links (#90 by @littleraisins, #134 by @r37r05p3C7)
- Notes in extension icons tooltips (#160 by @FaceCrap)
- Extension install instructions in settings sidebar (by @Willy-JL)
- Show last refresh time on refresh button when hovered (#159 by @FaceCrap)
- Downloads dir option for F95zone Donor DDL and RPDL torrent files (by @Willy-JL)
- Skip/force refresh for completed/archived games (#170 by @disaster2395)
- Middle click bottom right text to open F95zone (by @Willy-JL)
- Middle click game name to copy it (by @Willy-JL)
- Middle click open folder to copy exe path (by @Willy-JL)
- Press Ctrl+A to select all currently shown games (by @Willy-JL)
- Show async tasks count in refresh settings (by @Willy-JL)
- Filter by custom game flag (by @Willy-JL)
- Software webview option for integrated browser (by @Willy-JL)
- Style loading from `styles.toml` file (#130 by @r37r05p3C7)

### Updated:
- Refresh process with F95Checker Cache API (#180 #184 #187 #188 by @Willy-JL):
  - Due to strain this tool was causing on F95zone servers they now have many ratelimits which make it unfeasible to continue as before
  - From now on F95Checker will not get game info directly from F95zone, but from a middleman F95Checker Cache API
  - Check the README/OP for more info, but basically:
    - No longer required to login with an F95zone account to use F95Checker, except for:
      - Regular, Masked and Donor DDL downloads
      - Notification checking
      - Importing bookmarked/watched threads
      - Opening webpages if you enabled Browser > Download pages
    - Refreshing is MUCH faster, semantics of "fast/full refresh" changed (check README/OP)
    - Regular download links are not publicly available, thus they now work similarly to masked download links
    - In rare occasions some updates/changes may be slightly slower to be detected and this is out of your control
  - Workers setting renamed to Connections, has different meaning, and max is now 10
  - Bunch of other things tweaked slightly to fit this new paradigm
- Finished/Played checkbox behavior (by @Willy-JL):
  - Renamed from Played to Finished as its semantics changed
  - Now remembers what version you marked as finished
  - Finished checkbox is relative to Installed checkbox:
    - Marking as finished saves the currently installed version as the one you finished
    - If you mark as finished when you don't have the latest version installed, Finished checkbox will be half-set when you mark a new version as installed
  - Basically reflects the previous behavior of Installed checkbox, but now for Finished checkbox too
  - Check README/OP FAQ for more info
- Redesigned grid/kanban cells, more compact data and nicer to look at (by @Willy-JL)
- More efficient image loading, prioritizes newly shown images (by @Willy-JL)
- Option to open RPDL torrents as temporary files (by @Willy-JL)
- Adding game executable auto-navigates to closest named folder in default exe dir (by @Willy-JL, #129 by @r37r05p3C7)
- Quick search now uses latest updates search, more search options and no login required (by @Willy-JL)
- More info popup organized in 2 columns (by @Willy-JL)
- Dates shown in monospace font for consistent spacing (by @Willy-JL)
- Enabled smooth scrolling for integrated browser (by @Willy-JL)
- Default exe dir is OS dependent, useful if you dual-boot (by @Willy-JL)
- Default datestamp format changed to more readable `%b %d, %Y` (by @Willy-JL)
- Notification checking disabled by default to use F95Checker without logging in (by @Willy-JL)
- Animated tray icon during refresh (#114 by @FaceCrap)
- Mark installed after adding executable (#172 by @FaceCrap)
- Clicking masked download links will process the captcha then open in chosen browser (by @Willy-JL)
- Regular download links now grabbed from thread in small integrated browser (#187 by @Willy-JL)
- Integrated browser now supports private mode (by @Willy-JL)
- Custom game flag separate from status, can now set status for custom games (by @Willy-JL)
- Allow setting last updated for custom games (by @Willy-JL)
- Adding a custom game now opens a more info popup for it (by @Willy-JL)
- Open button in file picker as alternative to double click (by @Willy-JL)
- Bumped OS targets and dependencies, Python 3.11 minimum (by @Willy-JL)

### Fixed:
- F95zone ratelimits properly handled (by @Willy-JL)
- Improve detection of many game details and add fallbacks (by @Willy-JL)
- SSL certificates verified, and SSL trusted CA loaded from system store (by @Willy-JL)
- Many memory usage optimizations (by @Willy-JL)
- Fix refresh cancel leaving many background tasks (by @Willy-JL)
- Fetch images with alternative hosts when main domain is blocked (by @Willy-JL)
- Custom images won't reset after threads change images (by @Willy-JL)
- Imgur images caught and marked as blocked as imgur does not allow fetching them (by @Willy-JL)
- Fix image loading issues due to file size and truncated data (#93 by @littleraisins)
- Choose font color with enough contrast for labels, types and tabs (#109 #131 by @r37r05p3C7)
- Notification popup removed if next check finds no more notifs (by @Willy-JL)
- Temporary connection errors ignored while in background mode (by @Willy-JL)
- Fixed issues on Wayland Linux (#182 by @Willy-JL)
- Detect all browsers on Linux according to `XDG_DATA_DIRS` (by @Willy-JL)
- Detect shebang and exe/msi magic numbers to correctly launch games on Linux/MacOS (by @Willy-JL)
- Open HTML files in chosen browser when clicking play (by @Willy-JL)
- Quickly reverse direction when smooth scrolling (#139 by @harikattar)
- Bookmarks import correctly with more than 99 pages (#99 by @initialseed)
- Improve RPDL search query generation with special characters (by @Willy-JL)
- Fix integrated browser extension connection errors (by @Willy-JL)
- Improved error handling for RPDL torrents downtime (by @Willy-JL)
- Fix latest updates extension icons conflicting with forum watched icon (#113 by @FaceCrap)
- Fix visual clipping due to rounding errors in interface scaling (#128 by @r37r05p3C7)
- Fix MacOS PyQt segfault (#87 by @batblue)
- Fix integrated browser on Linux (#96 by @littleraisins)
- Fix integrated browser race condition hanging at 100% CPU usage (by @Willy-JL)
- Fix BG mode refresh/notifs race condition causing runtime warning (by @Willy-JL)
- Fix many old typos or weird wording (#194 by @Willy-JL)
- Fixed running without stdout or stderr (by @Willy-JL)

### Removed:
- MacOS binaries no longer codesigned as Apple broke it and made it pointless
- Main process doesn't re-launch itself to make a `log.txt` anymore, use the Debug binaries in console to get log output
- Parser processes option and multiprocessing logic are gone, as parsing is no longer done clientside

### Known Issues:
- Sorting can sporadically break/change with some actions, seems to be memory corruption inside (py)imgui, re-launch to fix it or change sorting manually
- MacOS webview in frozen binaries remains blank, run from source instead

## 10.2

- FIXED: game name parsing (again)
- FIXED: binaries on macOS

## 10.1.2

- FIXED: `ai-cg` tag missing
- FIXED: prefixes in game names bug (breaking forum change)
- NEW: multi- select games in list with ctrl and shift click
- NEW: customize RPDL.net search prompt
- NEW: count of matching games in kanban column headers
- FIXED: Login window issues on unix-like when running from binaries
- FIXED: building on macos (hopefully for good)
- FIXED: issues with new refresh process
- FIXED: handle more forum errors
- NEW: option to copy links as bbcode (forum format)
- FIXED: more filepicker crashes

## 10.1.1

- NEW: Switched to a new refresh system (thanks Sam) which is more reliable, MUCH  faster, and causes less problems to the forum

WARNING: This means versions are detected differently, I tried to make it auto-adjust but you might encounter some slight annoyances; I suggest you refresh right after you update to the new version

- NEW: Custom games, manually add games from other platforms
- FIXED: Update marker for not installed games
- NEW: Archive games to mute updates
- FIXED: Removed RPDL magnet links (they were removed from the website)
- FIXED: Filepicker permission errors
- NEW: Select custom images for games (if you were using "keep game image" you need to reselect the custom images!)
- NEW: URI as game executable option
- FIXED: Update script console output
- FIXED: RPDL date timestamps

## 10.0

- NEW: download links! open / copy download inks from info popups
- NEW: rpdl.net integration! download tab in info popup has quick access to RPDL
- NEW: integrated browser, replaces default "unselected" browser
- NEW: more compact and friendlier filters
- NEW: recheck button in game info popup
- NEW: improved version and developer text detection
- NEW: improved description and changelog text detection
- NEW: attempt to bypass DDOS-Guard challenges
- FIXED: better login window
- FIXED: MacOS builds not opening
- FIXED: smarter quicksearch popup
- FIXED: problems with daily backups
- FIXED: improved parser process handling
- FIXED: removing executables saves properly
- FIXED: retry count applies to timeout errors
- FIXED: rpc error "address in use" after tool restart
- FIXED: keep refreshing if a thread was not found / deleted, show msgbox instead
- and more performance and stability improvements

## 9.6.2

- FIXED: crash on grid columns = 1

## 9.6.1

- NEW: quick filters (click on types, labels, statuses, tags, updates to add a filter)
- FIXED: all known "UnboundLocal" crashes
- FIXED: bottombar quicksearch error
- FIXED: extension issues on chrome
- FIXED: extension library icons missing on default F95Zone theme
- FIXED: extension library icons clipping on latest updates page
- FIXED: extension library icons are more visible
- FIXED: developer column sortable again
- FIXED: weird kanban view wrapping issues
- NEW: update script shows status messages
- FIXED: added note about user data on update prompt

## 9.6

- NEW: create your custom labels and assign them to games how you want
- NEW: kanban board style view based on your custom labels
- NEW: browser extension shows icons for games inside your library
- NEW: game forum score value (stars / 5), separate from personal rating
- NEW: insert icon selector for most textboxes
- FIXED: some combo boxes show tags and widgets properly

## 9.5.1

- NEW: faster OpenGL, should be a little smoother and use a little less CPU/GPU
- NEW: improved image zoom functionality, should feel more intuitive
- NEW: user customizable timestamp formats
- NEW: smaller bundle size for Linux and MacOS by zipping some Qt6 dependencies
- FIXED: dropping files and folder in filepicker popups
- FIXED: added back thread and link buttons to update popups, along with the info popup button
- FIXED: duplicated popups for updates and notifications
- FIXED: info popup closes after removing game
- FIXED: some self updater bugs

## 9.5

- NEW: better handling of game updates:
  - difference between installed and latest version is clearer
  - "installed" checkbox can have 3 states: on, off and half (installed but new version available)
  - "update available" marker next to game's names
  - check the FAQ for more info
- NEW: games can have multiple executables selected
- FIXED: many login window issues, mainly on macOS
- NEW: interface refreshes only when needed for less cpu/gpu usage
- NEW: updates popup allows to open the game info screens and scroll through them
- NEW: play button shows executable status (selected, not set, invalid/missing)
- NEW: type labels now show in the image corner for grid mode
- FIXED: better detection for developer names
- NEW: option to not check notifications
- NEW: separate timer for BG notifs and game update checks
- NEW: last successful refresh timestamp in settings > refresh
- NEW: clicking the "last played" timestamp changes it to right now
- FIXED: less lag when doing full rechecks
- FIXED: scrolling is synced between list and grid mode
- FIXED: when closing in grid mode, ui settings are not reset
- FIXED: easier to clear the bottombar, by pressing ESC or right click > clear
- NEW: dedicated column for status value, allows sorting by it
- FIXED: crash at start on some debian linux distros (glGetUniformLocation invalid operation)
- NEW: debug mode (F95Checker-Debug.exe) - keeps the console and logs more errors
- FIXED: console screen hanging around on Windows (when not using debug mode)
- FIXED: large amounts of unread alerts (1000+) are handled correctly
- FIXED: clearing cookies actually clears them, also fixes error 400 header too long
- FIXED: Windows "semaphore expired" error, check the refresh settings
- FIXED: database and ui settings should save properly on unexpected exits
- FIXED: "more details" text now uses a monospace font for cleaner error messages
and a bunch more fixes and improvements...

## 9.4

While adjusting some widget IDs the main table configuration got messed up, so you might lose your layout (column order, visibility, sorting and size). No other data will be lost. Sorry for the inconvenience.
- FIXED: weird input box behavior when deleting text
- FIXED: removing filters takes out the correct one
- FIXED: crash when pasting with non-text payloads
- FIXED: split Media type into CG, Collection, Comics, GIF, Manga, Pinup, SiteRip and Video types
- NEW: added a filter box for the file picker widget
- NEW: added loading bar and refresh button to login window
- NEW: toggle to disable RPC
- NEW: added open folder button and middle click functionality to open webpage (copies link)
- FIXED: improvements to database, thread parsing, breaking change detection
- FIXED: uploaded old releases to virustotal

## 9.3.1

- FIXED: crash when removing games in the info pane
- FIXED: updates popup only shows important info
- FIXED: bottom bar filtering improvements
- FIXED: better text inputs, less lag and unlimited text input
- NEW: added paste to right click menu on most text boxes
- NEW: smooth refresh progress bar
and more small fixes

## 9.3

I would like to apologize for the absolute train wreck that 9.2.x was, every single fix introduced another bug, and it was all due to distraction. Even though this release in particular has very little change in it, I feel like its stable enough (only 3 bugs left on my bug tracker are niche errors each experienced by 1 single user) and it is how 9.0 should have been, so I bumped to 9.3 to dissociate it from the disaster that 9.2.x was.

- FIXED: update check error for no apparent reason (reason is im stupid)

## 9.2.3

- WARNING: moved linux data folder from ~/.f95checker to ~/.config/f95checker, your data should be transferred automatically
- FIXED: many issues with interface scaling
- FIXED: better warning for github garbage in update check

## 9.2.2

- FIXED: error on first check for 8.5 or newly added games
- FIXED: auto update now tells you to NOT reopen manually

## 9.2.1

- FIXED: crash with x button on info popups

## 9.2

- NEW: cycle through info popups with arrows
- NEW: middle click on play buttons to open folders
- NEW: themed tray icon menu
- FIXED: auto update system (9.0 and 9.1 are still broken so you might need to update manually to 9.2, but this means that from 9.2 it should be fine going forwards)
- FIXED: window coordinates and sizing weirdness
- FIXED: too many redirects error
- FIXED: subprocesses are now async, interface wont lag while launching games
- FIXED: occasional crash with manual sort inconsistency
- FIXED: some weird browser list behavior

## 9.1

- FIXED: config migration is much more reliable
- FIXED: dead image links are skipped
- FIXED: dead browsers are not considered anymore
- FIXED: rpc port errors now have a more useful popup
- FIXED: uvloop import errors are skipped
- FIXED: cmd window no longer lingers on windows
- FIXED: ssl import errors on linux

## 9.0

Guess what, I rewrote this AGAIN, for the fourth time now!
Everything is new, better and fresh. Here I'll just mention the main differences.

- FIXED: switched from json to a real database system, no more corrupted configs!
- FIXED: you folks on linux should have no more issues with the login window.
- FIXED: prebuilt binaries are now available also for linux and macos.
- NEW: completely new interface!
- NEW: list mode has WAY more columns, and you can choose which ones are shown.
- NEW: you can sort and filter by many more information, also multisort, multifilter and manual sorting.
- NEW: added a comfy grid mode with game images.
- NEW: way more game information is supported, like engines, post types, descriptions, tags and more.
- FIXED: all your installed browsers are detected now, and you can configure a custom one if you want.
- NEW: way more settings and options, and the sidebar has been reorganized to be tidier.
- NEW: new ways to import games, such as F95Zone bookmarks and watched threads, browser bookmarks, multiple links and URL (shortcut) files.
- NEW: browser extension to add games easier! check out the F95Zone thread for more information.
- FIXED: it now remembers what version you had installed when an update comes, so you can still hit play.
- FIXED: you can now mark as played without marking the latest version as installed, for example if the update was just a small hotfix.
- And many more small fixes and improvements!

## 8.5

- NEW: MacOS support (experimental, let me know your experience); thanks @batblue!
- NEW: custom app icons made by yours truly
- NEW: keep old exe path and old image upon update toggles
- NEW: switch to aiofiles, interface might lag a bit less while refreshing
- NEW: use system theme styler option
- FIXED: backup broken config when reading fails
- FIXED: many issues with the tray icon
- FIXED: F95zone error / maintenance detection
- FIXED: crashes with big image popouts
- FIXED: browser detection on linux
- FIXED: rounded corners around refresh button image overlays
- FIXED: tweaked bg mode, now waits instead of refreshing immediately
- FIXED: many issues with the play button, now also allows non executables (like .html)
- FIXED: better console output logger

## 8.3h3

- FIXED: after updating the tool it should now relaunch properly

## 8.3h2

- NEW: checkbox to disable checking of completed games, this will speed up refreshing if you keep completed games in your list. Thanks @GR3ee3N!
- FIXED: .sh launch script for linux
- FIXED: new build routine for windows .exe, should not flag antiviruses as much now hopefully. Thanks @GR3ee3N!

## 8.3h1

- FIXED: The tool's own update checker runs after refreshing, so you see the game updates popup now
- FIXED: The tool's own update prompt now shows the changelog in a bigger textbox, easier to read
- NEW: When checking updates for the tool itself you will see it in the status text (bottom right corner)

## 8.3

- NEW: big image viewer: click on an image inside the changelog screen to open it bigger
- FIXED: login crash when clicking Save / Ok buttons on the login popups
- FIXED: cursor now shows as pointer over interactable elements
- NEW: "debug" option in the config file allows to log errors to file

## 8.2

- NEW: images support
  - Show game image inside refresh button on game hover
  - Show image header in changelog screen
  - Downloading images goes on in the background
- FIXED: smoother progress bar, not as noticeable when refreshing very fast / using the EXE version though
- FIXED: changelog screen more reliable
- FIXED: smaller code improvements

## 8.1

- NEW: filter / search game list by typing in the add box
- NEW: add games by searching for the name in the add box; will take first result from website's quicksearch
- FIXED: alphabetical and first added sortings work now
- FIXED: status badges on alternate rows now have correct background color
- FIXED: pasting an incomplete / bad link will not add the game

## 8.0h1

- FIXED: an issue with enabling 2FA while the tool is open

## 8.0

- Switched to HEAD requests (thanks ploper26!):
  - MUCH faster
  - No more issues with games not being found
  - Support any kind of thread (non game threads wont have accurate versions / changelogs, you'll just get notified when the link changes)
  - Restructured the config file, will auto adapt (7.x compatible saved as "pre8.0.json")
  - Max threads are now 999, default 100, will auto adjust to game count (just leave it at 100 if youre unsure)
- Added 2FA account support
- Added a notes section to the changelog screen
- Persistent cookies + login, tool won't need to login every time, 2FA login should last 30 days
- Added proper window geometry persistency, will remember size and position
- When opening the GUI it will now appear on top even if you clicked something else while it was loading
- Added some placeholder text in a few places to clear some things up
- Changed styling for the tray icon right click menu, now has a gradient and better margins
- More reliable overall
- Many other minor refinements and fixes

## 7.1h2

- NEW: singleton, now only one instance of the tool can run simultaneously, preventing config issues

## 7.1h1

- FIXED: bad import (QtWinExtras) on Linux

## 7.1

- NEW: custom thread count, default is 5 threads, should refresh at same speed as before but smoother, I suggest putting it higher only if you have more than 50 games
- NEW: BG mode aka background mode, hide F95Checker in system tray and have it refresh periodically (every 15 minutes by default)
- NEW: while refreshing in gui mode, the progress bar shows on the app icon
- FIXED: added some exceptions for games not being found
- FIXED: disabled adding and removing games while refreshing
- FIXED: removed winreg import on linux
- FIXED: minor bugfixes

## 7.0

- Completely rewrote the script ANOTHER time, switched from TkInter to PyQt and from requests to aiohttp
- The refresh process is A LOT faster (multithreaded), and much more reliable
- Whole new GUI, a lot more stable and much more beautiful, also featuring a theme editor
- NEW: if some games were updated, you will now get a messagebox telling you
- NEW: play button, you can now launch games straight from the tool
- NEW: played checkboxes, very similar to installed checkboxes, you can guess yourself what they do
- NEW: game statuses, now games will show an icon indicating if they are completed, on hold or abandoned
- NEW: standalone exe, you dont need to install Python anymore
- FIXED: connection errors and F95Zone server issues get handled properly
- FIXED: switched to json config (your previous config will transfer automatically), no more broken configs and angry users
- FIXED: Opera and OperaGX now get detected properly
- FIXED: when opening a webpage you wont see the cmd window anymore
- FIXED: auto updating from now on will be much smoother
- FIXED: the add text box and button are now pinned
