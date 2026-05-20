# SoundsView — AGENTS.md

## Build & run

```sh
cmake -B build_terminal && cmake --build build_terminal
./build_terminal/SoundsView
```

## Architecture

- **Entrypoint:** `src/main.cpp` — creates `VoiceRoulette` (fullscreen frameless overlay)
- **No tests, no CI, no linter/formatter config** exists in repo
- `CMakeLists.txt` builds only under Qt6 (`if(QT_VERSION_MAJOR GREATER_EQUAL 6)`); Qt5 path is dead
- **Qt6 Multimedia / TextToSpeech modules are intentionally NOT linked** — ABI mismatch between 6.10.2 and 6.11.0, per comment in `CMakeLists.txt:16`
- **Old root-level files** (`datacontroller.*`, `datamodel.*`, `dataview.*`, `mainwindow.*`, `mainwindow.ui`) are stale — not in the CMake build

## Audio subsystem

| Concern | Mechanism |
|---|---|
| Sound playback | `QProcess` calling system tool: `paplay` → `pw-play` → `ffplay -nodisp -autoexit` → `aplay` (first found wins) |
| TTS | `spd-say -e -r -50 <text>` (speech-dispatcher) |
| Sound source | Hardcoded path `/home/Tiopaz/SoundsView/src/sounds/` in `SoundManager` |
| Supported formats | wav, mp3, ogg, flac, aac, m4a, wma |

## Signal-based toggle

- **SIGUSR1** toggles window visibility: `pkill -USR1 SoundsView`
- Signal handler installed in `src/main.cpp:13`

## Key UI behaviors

- **Escape:** hide (sound mode) / exit add mode / back from list mode
- **Alt:** hold to show radial menu; release to hide
- **Ctrl:** toggle menu size and loquendo expand/collapse
- **Enter / Space:** activate currently focused sector (under crosshair)
- Crosshair constrained to radius 400 (160 when Alt-held)
- Profile saved as `perfil.json` next to executable

## External dependencies

- `lib/QtAwesome` — vendored git submodule (Font Awesome icon provider)
- `QHotkeys/` — vendored git submodule (X11 global hotkeys, Qt6)
- Runtime: `paplay` or `pw-play` or `ffplay` or `aplay`; `spd-say`
