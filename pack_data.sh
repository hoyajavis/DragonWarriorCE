#!/bin/bash

echo "========================================="
echo " Dragon Warrior Data Packer"
echo "========================================="

echo "[INFO] Packing Game Data..."
python3 tools/pack_spells.py
if [ $? -ne 0 ]; then exit 1; fi
python3 tools/pack_monsters.py
if [ $? -ne 0 ]; then exit 1; fi
python3 tools/pack_levels.py
if [ $? -ne 0 ]; then exit 1; fi
python3 tools/pack_map.py
if [ $? -ne 0 ]; then exit 1; fi
echo "[INFO] Data packing complete!"
