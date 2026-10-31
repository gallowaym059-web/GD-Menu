# EndoHack

Android32 + Android64 Geode mod. Touch-first, draggable category windows. Open it with the **Menu** button on the main menu.

## Windows (108 controls + 9 info rows)
- **Display**: Show FPS (Cocos stats), FPS Modifier 120, Custom FPS target slider (30-240), 30/60/90/120/144/240 presets, Battery Saver 30, Reset, Apply at startup.
- **Stats Overlay**: persistent on-screen FPS / frame time / avg / min / max / 1% low / jitter / graph / clock / date / session / frames / resolution / node count, plus corner, size, opacity, font, update rate and warning threshold.
- **Appearance**: presets (Megahack / Eclipse / Compact), accent modes (incl. rainbow), UI scale, opacities, window width, row/title height, rows per page, fonts, text colors, high contrast.
- **Behavior**: reduce motion, open animation, snap to grid/edges, lock windows, accordion, back button behavior, safe margin, click sound, reset/arrange/collapse/expand buttons.
- **Shortcuts**: one-tap buttons for the most used toggles.
- **Info**: versions, resolution, FPS target, session, copy debug info.

FPS Modifier requests a 1/120 s animation interval; it does **not** guarantee 120 rendered FPS (device, game, loader or power limits can cap it). Priority: Battery Saver > Custom target > FPS Modifier > 60.

## Source layout
`src/Settings.hpp` registry + persistence · `Fps.hpp` · `StatsOverlay.hpp` · `Actions.hpp` · `MenuWindow.hpp` · `ModMenuLayer.hpp` · `main.cpp`. Add a control by adding one line to the registry in `Settings.hpp` and, if it needs behavior, a branch in `Actions.hpp`.

## Build
Set `GEODE_SDK`, then `geode build --platform android64` and `geode build --platform android32`, or use `.github/workflows/build.yml` (root `build.yml` is a copy).

## Tested vs NOT tested
- Tested: syntax check only, against hand-written stub headers (no real Geode SDK or NDK was available). This catches typos, not API mismatches.
- NOT tested: real Geode/NDK build on either ABI, on-device behavior, touch feel, memory/FPS impact.
- APIs to verify first against your SDK headers: `SceneManager::keepAcrossScenes`, `utils::clipboard::write`, `FMODAudioEngine::sharedEngine()->playEffect`, `CCDirector::setAnimationInterval/setDisplayStats`, `VersionInfo::toVString`, `CCLayer::setTouchPriority` priority ordering, and the `mod.json` `gd` version / version tag.

## Known limitations
- No search bar, tooltips, scrolling (windows are paged), theme color picker, or floating open button; the menu can only be opened from the main menu.
- Safe area is a user-set margin, not queried from the OS notch/cutout insets.
- No gameplay hacks are included; everything is display/UI/stats, so nothing touches leaderboards or other players.
- Layout presets overwrite related Appearance values once; later edits do not flip the preset back to "Custom".

## Target versions
`mod.json` targets Geode 5.11.2 and GD 2.2091 (read from the loader binary supplied by the user). The source was written from Geode 4.x API knowledge, so expect some compile fixes against the 5.x SDK headers.
