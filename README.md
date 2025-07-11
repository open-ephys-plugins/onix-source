# ONIX Source

Streams data from an ONIX device.

## Installation

This plugin must be built from source (see instructions below).

## Usage

(Coming soon)

## Building from source

First, follow the instructions on [this page](https://open-ephys.github.io/gui-docs/Developer-Guide/Compiling-the-GUI.html) to build the Open Ephys GUI.

Then, clone this repository into a directory at the same level as the `plugin-GUI`, e.g.:

```
Code
├── plugin-GUI
│   ├── Build
│   ├── Source
│   └── ...
├── OEPlugins
│   └── onix-source
│       ├── Build
│       ├── Source
│       └── ...
```

To get a fresh install of the repository, use `git clone --recurse-submodules` to fetch the submodule as well.

For existing clones of the repo, run `git submodule update --init --recursive` to initialize the submodule and update it simultaneously.

### Windows

**Requirements:** [Visual Studio](https://visualstudio.microsoft.com/) and [CMake](https://cmake.org/install/)

#### Create OnixSource project using CMAKE

From the `Build` directory, enter:

```bash
cmake -G "Visual Studio 17 2022" -A x64 ..
```

Next, launch Visual Studio and open the `OE_PLUGIN_onix-source.sln` file that was just created. Select the appropriate configuration (Debug/Release) and build the solution.

Selecting the `INSTALL` project and manually building it will copy the `.dll` and any other required files into the GUI's `plugins` directory. The next time you launch the GUI from Visual Studio, the ONIX Source plugin should be available.
 
