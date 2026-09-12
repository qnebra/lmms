# Copilot instructions for LMMS

## Repository at a glance

- LMMS is a cross-platform digital audio workstation written in C++20 and built with CMake.
- Main application code lives in `src/`:
  - `src/common/` shared code
  - `src/core/` engine and non-UI application logic
  - `src/gui/` Qt UI
  - `src/tracks/` track-specific logic
- The executable is built from `src/`, and plugins are built from `plugins/`.
- Optional plugin selection and build-time plugin filtering are defined in `cmake/modules/PluginList.cmake`.
- Shared plugin build logic lives in `cmake/modules/BuildPlugin.cmake`.
- Tests live in `tests/`.
- Assets, presets, samples, and translations live in `data/`.
- Packaging, toolchains, and platform-specific build logic live in `cmake/` and `.github/workflows/`.

## How to search efficiently

- Start with narrow searches in `src/`, `plugins/`, `cmake/`, and `tests/`.
- Avoid broad repo-wide searches unless necessary: `data/` is very large and contains many presets, samples, translations, and binary assets.
- If a task is plugin-specific, limit reads and searches to that plugin directory plus the relevant `CMakeLists.txt`.
- Before assuming missing vendored code was deleted, check `.gitmodules`; this repository depends on many submodules.

## Build and validation

- Default local configure command:
  - `cmake -S . -B build -DUSE_WERROR=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo -DTARGET_UARCH=official`
- Build:
  - `cmake --build build`
- Unit tests:
  - `cd build/tests && ctest --output-on-failure -j2`
- Scripted repository checks:
  - `tests/scripted/verify`
  - `tests/scripted/check-strings`
  - `tests/scripted/check-namespace`
- Run the scripted checks when changing names, namespaces, file/class identifiers, or other strings that may also appear outside compiled C++ code.
- If you change translations under `data/locale/`, use the locale targets defined there (`update-locales` / `finalize-locales`).

## Platform and dependency notes

- Linux CI builds use Qt5, while macOS and Windows CI builds use Qt6.
- Linux CI also checks out and installs FLTK before configuring LMMS.
- `plugins/ZynAddSubFx` requires the FLTK `fluid` binary; failures there usually mean FLTK tooling is missing.
- The build depends on submodules. If vendored directories are missing, run:
  - `git submodule update --init --recursive`
- For faster focused iteration, prefer:
  - `-DLMMS_MINIMAL=ON`
  - or `-DPLUGIN_LIST="PluginA PluginB"`
  This reduces plugin build scope and can skip fetching unrelated plugin submodules.
- After configure, read the printed dependency summary. If an expected feature is disabled, install the missing dependency, remove `CMakeCache.txt`, and configure again.

## Style and editing conventions

- Use tabs with width 4.
- Follow the repository `.clang-format` for C++ style and include ordering.
- Keep changes surgical; this repository has many optional plugins and platform-specific code paths.
- Match nearby CMake style; CMake files here also use tabs heavily.

## Useful source-of-truth files

- `CMakeLists.txt` for top-level options, dependency detection, and build summaries
- `.github/workflows/build.yml` for the CI-supported build matrix and canonical configure/test commands
- `.github/workflows/checks.yml` for scripted validation commands
- `INSTALL.txt` for local build guidance
- `Brewfile`, `vcpkg.json`, and `.github/workflows/deps-*.txt` for platform dependency lists
