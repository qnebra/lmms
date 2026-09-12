# LMMS – Copilot Agent Instructions

Trust these instructions. Only search the codebase if information here is incomplete or appears incorrect.

## Project Summary
LMMS (Linux MultiMedia Studio) is a cross-platform, open-source digital audio workstation (DAW) written primarily in C++20. It provides a Song Editor, Piano Roll, Beat Sequencer, Mixer, and a plugin system supporting VST2, LADSPA, LV2, SoundFont2, MIDI, and more.

**Languages:** C++ (90%), C (5%), CMake (3%), CSS, HTML, Python  
**Build system:** CMake  
**UI framework:** Qt 5 (Linux/default) or Qt 6 (macOS/Windows/optional)  
**Default branch:** `master`

---

## Repository Layout

```
CMakeLists.txt          Root CMake configuration (31 KB – all major options here)
vcpkg.json              vcpkg dependency manifest (Windows/MinGW builds)
Brewfile                macOS Homebrew dependency list
.clang-format           Code formatting rules (C++20, tabs=4, ColumnLimit=120)
.clang-tidy             Static analysis checks
.editorconfig           Editor settings
.gitmodules             Submodule declarations (Carla, portsmf, etc.)
src/
  core/                 Core audio engine (Engine, AudioEngine, Song, etc.)
  gui/                  All Qt GUI widgets and dialogs
  tracks/               Track types (BBTrack, SongTrack, AutomationTrack, etc.)
  common/               Shared utilities
  3rdparty/             Vendored third-party source (populated by submodules)
  lmmsconfig.h.in       CMake-generated config header template
  lmmsversion.h.in      CMake-generated version header template
include/                Public headers shared across core, gui, plugins
plugins/                One directory per plugin (instrument or effect)
tests/
  src/                  C++ unit tests (built and run via ctest)
  scripted/             Python scripted checks (check-strings, check-namespace, verify)
data/                   Themes, locales, presets, demo songs
cmake/                  CMake modules and toolchains
  toolchains/           Cross-compilation toolchains (MinGW, etc.)
buildtools/             Helper scripts (e.g. update_locales)
doc/                    Wiki source, bash-completion, doxygen config
.github/workflows/      CI workflow definitions
```

---

## Code Style Rules (enforced by CI)

- **Formatting:** clang-format config in `.clang-format`. Tabs (width 4), column limit 120, braces on their own line (`BreakBeforeBraces: Custom`), pointer alignment left.
- **Naming:** Classes/Enums/Structs → `CamelCase`; functions/methods → `camelBack`; static variables prefix `s_`; member variables prefix `m_` (convention, not yet enforced by tidy).
- **Namespaces:** All new source files (`.h`, `.cpp`) **must** be inside `namespace lmms { }`. Header files require a header guard (`#ifndef … #define`) or `#pragma once`. Long namespaces must have a closing comment `} // namespace lmms`. Violations cause CI failure via `tests/scripted/check-namespace`.
- **Translation strings:** Class names used in `data/locale/*.ts` and `data/themes/*/style.css` must exist in the source. Violations cause CI failure via `tests/scripted/check-strings`.
- **YAML files:** Must pass `yamllint` (config: `.yamllint`). All `.yml` files in the repo are linted.
- **Shell scripts:** Must pass `shellcheck v0.9.0`.

---

## Building (Linux / Ubuntu 24.04 – primary CI target)

Always check out with submodules:
```bash
git submodule update --init --recursive
```

### 1. Install dependencies (Ubuntu 24.04)
```bash
sudo apt-get update -y
sudo apt-get install -y --no-install-recommends $(xargs < .github/workflows/deps-ubuntu-24.04-gcc.txt)
```
Key packages: `cmake`, `gcc`, `g++`, `libqt5svg5-dev`, `qtbase5-dev`, `libfftw3-dev`, `libfluidsynth-dev`, `libasound2-dev`, `libjack-jackd2-dev`, `libsndfile1-dev`, `libsdl2-dev`, `libsamplerate0-dev`, `liblilv-dev`, `libsuil-dev`, `lv2-dev`, `libgig-dev`, `libstk-dev`, `portaudio19-dev`, `ccache`.

FLTK (custom build required – not from apt):
```bash
git clone --depth=1 https://github.com/fltk/fltk -b master fltk
cmake -S fltk -B fltk/build -DFLTK_BUILD_SHARED_LIBS=ON -DFLTK_BACKEND_WAYLAND=ON \
  -DFLTK_USE_LIBDECOR_GTK=OFF -DFLTK_BUILD_TEST=OFF -DFLTK_BUILD_GL=OFF
cmake --build fltk/build
sudo cmake --install fltk/build --prefix /usr
```

### 2. Configure
```bash
cmake -S . -B build \
  -DUSE_WERROR=ON \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DTARGET_UARCH=official \
  -DUSE_COMPILE_CACHE=ON
```
`-DUSE_WERROR=ON` is set in CI – all compiler warnings are errors. Do not introduce new warnings.

### 3. Build
```bash
cmake --build build
# or with parallel jobs:
cmake --build build -- -j$(nproc)
```

### 4. Run Tests
```bash
cd build/tests
ctest --output-on-failure -j2
```

### 5. Package (optional)
```bash
cmake --build build --target package
```

---

## Scripted Checks (CI: `checks.yml`)

Run these before pushing. Requires Python 3 and `python3-tinycss2`:
```bash
sudo apt-get install -y python3-tinycss2
git submodule update --init --recursive  # required by check-namespace
tests/scripted/verify         # self-tests for the scripted checks
tests/scripted/check-strings  # validates locale/theme class names vs source
tests/scripted/check-namespace  # validates namespace/header-guard conventions
```

YAML lint (run from repo root):
```bash
for i in $(git ls-files '*.yml'); do yamllint $i; done
```

Shell scripts lint:
```bash
shellcheck $(find "./cmake/" -type f -name '*.sh' -o -name "*.sh.in") \
  doc/bash-completion/lmms buildtools/update_locales
```

---

## macOS Build Notes
- Uses Qt 6 (`-DWANT_QT6=ON`), Xcode 16.4, target deployment: 10.15 (x86_64) / 11.0 (arm64).
- Install dependencies: `brew bundle install`
- Same CMake configure/build/test/package steps as Linux apply.

## Windows (MSVC) Build Notes
- Qt 6.8.x (`win64_msvc2022_64`), Ninja generator, vcpkg toolchain.
- `cmake … --toolchain C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET="x64-windows"`

## Windows (MinGW cross-compile from Linux) Build Notes
- Uses `cmake/toolchains/x64-mingw-vcpkg.cmake` and vcpkg.
- Run `.github/workflows/setup-mingw-packages.sh` then install deps from `deps-ubuntu-24.04-mingw.txt`.

---

## Adding a Plugin
Each plugin lives in `plugins/<PluginName>/` with its own `CMakeLists.txt`. Register it in the parent `plugins/CMakeLists.txt` via `add_subdirectory`. Plugins must use `namespace lmms { }` and follow the class naming convention.

## Key CMake Options
| Option | Default | Effect |
|---|---|---|
| `USE_WERROR` | OFF | Treat warnings as errors (ON in CI) |
| `WANT_QT6` | OFF | Use Qt6 instead of Qt5 |
| `USE_COMPILE_CACHE` | OFF | Enable ccache |
| `TARGET_UARCH` | — | CPU arch optimisation (`official` = generic) |
| `CMAKE_BUILD_TYPE` | — | Set to `RelWithDebInfo` in CI |
