# Changelog

## v2.2091.1-beta
- Rewrote the menu as a split, data-driven source tree (`src/*.hpp`). 108 interactive controls + 9 info rows in 6 windows: Display, Stats Overlay, Appearance, Behavior, Shortcuts, Info.
- Display tab: Show FPS, FPS Modifier 120, custom FPS target slider, 30/60/90/120/144/240 presets, Battery Saver 30, reset, apply-at-startup.
- New persistent Stats Overlay (survives scene changes): FPS, frame time, avg/min/max/1% low, jitter, graph, clock/date, session, frame count, resolution, node count, corner/size/opacity/font options.
- Custom touch handling (no CCMenu): single-touch state machine, tap slop, cancellation handling, extra fingers swallowed, high touch priority so nothing leaks to the game.
- Paged windows (no clipping node needed), collapse state, drag with grid/edge snapping, safe margin clamp, relayout on window-size change, open animation with reduce-motion.
- Layout presets (Megahack / Eclipse / Compact), accent modes (per-window / custom hue / rainbow), UI scale, opacity, font and color options.
- Removed placeholder Player / Creator / Bypass windows and the no-op "Open Button" toggle. The main-menu open button can no longer be hidden (only resized), so the menu can't be locked away.
- mod.json `geode` set to 5.11.2 and `gd` 2.2091, both read from the user's installed Geode_android64.so.
- mod.json version tag changed from `-pre` to `-beta` (Geode's accepted tags are alpha/beta/prerelease; verify against your loader).

## v2.2091.0-pre
- Added the Display window with Show FPS and persistent FPS Modifier.
- Added original icon in `logo.png` and `assets/mobile-menu-icon.png`.
- Included visible root `build.yml` and GitHub Actions workflow for combined Android32 + Android64 builds.
- Added Claude Code handoff brief and testing limitations.
