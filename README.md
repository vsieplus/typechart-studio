# Typechart Studio

Typechart Studio is a chart editor for [Typing Tempo](https://store.steampowered.com/app/2332930/Typing_Tempo), a typing rhythm game for Windows and Linux. It currently supports

* ▶️ **Dynamic playback of chart and music files**, with flexible timeline navigation abilities
* 📝 **Efficient and intuitive charting tools** for note and gimmick placement, such as copy-paste, note shifting, mirroring, undo/redo, and more
* 📂 **Loading and saving of chart files**, directly for use by Typing Tempo

Check out the [wiki](https://github.com/vsieplus/typechart-studio/wiki) for an introduction on how to use the tool.

## Download

Typechart Studio is available under [Releases](https://github.com/vsieplus/typechart-studio/releases) and on [itch.io](https://rainbeatgames.itch.io/typechart-studio).
If you purchase [Typing Tempo](https://store.steampowered.com/app/2332930/Typing_Tempo) on Steam, it will also be available in your Steam Library under 'Tools'.

## Build

Typechart Studio uses several external dependencies, including [Dear ImGUI](https://github.com/ocornut/imgui) and [json](https://github.com/nlohmann/json).
The following dependencies should be installed locally:

* [SDL2](https://github.com/libsdl-org/SDL) (>= 2.0.18 for ImGui compatibility)
* [SDL2 Image](https://github.com/libsdl-org/SDL_image) (>= 2.0.5)
* [OpenAL Soft](https://github.com/kcat/openal-soft) (>= 1.21.0)
* [libsndfile](https://github.com/libsndfile/libsndfile) (>= 1.1.0beta2 for mp3 support)

Once the above have been installed, you can create a Debug build using [CMake](https://cmake.org/):

```bash
mkdir build
cd build
cmake ..
cmake -- build .
```

Alternatively, if developing on linux, a utility build script is available to use in `build.sh`. If cross-compiling to windows, be sure to update the install paths for mingw32 as required.

```bash
# Create a debug build for linux -> build/linux/Debug/typechart-studio
bash build.sh

# Create a release build for linux -> releases/{version}/typechart-studio-{version}-linux.zip
bash build.sh linux Release

# Cross-compile to windows 64-bit using mingw -> releases/{version}/typechart-studio-{version}-windows64.zip
bash build.sh windows64 Release
```

## Contributing

- 🐛 To report a bug, please check the [issues](https://github.com/vsieplus/typechart-studio/issues) tab, and create a new issue if one does not already exist
  - Please fill out the issue template with sufficient detail, so that the bug may be accurately reproduced
- 🖋️ If you would like to contribute changes to this project, please refer to [CONTRIBUTING.md](CONTRIBUTING.md).

Thanks to everyone who has contributed to this project!

## License

Typechart Studio is licensed under the [zlib license](LICENSE.txt). See [licenses/](licenses/) for a full list of licenses of the dependencies used by Typechart Studio.
