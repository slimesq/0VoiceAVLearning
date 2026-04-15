# 0VoiceAVLearning

Audio/video learning examples built with CMake, Conan, and FFmpeg.

## Quick Start

Install or refresh dependencies:

```powershell
.\pull_dependency.ps1
```

Press Enter at both prompts to use the defaults:

- FFmpeg flavor: `light`
- Build type: `Debug`

Configure and build:

```powershell
cmake --preset voice-av-vs2022-x64
cmake --build --preset voice-av-vs2022-x64-debug
```

Run the basic FFmpeg version example:

```powershell
.\build\cmake\Debug\voice_av_learning.exe
```

## Dependencies

This project uses `ffmpeg/7.1.3` from Conan Center. Conan Center does not
publish an exact `ffmpeg/7.1` recipe, so the project pins the latest available
7.1 patch release.

Dependency files are generated under `build/generators`, and this project uses
a project-local Conan cache under `build/conan_home`.

Use a specific dependency flavor or build type:

```powershell
.\pull_dependency.ps1 -Flavor light -BuildType Debug
.\pull_dependency.ps1 -Flavor full -BuildType Release
```

The `light` flavor is the fast default and tries to reuse prebuilt Conan
packages. The `full` flavor enables more FFmpeg options and may compile FFmpeg
locally.

See [FFmpeg Dependency](docs/ffmpeg-dependency.md) for flavor details, Conan
option mapping, and native FFmpeg configure flags that the Conan Center recipe
does not expose.

## Editor Navigation

`pull_dependency.ps1` prepares FFmpeg source navigation for clangd:

- searches `third_party` for an existing FFmpeg source tree
- downloads FFmpeg source to `third_party/ffmpeg-src` if none is found
- writes the merged clangd database to `build/clangd/compile_commands.json`
- copies a generated root `compile_commands.json` for editor compatibility

If clangd reports `STL1000: Unexpected compiler version`, update LLVM clangd
to match your Visual Studio STL, or keep the provided `.clangd` setting that
suppresses that editor-only version check.

## Build Commands

Configure:

```powershell
cmake --preset voice-av-vs2022-x64
```

Build Debug or Release:

```powershell
cmake --build --preset voice-av-vs2022-x64-debug
cmake --build --preset voice-av-vs2022-x64-release
```

Open the generated Visual Studio solution:

```powershell
cmake --open build/cmake
```

## Examples

Examples live under [voice_av](voice_av).

- `voice_av/00-ffmpeg-version`: prints linked FFmpeg library versions.
- `voice_av/08-ffmpeg-demuxing-and-decoding`: demuxing and decoding examples.
- `voice_av/09-ffmpeg-encoding-and-muxing`: prepared for encoding and muxing examples.

Build the current group 08 targets:

```powershell
cmake --build --preset voice-av-vs2022-x64-debug --target voice_av_08_05_decode_audio
cmake --build --preset voice-av-vs2022-x64-debug --target voice_av_08_06_decode_video
```

Run group 08 examples from their `testFiles` directory:

```powershell
Push-Location .\voice_av\08-ffmpeg-demuxing-and-decoding\testFiles
..\..\..\build\cmake\Debug\voice_av_08_05_decode_audio.exe believe.aac believe.pcm
..\..\..\build\cmake\Debug\voice_av_08_06_decode_video.exe source.200kbps.768x320_10s.h264 source.yuv
Pop-Location
```

See [Voice AV Examples](voice_av/README.md) for the steps to add a new example.

## Documentation

- [docs/README.md](docs/README.md): documentation index.
- [docs/ffmpeg-dependency.md](docs/ffmpeg-dependency.md): FFmpeg dependency notes.
