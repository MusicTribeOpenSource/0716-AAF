# IGNITE Application

The IGNITE sequencer application: UI, model/engine and hardware integration, built on JUCE.

## License

The IGNITE application is licensed under the GNU Affero General Public License v3.0 or later, see
[LICENSE](LICENSE). Bundled third-party code keeps its own license (see below).

If you run a modified version that users interact with over a network (for example the device's Wi-Fi
or LAN features), AGPL-3.0 section 13 requires you to offer those users the corresponding source code.

## Building

CMake 3.20 or later is required. [JUCE 7.0.12](https://github.com/juce-framework/JUCE) (GPLv3 / commercial)
is downloaded by CPM during configuration and patched with `patch/juce_multitouch.patch`. To use a local
JUCE checkout, pass `-DCPM_JUCE_SOURCE=<path>`.

Desktop (Windows, macOS, Linux):

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

On Linux, install JUCE's usual Linux build dependencies (ALSA, X11, freetype and so on) and the wireless
tools headers used by the Wi-Fi code (`libiw-dev` on Debian and Ubuntu, which provides `iwlib.h`).

The app is written to `build/apps/ignite/Ignite_artefacts/<Config>/`, together with `assets/`, which it
loads at runtime. The General MIDI SoundFont is not included; see [sf/README.md](sf/README.md).

Device (aarch64): build the SDK from the companion Buildroot tree (`Linux/`) with
`make Ignite_defconfig && make sdk`, then pass the SDK's toolchain file:

```sh
cmake -B build-arm -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=<sdk>/share/buildroot/toolchainfile.cmake
cmake --build build-arm
```

## Third-party components

| Component | Location | License |
|---|---|---|
| JUCE 7.0.12 (downloaded, not bundled) | fetched by `CMakeLists.txt` | GPL-3.0 / commercial |
| CPM.cmake | `CPM.cmake` | MIT |
| cmake_timestamp (Kalle Raiskila) | `timestamp/` | see `timestamp/license.txt` |
| CMSIS-RTOS2 API header (Arm) | `modules/Common/RTOS2/include/cmsis_os2.h` | Apache-2.0 |
| log.c (rxi) | `modules/Common/Thirdparty/log_c` | MIT |
| MidiLib (Francois Best) | `modules/Common/Thirdparty/MidiLib` | MIT |
| RtMidi (Gary P. Scavone) | `modules/Common/Thirdparty/rtmidi` | RtMidi license (MIT-style) |
| TinySoundFont (Bernhard Schelling) | `modules/Common/Thirdparty/TinySoundFont`, `modules/Firmware/Application/Model/tsf.h` | MIT |
