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

## How To Play
After injecting the code into the board, you can play the game by using your IR remote's buttons:
- `POWER`: toggle the game on/off
- `FAST BACK`: move the piece to the left
- `FAST FORWARD`: move the piece to the right
- `PAUSE`: toggle the game pause on/off 
- `VOl+`: increase the speed of the game
- `VOL-`: decrease the speed of the game