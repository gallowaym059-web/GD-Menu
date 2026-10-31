# Claude Code handoff brief

Use Claude Code to audit and repair this project against the actual installed Geode SDK headers. Do not guess C++/Geode APIs.

Target Android32 (`armeabi-v7a`) and Android64 (`arm64-v8a`). First identify concrete compile, lifecycle, touch-routing, persistence, and layout defects, then fix them in the project. Preserve the icon and `.github/workflows/build.yml`.

Keep the Display window. Show FPS must use the verified API to control Cocos display stats. FPS Modifier must request 120 updates/sec when on and restore 60 when off, with saved state restored at startup. Never guarantee actual 120 FPS. Preserve draggable windows, persistence, menu open/close, and back behavior. Mark unfinished controls as placeholders.

Fix touch cancellation/multitouch, safe-area bounds, orientation relayout, saved toggle initialization, and safe clamping on small screens. Keep gameplay features offline/single-player-friendly; do not affect leaderboards, other players, or anti-cheat.

Verify `mod.json` compatibility. Build both ABIs with the real SDK/NDK and report exactly what was tested versus not tested. Never call the mod bug-free without builds and device tests. Return complete changed files, paths, known issues, and build results.
