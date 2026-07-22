@echo off
echo =========================================
echo  Dragon Warrior Data Packer
echo =========================================

echo [INFO] Packing Game Data...
python tools\pack_spells.py
if %errorlevel% neq 0 exit /b 1
python tools\pack_monsters.py
if %errorlevel% neq 0 exit /b 1
python tools\pack_levels.py
if %errorlevel% neq 0 exit /b 1
python tools\pack_map.py
if %errorlevel% neq 0 exit /b 1
echo [INFO] Data packing complete!
