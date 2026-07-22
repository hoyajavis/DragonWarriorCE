#!/bin/bash
echo "========================================="
echo " Dragon Warrior Build Script"
echo "========================================="
echo ""

# Check if make is installed
if ! command -v make &> /dev/null; then
    echo "[ERROR] 'make' could not be found. Please ensure CE C Toolchain is installed and in your PATH."
    exit 1
fi

echo "[STEP 1] Compiling Graphics (make gfx)..."
make gfx
if [ $? -ne 0 ]; then
    echo "[ERROR] Graphics compilation failed!"
    exit 1
fi

echo ""
echo "[STEP 2] Building Project (make)..."
make
if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed!"
    exit 1
fi

echo ""
echo "========================================="
echo " SUCCESS! "
echo " Your game is ready: bin/PYDW.8xp"
echo "========================================="
