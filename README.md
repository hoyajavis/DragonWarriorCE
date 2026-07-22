# Dragon Warrior Randomizer (TI-84 Plus CE Demake)

A native C port and demake of the `pydw-randomizer` project, designed specifically to run on the **TI-84 Plus CE** graphing calculator.

This project aims to recreate the classic Dragon Warrior experience on the calculator using the CE C Toolchain. It features a robust custom engine capable of parsing XML map data into optimized AppVars, a full bytecode interpreter for complex dialogue and event scripting, and classic turn-based combat.

## Features

*   **Faithful Map Recreation**: A full parsing pipeline that converts classic Dragon Warrior map layouts (towns, castles, caves, and the overworld) into highly compressed memory structures, complete with line-of-sight cave darkness masking!
*   **Dialogue Bytecode Interpreter**: A custom virtual machine built in C that interprets packed binary dialogue scripts. It supports branching dialogue, conditional checks (`HAS_ITEM`, `LACKS_ITEM`), granting items, altering game state, and shop menus!
*   **Advanced Data Packing**: Because the TI-84 Plus CE has limited RAM, game maps and dialogue text are packed into external archive variables (`AppVars`) using custom Python build tools, allowing for huge amounts of content.
*   **Turn-Based Combat**: Classic RPG combat mechanics featuring a scaling level system, experience points, magic points, complex status effects (Sleep/Stopspell), authentic fleeing formulas, and scripted multi-stage boss encounters (including the Dragonlord's True Form!).
*   **Spells & Items**: Manage your inventory, consume Herbs to heal, use the SEARCH command to find hidden quest items, or cast spells like HEAL, HURT, SLEEP, STOPSPELL, OUTSIDE, RETURN, RADIANT, and REPEL during exploration or combat.
*   **Authentic Encounter System**: Accurately maps the overworld into 20 authentic `MonsterSets` for true-to-original encounter scaling, alongside faithful Repel/Fairy Water math based on Hero defense.
*   **Persistent Save States**: Features a robust three-slot save system written seamlessly to AppVars in the calculator's memory, allowing you to turn the calculator off and pick up right where you left off.
*   **Highly Optimized CE Engine**: Features smooth UI screen transitions, robust memory-safety checks for AppVar validation, dynamic RAM pointer protection during map loading, and heavy caching of Hero and Monster combat stats to maximize performance and prevent hardware resets.

## Controls

*   **Arrow Keys**: Move the Hero across the map / Navigate menus.
*   **[2nd]** or **[ENTER]**: Confirm menu selection / Interact with objects / Attack in combat.
*   **[ALPHA]**: Open the Command Menu / Go back / Acknowledge pop-up messages.
*   **[CLEAR]**: Instantly quit the game.

*Note: To SEARCH chests or SAVE your game, press **[ALPHA]** while exploring to open the Command Menu.*

## Installation & Compilation

To build this project from the source code, you will need the [CE C Toolchain](https://github.com/CE-Programming/toolchain) installed on your system.

### Compiling the Source Code

To heavily optimize the development process, the build pipeline is strictly separated into two distinct steps: Data Packing and C Compilation.

#### 1. Data Packing (Python)
If you are modifying the game's XML data files, dialogue scripts, or level/monster definitions inside the `python-reference` folder, you must pack them into the binary formats used by the calculator.
*   **Windows**: Double-click or run `pack_data.bat`.
*   **Mac/Linux**: Run `./pack_data.sh`.
*(Note: This step requires Python 3 installed. You only need to run this when the core game data changes, not when editing C code.)*

#### 2. C Compilation (Make)
To compile the C code and graphics into the final executable game:
*   **Windows**: Double-click or run `compile.bat`.
*   **Mac/Linux**: Run `./compile.sh`.
*(Note: This step is entirely independent of Python. It will rapidly build your C code and generate the `PYDW.8xp` file in the `bin/` directory.)*

### Testing in an Emulator (CEmu)

Testing the game in an emulator is the fastest way to iterate during development.
1. Download and install [CEmu](https://cemu.app/), the standard open-source TI-84 Plus CE emulator.
2. You will need a ROM image of a TI-84 Plus CE to use CEmu (you can extract this from your physical calculator using the emulator's built-in tools).
3. Once CEmu is running, simply drag and drop the `bin/PYDW.8xp` file into the emulator window.
4. Press `[PRGM]` on the emulated keypad, select `PYDW`, and press `[ENTER]` to run the game!

### Running on Physical Hardware

To play the game on a real TI-84 Plus CE calculator:
1. Connect your calculator to your PC using a Mini-USB cable.
2. Open [TI Connect CE](https://education.ti.com/en/products/computer-software/ti-connect-ce-sw) (official) or [TILP](http://lpg.ticalc.org/prj_tilp/) (open-source).
3. Open the "Calculator Explorer" tab to view the files on your device.
4. Drag and drop `bin/PYDW.8xp` into the Explorer window to send it to the calculator.
5. On your calculator, press the `[PRGM]` button, select `PYDW` from the menu, and press `[ENTER]` to start playing!

## Credits & Acknowledgements

*   **[hoyajavis/pydw-randomizer](https://github.com/hoyajavis/pydw-randomizer)**: The original Python project that this demake is based upon. The python-reference codebase provided the core randomizer concepts, math, and the original ripped sprite assets used in this game.
*   **[justinbeetle/pyDragonWarrior](https://github.com/justinbeetle/pyDragonWarrior)**: Justin Beetle and contributors for creating the original, highly-faithful recreation of Dragon Warrior in Python/Pygame, which served as the engine framework for `pydw-randomizer`.
*   **[mcgrew/dwrandomizer](https://github.com/mcgrew/dwrandomizer)**: McGrew and the Dragon Warrior randomizer community for creating the C++ ROM hacking tool and logic graphs that define modern Dragon Warrior randomization.
*   **[CE C Toolchain Team](https://github.com/CE-Programming/toolchain)**: For building the incredible open-source C compiler, libraries, and graphics tools (`convpng`) that make native development on the TI-84 Plus CE possible. 
*   **Dragon Quest / Dragon Warrior**: Originally developed by Chunsoft and published by Enix (now Square Enix). All sprite art is derivative of the classic NES property.
