#include "ai.h"
#include <stdio.h>
#include <string.h>
#include <random>

extern "C" char* TetrisAI(int overfield[], int field[], int field_w, int field_h, int b2b, int combo,
               char next[], char hold, bool curCanHold, char active, int x, int y, int spin,
               bool canhold, bool can180spin, int upcomeAtt, int comboTable[], int maxDepth, int level, int player);

int main()
{
    std::mt19937 g(777u);
    int field[64], overfield[64], comboTable[64];
    char next[64];
    int diff = 0, total = 0, placeable = 0;
    for ( int iter = 0; iter < 800; ++iter ) {
        int stackh = 2 + ( int )( g() % 8 );
        for ( int i = 0; i <= 22; ++i ) {
            if ( i > 22 - stackh ) {
                int row = 0;
                for ( int b = 0; b < 10; ++b ) if ( (int)( g() % 100 ) < 45 ) row |= 1 << b;
                field[i] = row;
            } else {
                field[i] = 0;
            }
        }
        for ( int i = 0; i < 8; ++i ) overfield[i] = 0;
        for ( int i = 0; i < 64; ++i ) next[i] = " ITLJZSO"[ 1 + g() % 7 ];
        for ( int i = 0; i < 64; ++i ) comboTable[i] = -1;
        comboTable[0] = 0; comboTable[1] = 1; comboTable[2] = 2; comboTable[3] = -1;
        char active = "ITLJZSO"[ g() % 7 ];
        int up1 = 12 + ( int )( g() % 14 );
        char sa[128], sb[128];
        strncpy(sa, TetrisAI(overfield, field, 10, 22, 1, 0, next, ' ', true, active, 3, 1, 0, true, false, 0,  comboTable, 2, 2, 0), 127); sa[127] = 0;
        strncpy(sb, TetrisAI(overfield, field, 10, 22, 1, 0, next, ' ', true, active, 3, 1, 0, true, false, up1, comboTable, 2, 2, 0), 127); sb[127] = 0;
        ++total;
        if ( strcmp(sa, " V") != 0 ) ++placeable;
        if ( strcmp(sa, sb) != 0 ) ++diff;
    }
    printf("plans placeable: %d/%d, upcome-sensitive: %d changed when upcomeAtt 0 -> +12..25\n", placeable, total, diff);
    return ( diff > 0 ) ? 0 : 1;
}
