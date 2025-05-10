# 🎮 Arduino Tetris with IR Remote, LCD, and Rotary Encoder

## 📦 Requirements
### Hardware
- Arduino UNO (or any other compatible board)
- IR Remote Control
- IR Receiver Module
- 16x2 LCD Display
- Rotary Encoder Module
- 220 Ohm Resistor (for the LCD)
- 8x8 LED Matrix Display (MAX7219)

### Arduino Libraries
  - `IRremote` (for IR remote control)
  - `LiquidCrystal` (for LCD display)
  - `LedControl` (for LED matrix display)
  
### Requirements to run in the simulator
- Install PlatformIO extension in VSCode
- Install Wokwi extension in VSCode

## 🔌 Components connections
### 🖥️ LCD (16x2)
| Pin LCD       | Connected to    |
|---------------|-----------------|
| RS            | Pin 13          |
| E             | Pin 9           |
| D4            | Pin 6           |
| D5            | Pin 5           |
| D6            | Pin 7           |
| D7            | Pin 4           |
| VSS           | GND             |
| VDD           | 5V              |
| RW            | GND             |
| A             | 5V *(con 220Ω)* |
| K             | GND             |

### 📶 Ricevitore IR
| Pin IR       | Connected to|
|--------------|-------------|
| VCC          | 5V          |
| GND          | GND         |
| OUT          | Pin 3       |

### 🔄 Rotary Encoder
| Pin Encoder | Connected to    |
|-------------|-----------------|
| CLK         | Pin 2           |
| DT          | Pin 8           |
| SW          | *Not used*      |
| VCC         | 5V              |
| GND         | GND             |

### 🔳 LED Matrix (MAX7219)
| Pin Matrix | Connected to|
|------------|-------------|
| VCC        | 5V          |
| GND        | GND         |
| DIN        | Pin 12      |
| CS         | Pin 10      |
| CLK        | Pin 11      |

---


## Installation
Clone the repository to your local machine. Then open the project in your IDE and inject the code into your Arduino board.

### Simulator
In VSCode, install the PlatformIO extension and the Wokwi extension. The PlatformIO extension is used to build the project and the Wokwi extension is used to simulate the project.   
Using the PlatformIO extension, pick the directory of this project as the project directory.
Type `f1` then type `PlatformIO: Build` to build the project.
This will create a `.pio` folder in the root directory of the project. This folder contains all the files needed to build the project.
- `diagram.json` file is used to simulate the project in Wokwi. This file contains the schematic of the project.
- `wokwi.toml` file is used to configure the Wokwi simulator. This file contains the configuration of the project.

The `wokwi.toml` file should look like this:
```toml
[wokwi]
version = 1
firmware = '.pio\build\uno\firmware.hex'
elf = '.pio\build\uno\firmware.elf'
```

In order to use the Wokwi extension type `f1` then type `Wokwi: Start Simulator`. This will open a new window with the Wokwi simulator.

## How To Play
### Before playing
> If you are testing the code in the simulator you have to set the `PRODUCTION` variable to `false` in the `main.cpp` file.
> Otherwise, if you are testing the code in the Arduino board you have to set the `PRODUCTION` variable to `true` in the `main.cpp` file.  
This is because the simulator IR remote control has different codes than the real IR remote control.  

>You are welcome to change the IR remote control codes in the `lib/utils/utils.h` file to fit in your IR remote control.  
After injecting the code into the board, you can play the game using the following controls:
#### 🎮 IR Remote Controls:
- `POWER`: Toggle the game on/off
- `FAST BACK`: Move the piece to the left
- `FAST FORWARD`: Move the piece to the right
- `PAUSE`: Toggle the game pause on/off 
  - During "Game Over" state, this button will restart the game
- `VOl+`: Increase the speed of the game
- `VOL-`: Decrease the speed of the game

#### 🔁 Rotary Encoder Controls:
- `Rotate Clockwise`: Increase the speed of the game
- `Rotate Counter-Clockwise`: Decrease the speed of the game


## ⭐ Game Features
- 8x8 LED matrix display for the game board
- LCD display for game state, score, and speed
- IR remote control for game controls
- Rotary encoder for speed control
- Game over detection and restart option
- Pause and resume functionality
- Scoring system based on the number of lines cleared
  - Each line cleared gives 10 points
- Visual feedback for game state