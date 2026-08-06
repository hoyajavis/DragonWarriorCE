# Dragon Warrior CE (TI-84 Plus CE Demake & Native Randomizer)

A native C port, demake, and full-featured randomizer engine for the classic *Dragon Warrior* experience, designed specifically to run on the **TI-84 Plus CE** graphing calculator.

This project recreates the full NES *Dragon Warrior* RPG experience on the calculator using the CE C Toolchain. It features a custom engine capable of parsing XML map data into optimized AppVars, a full bytecode interpreter for dialogue and event scripting, classic turn-based combat, and a **native 32-bit randomizer engine** with zero Flash memory wear.

---

## Features

### 🎲 Native 32-Bit Randomizer Engine
*   **Dynamic Seed Generation**: Features a dedicated Seed Entry screen pre-filled with a random 32-bit seed derived from the calculator's Real-Time Clock (`rtc_Time()`). Use the number keys `[0]`–`[9]` to enter a custom seed or press `[WINDOW]` to re-roll!
*   **Modulo-Bias-Free PRNG**: Powered by `Xorshift32` with 64-bit multiply-shift scaling (`((uint64_t)next_prng() * range) >> 32`), completely eliminating software integer division/modulo cycles on the eZ80.
*   **Customizable Rando Flags**:
    *   **Shuffle Key Items & Quests**: Shuffles chest and quest item rewards.
    *   **Shuffle Monster Encounter Zones**: Scrambles overworld and dungeon monster sets.
    *   **Scale Monster Stats**: Applies $0.75\times$–$1.25\times$ variance to monster HP, Attack, Defense, XP, and Gold.
    *   **Shuffle Hero Stat Growth**: Scrambles HP, MP, Strength, and Agility growth curves across level-ups.
    *   **Shuffle Town Shops**: Shuffles shop inventories and item prices.
    *   **QoL Modifiers**: Toggle 2x EXP & Gold, 50% Encounter Rate, Instant Text, and Guaranteed Flee/Run.
*   **Presets**: Includes `Standard Rando`, `Speedrunner QoL`, `Chaos Mode`, `Vanilla DW`, and `Custom` flag configurations.
*   **Key Economy Guarantee**: Enforces Tier 0 key/key shop availability rules to ensure 100% winnable seeds without softlocking.
*   **Zero Flash Wear & 1.2 KB RAM Overlay**: Master data remains safely in ROM/AppVars. Seed initialization builds a lightweight ~1.2 KB RAM overlay in < 2ms without writing to Flash memory.

### 🎮 Classic RPG Engine Features
*   **Faithful Map Recreation**: A full parsing pipeline that converts classic Dragon Warrior map layouts (towns, castles, caves, and the overworld) into highly compressed memory structures, complete with line-of-sight cave darkness masking!
*   **Dialogue Bytecode Interpreter**: A custom virtual machine built in C that interprets packed binary dialogue scripts. Supports branching dialogue, conditional checks (`HAS_ITEM`, `LACKS_ITEM`), item distribution, game state alteration, and shop menus.
*   **Turn-Based Combat**: Classic RPG combat mechanics featuring a scaling level system, experience points, magic points, complex status effects (Sleep/Stopspell), authentic fleeing formulas, and scripted multi-stage boss encounters (including the Dragonlord's True Form!).
*   **Spells & Items**: Consume Herbs to heal, use the SEARCH command to find hidden quest items, or cast spells like HEAL, HURT, SLEEP, STOPSPELL, OUTSIDE, RETURN, RADIANT, and REPEL.
*   **Persistent Save System**: A robust three-slot save system with a `0x4457` magic signature and 16-bit CRC checksum stored in calculator AppVars. Save slots seamlessly regenerate randomized RAM tables upon load.

### ⚡ Highly Optimized Decoupled Architecture
*   **Modular Architecture**:
    *   `src/main.c`: Minimal high-level frame loop (< 70 lines).
    *   `src/exploring.c`: Overworld walking, tile collisions, map warps, and object interactions.
    *   `src/combat.c`: Complete battle state machine engine and combat UI.
    *   `src/menu.c`: Application menus, seed/flag entry, inventory/spells UI, and vendor shops.
    *   `src/hero.c`: Hero stats, inventory storage, stat growth, and equipment math.
    *   `src/randomizer.c`: Fast PRNG engine and RAM overlay tables.
    *   `src/map.c`: Tilemap engine and collision checks.
    *   `src/effects.c`: Palette fade transitions.
    *   `src/input.c`: Keypad state tracking.
*   **eZ80 Assembly & C Optimizations**: Utilizes fast bit-shift arithmetic (`>> 4`, `<< 4`, `& 15`), LUT bit masking (`bit_mask[i]`), pointer-advance loops (`obj += 6`, `z += 5`), and `NoClip` sprite rendering routines for maximum execution speed on the 48 MHz eZ80 processor.

---

## Controls

*   **D-Pad Arrow Keys**: Move the Hero across the map / Navigate menus.
*   **[2nd]** or **[ENTER]**: Confirm menu selection / Interact with objects / Attack in combat.
*   **[ALPHA]**: Open the Command Menu / Go back / Acknowledge pop-up messages.
*   **[WINDOW]**: Re-roll seed on the Seed Entry menu.
*   **[CLEAR]**: Instantly quit the game.

*Note: To SEARCH chests or SAVE your game, press **[ALPHA]** while exploring to open the Command Menu.*

---

## Installation & Compilation

To build this project from source code, you need the [CE C Toolchain](https://github.com/CE-Programming/toolchain) installed on your system.

### Compiling the Source Code

The build pipeline is strictly separated into two distinct steps: Data Packing and C Compilation.

#### 1. Data Packing (Python)
If you modify XML map files, dialogue scripts, or level/monster definitions inside `data/`, pack them into binary AppVars:
*   **Windows**: Double-click or run `pack_data.bat`.
*   **Mac/Linux**: Run `./pack_data.sh`.

#### 2. C Compilation (Make)
To compile the C code into the final executable game:
*   **Windows**: Double-click or run `compile.bat`.
*   **Mac/Linux**: Run `./compile.sh`.

Output files will be generated in the `bin/` directory:
*   `PYDW.8xp`: Main program executable launcher.
*   `PYDW0.8xv` & `PYDW1.8xv`: Game map and dialogue data AppVars.

### Testing in an Emulator (CEmu)

1. Download and install [CEmu](https://cemu.app/), the standard open-source TI-84 Plus CE emulator.
2. Drag and drop `bin/PYDW.8xp`, `bin/PYDW0.8xv`, and `bin/PYDW1.8xv` into the emulator window.
3. Press `[PRGM]` on the emulated keypad, select `PYDW`, and press `[ENTER]` to start playing!

### Running on Physical Hardware

1. Connect your calculator to your PC using a Mini-USB cable.
2. Open [TI Connect CE](https://education.ti.com/en/products/computer-software/ti-connect-ce-sw) or [TILP](http://lpg.ticalc.org/prj_tilp/).
3. Send `PYDW.8xp`, `PYDW0.8xv`, and `PYDW1.8xv` to your calculator's RAM/Archive.
4. Press `[PRGM]`, select `PYDW`, and press `[ENTER]`!

---

## Credits & Acknowledgements

*   **[hoyajavis/pydw-randomizer](https://github.com/hoyajavis/pydw-randomizer)**: The original Python project that this demake was initially based upon.
*   **[justinbeetle/pyDragonWarrior](https://github.com/justinbeetle/pyDragonWarrior)**: Justin Beetle and contributors for creating the original, highly-faithful recreation of Dragon Warrior in Python/Pygame.
*   **[mcgrew/dwrandomizer](https://github.com/mcgrew/dwrandomizer)**: McGrew and the Dragon Warrior randomizer community for creating the original ROM hacking tool and logic graphs that define modern Dragon Warrior randomization.
*   **[CE C Toolchain Team](https://github.com/CE-Programming/toolchain)**: For building the open-source C compiler, libraries, and graphics tools (`convpng`) for native TI-84 Plus CE development.
*   **Dragon Quest / Dragon Warrior**: Originally developed by Chunsoft and published by Enix (now Square Enix). All sprite art is derivative of the classic NES property.
