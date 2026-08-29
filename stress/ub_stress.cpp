#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <random>

extern "C" char* TetrisAI(int overfield[], int field[], int field_w, int field_h, int b2b, int combo,
               char next[], char hold, bool curCanHold, char active, int x, int y, int spin,
               bool canhold, bool can180spin, int upcomeAtt, int comboTable[], int maxDepth, int level, int player);

static const char* gemchars = " ITLJZSO";

static void fill_field(int* field, int* overfield, int h, std::mt19937& g)
{
    std::uniform_int_distribution<int> d(0, 1023);
    std::uniform_int_distribution<int> half(0, 1);
    for ( int i = 0; i <= h; ++i ) {
        int mode = d(g) % 5;
        if ( mode == 0 ) field[i] = 1023;
        else if ( mode == 1 ) field[i] = half(g) ? 1023 : 0;
        else if ( mode == 2 ) field[i] = 1023 & ~( 1 << ( d(g) % 10 ) );
        else field[i] = d(g) & 1023;
    }
    for ( int i = 0; i < 8; ++i ) overfield[i] = ( d(g) % 3 == 0 ) ? ( d(g) & 1023 ) : 0;
}

int main()
{
    std::mt19937 g(12345u);
    int field[64], overfield[64], comboTable[64];
    char next[64];
    long calls = 0;
    for ( int iter = 0; iter < 4000; ++iter ) {
        int field_w = 10;
        int field_h = 20 + ( iter % 3 );
        fill_field(field, overfield, field_h, g);
        int maxDepth = iter % 9;
        for ( int i = 0; i < 64; ++i ) {
            int r = (int)(g() % 11);
            next[i] = ( r < 8 ) ? gemchars[r] : ' ';
        }
        for ( int i = 0; i < 64; ++i ) comboTable[i] = -1;
        int ct = (int)( g() % 7 );
        for ( int i = 0; i < ct; ++i ) comboTable[i] = (int)( g() % 5 );
        comboTable[ct] = -1;
        std::uniform_int_distribution<int> big(0, 12);
        int r = big(g);
        int x = 3, y = 1, spin = (int)( g() % 4 ), level = (int)( g() % 11 );
        int b2b = (int)( g() % 70 );
        int combo = (int)( g() % 70 );
        int hold = (int)( g() % 8 );
        int active = 1 + (int)( g() % 7 );
        bool canhold = ( g() & 1 ) != 0;
        bool curCanHold = ( g() & 1 ) != 0;
        bool spin180 = ( g() & 1 ) != 0;
        int upcomeAtt = (int)( g() % 40 );
        if ( r == 4 ) { x = -6; }
        if ( r == 5 ) { x = 18; y = 25; }
        if ( r == 6 ) { spin = 3; y = 22; }
        if ( r == 7 ) { x = -2; spin = 2; }
        if ( r == 8 ) { combo = 32000; b2b = 32000; }
        char actc = gemchars[active];
        char holdc = ( hold == 0 ) ? ' ' : gemchars[hold];
        char* res = TetrisAI(overfield, field, field_w, field_h, b2b, combo,
            next, holdc, curCanHold, actc, x, y, spin,
            canhold, spin180, upcomeAtt, comboTable, maxDepth, level, (int)( g() % 8 ));
        if ( res == NULL ) { printf("NULL result at iter %d\n", iter); return 2; }
        if ( strlen(res) > 4096 ) { printf("overrun at iter %d\n", iter); return 3; }
        ++calls;
    }
    for ( int iter = 0; iter < 400; ++iter ) {
        int field_w = 10, field_h = 22;
        for ( int i = 0; i <= field_h; ++i ) field[i] = 1023;
        for ( int i = 0; i < 8; ++i ) overfield[i] = 0;
        for ( int i = 0; i < 64; ++i ) next[i] = 'T';
        for ( int i = 0; i < 64; ++i ) comboTable[i] = -1;
        int spin = iter % 4;
        char* res = TetrisAI(overfield, field, field_w, field_h, 3, 5,
            next, ' ', true, 'T', 3, 1, spin,
            true, true, 0, comboTable, 6, 2, 1);
        if ( res == NULL ) { printf("NULL result in wall test %d\n", iter); return 2; }
        ++calls;
    }
    printf("stress ok, calls=%ld\n", calls);
    return 0;
}
