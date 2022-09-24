#include <ti/screen.h>
#include <sys/rtc.h>
#include <sys/util.h>
#include <stdlib.h>
#include <sys/timers.h>

/*
 *--------------------------------------
 * Program Name: Left or Right?
 * Author: jmibo
 * License: MIT
 * Description: the worst game, now on your calculator!
 *--------------------------------------
*/

#include <graphx.h>
#include <keypadc.h>

#include "gfx/gfx.h"

void begin(void);
void end(void);
bool step(void);
void draw(void);
void init(void);
void debug_keys(void);

typedef struct {
    uint8_t down;
    uint8_t held;
    uint8_t up;
    uint8_t last;
} arrows;

void update(arrows *a, uint8_t d) {
    a->held = d;
    a->down = a->held & ~a->last;
    a->up = ~a->held & a->last;
    a->last = a->held;
}

uint8_t x = 0, y = 0, z = 0;
arrows a = { };

uint16_t seq;
int16_t cover = -200; // cover = (% screen covered) * 60

uint8_t offset = 0;
bool last = 0;

bool gameover = 0;
bool anim_complete = 0;

uint16_t score = 0;

void shiftseq() {
    seq = (seq >> 1) | (
        ((seq >> 0) ^ (seq >> 2) ^ (seq >> 3) ^ (seq >> 4)) << 15
    );
}

int main(void)
{
    bool partial_redraw = false;

    /* No rendering allowed! */
    begin();

    /* Initialize graphics drawing */
    gfx_Begin();

    gfx_SetPalette(pal, sizeof_pal, 0);
    gfx_SetTransparentColor(0);

    /* Draw to the buffer to avoid rendering artifacts */
    gfx_SetDrawBuffer();

    /* No rendering allowed in step! */
    while (step())
    {
        /* Only want to redraw part of the previous frame? */
        if (partial_redraw)
        {
            /* Copy previous frame as a base for this frame */
            gfx_BlitScreen();
        }

        /* As little non-rendering logic as possible */
        draw();

        /* Queue the buffered frame to be displayed */
        gfx_SwapDraw();
    }

    /* End graphics drawing */
    gfx_End();
    end();

    return 0;
}

/* Implement me! */
void begin(void)
{
    kb_SetMode(MODE_3_CONTINUOUS);
    kb_EnableOnLatch();
    kb_ClearOnLatch();

    srandom(rtc_Time());
    seq = random();
}

void init(void) {
    seq = random();
    cover = -200;
    offset = 0;
    last = 0;
    gameover = 0;
    anim_complete = 0;
    score = 0;
}

/* Implement me! */
void end(void)
{
    kb_DisableOnLatch();
}

/* Implement me! */
bool step(void)
{
    update(&a, kb_Data[7]);
    if(!gameover) {
        if(a.down & (kb_Left | kb_Right)) {
            bool r = !!(a.down & kb_Right);
            if(r != (seq & 1)) { gameover = true; return true; }
            ++score;
            last = r;
            offset = 4;
            shiftseq();
        }
        offset -= (offset != 0 ? 1 : 0);

        ++cover;
        if(cover > 5400) cover = 5400;
    } else if(!anim_complete) {
        cover += 180;
        if(cover > 6000) anim_complete = true;
    } else {
        if(a.down & (kb_Left | kb_Right)) {
            init();
        }
    }

    return !kb_On; // quit on ON key
}

/* Implement me! */
void draw(void)
{
    gfx_FillScreen(2);

    #ifdef debug
    debug_keys();
    #endif

    for(uint8_t i = 0; i < 10; ++i) {
        gfx_TransparentSprite(((seq >> i) & 1) ? right : left, 148, i * 24 + offset * 6);
    }

    if(offset > 0) {
        uint16_t a = offset * 40;
        gfx_TransparentSprite(last ? right : left, last ? 320 - a : a, 0);
    }

    gfx_SetColor(1);
    gfx_FillRectangle(0, 0, 320, cover * 240 / 6000);

    gfx_FillRectangle(0, 0, 50, 18);

    gfx_SetTextBGColor(1);
    gfx_SetTextFGColor(2);
    gfx_SetTextXY(2, 2);
    gfx_SetTextScale(2, 2);
    gfx_PrintUInt(score, 3);

    if(anim_complete) {
        gfx_SetTextBGColor(1);
        gfx_SetTextFGColor(0);
        gfx_SetTextScale(4, 4);
        gfx_SetTextXY(29, 104);
        gfx_PrintString("game over");
    }
}

void debug_keys() {
    gfx_SetColor(1);

    for(uint8_t i = 0; i < 8; ++i) {
        if((a.down >> i) & 1) {
            gfx_FillRectangle(i * 30, 0, 30, 30);
        }
    }
    for(uint8_t i = 0; i < 8; ++i) {
        if((a.held >> i) & 1) {
            gfx_FillRectangle(i * 30, 30, 30, 30);
        }
    }
    for(uint8_t i = 0; i < 8; ++i) {
        if((a.up >> i) & 1) {
            gfx_FillRectangle(i * 30, 60, 30, 30);
        }
    }
}