#include "ai.h"
#include <stdio.h>
extern "C" char* TetrisAI(int overfield[], int field[], int field_w, int field_h, int b2b, int combo,
               char next[], char hold, bool curCanHold, char active, int x, int y, int spin,
               bool canhold, bool can180spin, int upcomeAtt, int comboTable[], int maxDepth, int level, int player);
int main(int argc, char** argv)
{
    int field[64] = {0}, overfield[64] = {0}, comboTable[64];
    char next[64];
    int h = 20;
    for ( int i = 0; i <= h; ++i ) field[i] = 0;
    for ( int i = 0; i < 64; ++i ) next[i] = 'T';
    for ( int i = 0; i < 64; ++i ) comboTable[i] = -1;
    if ( argc > 1 && argv[1][0] == '1' ) { comboTable[0] = 0; comboTable[1] = -1; }
    printf("calling\n"); fflush(stdout);
    char* r = TetrisAI(overfield, field, 10, h, 0, 0, next, ' ', true, 'T', 3, 1, 0, true, false, 0, comboTable, 2, 2, 0);
    printf("ok result=[%s]\n", r);
    return 0;
}
