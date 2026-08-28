// dllai.cpp : Defines the exported functions for the DLL application.
//

#define _CRT_SECURE_NO_WARNINGS
#include "ai.h"
#include <vector>
#include <map>
#include <string>
#include <cstring>

#ifdef _WIN32
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT extern "C" __attribute__((visibility("default")))
#endif

char g_result[8][1024];
AI::MovingSimple last_best[8];

char* result(int player, std::string r)
{
    if ( player < 0 || player >= 8 ) player = 0;
    strcpy( g_result[player], r.c_str() );
    return g_result[player];
}

// very important value
#define AI_DLL_VERSION 2

DLLEXPORT
int AIDllVersion()
{
    return AI_DLL_VERSION;
}

DLLEXPORT
char* AIName( int level ) {
    char name[200];
    if ( level > AI_MAX_DEPTH ) level = AI_MAX_DEPTH;
    sprintf( name, "LV%d", level);
    return result( 0, std::string("Baka(9) ") + name );
}

/*
all 'char' type is using the characters in ' ITLJZSO'

field data like this:
00........   -> 0x3
00.0......   -> 0xb
00000.....   -> 0x1f

b2b: the count of special attack, the first one set b2b=1, but no extra attack. Have extra attacks when b2b>=2
combo: first clear set combo=1, so the comboTable in toj rule is [0, 0, 0, 1, 1, 2, 2, 3, ...]
next: array size is 'maxDepth'
x, y, spin: the active piece's x/y/orientation,
    x/y is the up-left corner's position of the active piece.
    see tetris_gem.cpp for the bitmaps.
curCanHold: indicates whether you can use hold on current move.
    might be caused by re-think after a hold move.
canhold: false if hold is completely disabled.
comboTable: -1 is the end of the table.
*/
DLLEXPORT
char* TetrisAI(int overfield[], int field[], int field_w, int field_h, int b2b, int combo,
               char next[], char hold, bool curCanHold, char active, int x, int y, int spin,
               bool canhold, bool can180spin, int upcomeAtt, int comboTable[], int maxDepth, int level, int player)
{
    if ( player < 0 || player >= 8 ) player = 0;
    if ( field_w < 3 || field_w > 22 ) field_w = 10;
    if ( field_h < 1 || field_h > AI_POOL_MAX_H - AI::gem_add_y - 2 ) field_h = 20;
    if ( spin < 0 || spin > 3 ) spin = 0;
    if ( x < -6 || x > 18 ) x = AI::gem_beg_x;
    if ( y < -15 || y > 25 ) y = AI::gem_beg_y;
    if ( maxDepth < 0 ) maxDepth = 0;
    if ( maxDepth > 32 ) maxDepth = 32;
    AI::GameField gamefield(field_w, field_h); // always 10 x 22
    std::vector<AI::Gem> gemNext;
    std::map<char, int> gemMap;
    // init
    gemMap[' '] = AI::GEMTYPE_NULL;
    gemMap['I'] = AI::GEMTYPE_I;
    gemMap['T'] = AI::GEMTYPE_T;
    gemMap['L'] = AI::GEMTYPE_L;
    gemMap['J'] = AI::GEMTYPE_J;
    gemMap['Z'] = AI::GEMTYPE_Z;
    gemMap['S'] = AI::GEMTYPE_S;
    gemMap['O'] = AI::GEMTYPE_O;
    // gamefield
    if ( field ) {
        for ( int iy = 0; iy <= field_h; ++iy ) {
            gamefield.row[iy] = field[iy];
        }
    }
    if ( overfield ) {
        for ( int iy = 0; iy < 8; ++iy ) {
            gamefield.row[-iy-1] = overfield[iy];
        }
    }
    gamefield.m_hold = gemMap[hold];
    gamefield.b2b = b2b;
    gamefield.combo = combo;
    // next
    if ( next ) {
        for ( int i = 0; i < maxDepth; ++i) {
            int ng = gemMap[next[i]];
            if ( ng <= 0 || ng > 7 ) break;
            gemNext.push_back( AI::getGem(ng, 0) );
        }
    }
    // combo list
    if ( comboTable ) {
        for ( int i = 0; i < 100; ++i) {
            if ( comboTable[i] < 0 ) {
                int n = ( i > 0 ? i : 0 );
                AI::setComboList( std::vector<int>(comboTable, comboTable + n) );
                break;
            }
        }
    }
    // rule
    AI::setSpin180(can180spin);

    if ( level > AI_MAX_DEPTH ) level = AI_MAX_DEPTH;
    if ( level < 0 ) level = 0;

    // player is the player number, for randomize or something else, to make different player do the different thing if it is necessary
    int active_num = gemMap[active];
    AI::MovingSimple best;
    if ( active_num >= 1 && active_num <= 7 ) {
        best = AI::AISearch(gamefield, AI::getGem(active_num, spin), curCanHold, x, y, gemNext, canhold, upcomeAtt, level, player);
    }

    // find path
    AI::Moving mov;
    if ( best.x != AI::MovingSimple::INVALID_POS ) {
        int hold_num = gamefield.m_hold;
        if ( gamefield.m_hold == 0 && ! gemNext.empty()) {
            hold_num = gemNext[0].num;
        }
        std::vector<AI::Moving> movs;
        AI::Gem cur = AI::getGem(0, 0);
        if ( best.hold && hold_num >= 1 && hold_num <= 7 ) {
            cur = AI::getGem(hold_num, 0);
            FindPathMoving(gamefield, movs, cur, AI::gem_beg_x, AI::gem_beg_y, true);
        } else if ( ! best.hold ) {
            cur = AI::getGem(active_num, spin);
            FindPathMoving(gamefield, movs, cur, x, y, false);
        }
        for ( size_t i = 0; i < movs.size(); ++i ) {
            if ( movs[i].x == best.x && movs[i].y == best.y && movs[i].spin == best.spin ) {
                if ( cur.num == AI::GEMTYPE_T && movs[i].wallkick_spin >= best.wallkick_spin || cur.num != AI::GEMTYPE_T ) {
                    mov = movs[i];
                }
            }
        }
    }
    if ( mov.movs.empty() ) {
        mov.movs.clear();
        mov.movs.push_back( AI::Moving::MOV_NULL );
        mov.movs.push_back( AI::Moving::MOV_DROP );
    } else {
        last_best[player] = best;
    }

    // return
    std::map<int, char> outMap;
    outMap[AI::Moving::MOV_NULL] = ' ';
    outMap[AI::Moving::MOV_L] = 'l';
    outMap[AI::Moving::MOV_R] = 'r';
    outMap[AI::Moving::MOV_LL] = 'L';
    outMap[AI::Moving::MOV_RR] = 'R';
    outMap[AI::Moving::MOV_D] = 'd';
    outMap[AI::Moving::MOV_DD] = 'D';
    outMap[AI::Moving::MOV_LSPIN] = 'z';
    outMap[AI::Moving::MOV_SPIN2] = 'x';
    outMap[AI::Moving::MOV_RSPIN] = 'c';
    outMap[AI::Moving::MOV_HOLD] = 'v';
    outMap[AI::Moving::MOV_DROP] = 'V';
    std::string out;
    for ( size_t i = 0; i < mov.movs.size(); ++i ) {
        out.push_back(outMap[mov.movs[i]]);
    }
    return result(player, out);
}
