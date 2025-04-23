# vscode
## Requirements
- Install PlatformIO extension in VSCode
- Install Wokwi extension in VSCode

## Usage
Using the PlatformIO extension, pick the directory of this project as the project directory. This will create a `.pio` folder in the root directory of the project. This folder contains all the files needed to build the project.   
Type f1 then type `PlateformIO: Build` to build the project.
Now create:
- `diagram.json` file in the root directory of the project. This file will be used to create the diagram.
- `wokwi.toml` file in the root directory of the project. This file will be used in order to utilize the Wokwi extension.

The `wokwi.toml` file should look like this:
```toml
[wokwi]
version = 1
firmware = '.pio\build\uno\firmware.hex'
elf = '.pio\build\uno\firmware.elf'
```

In order to use the Wokwi extension type f1 then type `Wokwi: Start Simulator`. This will open a new window with the Wokwi simulator.