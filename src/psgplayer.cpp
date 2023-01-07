#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <libgen.h> // for dirname()
#include <unistd.h>

#include "psg.h"
#include "psgplayer.h"

static u_int16_t DATA[] = {
    0x01ac,   //c4
    0x00d6,   //c5
    0x00e3,   //b4
    0x00d6,   //c5
    0x00aa,   //e5
    0x00d6,   //c5
    0x00e3,   //b4
    0x00d6,   //c5
};
static u_int16_t DATA2[] = {
    0x0153,   //e4
    0x00aa,   //e5
    0x00be,   //d5
    0x00aa,   //e5
    0x008f,   //g5
    0x00aa,   //e5
    0x00be,   //d5
    0x00aa,   //e5
};

int main() {
    PSGEmulator* psg = new PSGEmulator;
    psg->open();
    psg->play();

    int data = 0x00;
    psg->write(0x07, 0xf0);

    psg->write(0x08, 0x1f);
    psg->write(0x09, 0x1f);
    psg->write(0x0a, 0x1f);
    u_int8_t detune = 0;
    u_int8_t env = 0x010;

    while(1) {

        u_int16_t a = DATA[data%8];
        u_int16_t b = DATA[data%8] + detune % 16;
        u_int16_t c = DATA[data%8] - detune % 16;
        detune = (data / 128) % 8;
        env    = (data / 16) & 0x0f;
        psg->write(6,data /8);    // Noise
        psg->write(11,0x00);
        psg->write(12,0x04);
        printf("PSG: %04x %04x (%3d) = %d : detune:%d env:%d\n", 
        a, b, b-a, data, detune, env);
        psg->write(0, a &0xff);
        psg->write(1, (a>>8) &0xff);
        psg->write(2, b &0xff);
        psg->write(3, (b>>8) &0xff);
        psg->write(4, c &0xff);
        psg->write(5, (c>>8) &0xff);
        psg->write(13, env);
        data++;
        usleep(100000/1);
    }
    psg->close();
    delete psg;

    return 0;
}