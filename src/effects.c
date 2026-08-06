#include "effects.h"
#include "gfx/gfx.h"
#include <graphx.h>
#include <tice.h>

// Single working buffer: 512 bytes instead of fade_palettes[9][256] = 4,608 bytes
static uint16_t fade_buf[256];

static void applyFadeStep(int step) {
    uint16_t *pal = (uint16_t*)(void*)global_palette;
    for (int i = 0; i < 256; i++) {
        uint16_t c = pal[i];
        uint8_t r = ((c >> 10) & 31) * step / 8;
        uint8_t g = ((c >> 5) & 31) * step / 8;
        uint8_t b = (c & 31) * step / 8;
        fade_buf[i] = (r << 10) | (g << 5) | b;
    }
    gfx_SetPalette(fade_buf, sizeof_global_palette, 0);
    delay(20);
}

void fadeToBlack(void) {
    for (int step = 8; step >= 0; step--) {
        applyFadeStep(step);
    }
}

void fadeInFromBlack(void) {
    for (int step = 0; step <= 8; step++) {
        applyFadeStep(step);
    }
    // Restore 100% original full palette
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
}
