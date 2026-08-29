#include "tetris_ai.h"
#include "gamepool.h"
#include "random.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>

int main()
{
    AI::Random rnd( 987654321u );
    AI::AI_Param ap;
    memset( &ap, 0, sizeof(ap) );
    ap.miny_factor = 13; ap.hole = 9; ap.open_hole = 17; ap.v_transitions = 10; ap.tspin3 = 29;
    ap.clear_efficient = 25; ap.upcomeAtt = 39; ap.h_factor = 2; ap.hole_dis_factor2 = 12;
    ap.hole_dis = 19; ap.hole_dis_factor = 7; ap.tspin = 24; ap.hold_T = 21; ap.hold_I = 16;
    ap.clear_useless_factor = 14; ap.dif_factor = 19; ap.strategy_4w = 0;
    int combo_a[] = {0,0,0,1,1,2};
    AI::setComboList( std::vector<int>(combo_a, combo_a + 6) );
    long calls = 0;
    const int kNextLen = 16;
    for ( int iter = 0; iter < 2500; ++iter ) {
        int h = ( iter % 4 == 0 ) ? 24 : 22;
        AI::GameField pool( 10, (signed char)h );
        int mode = iter % 7;
        for ( int y = 0; y <= h; ++y ) {
            int v;
            if ( mode == 0 ) v = 0;
            else if ( mode == 1 ) v = 1023;
            else if ( mode == 2 ) v = ( int )( rnd.rand() % 1024 );
            else if ( mode == 3 ) v = 1023 & ~( 1 << ( int )( rnd.rand() % 10 ) );
            else if ( mode == 4 ) v = ( ( rnd.rand() & 1 ) ? 1023 : 0 );
            else if ( mode == 5 ) v = ( 15 << ( int )( rnd.rand() % 6 ) );
            else v = ~( 15 << 3 ) & 1023;
            pool.row[y] = v;
        }
        for ( int y = 1; y <= 6; ++y ) pool.row[-y] = ( ( rnd.rand() % 4 ) == 0 ) ? ( int )( rnd.rand() % 1024 ) : 0;
        pool.b2b = (signed short)( iter % 90 );
        pool.combo = (signed short)( iter % 90 );
        pool.m_hold = (int)( rnd.rand() % 8 );
        std::vector<AI::Gem> next;
        for ( int i = 0; i < kNextLen; ++i ) next.push_back( AI::getGem( 1 + (int)( rnd.rand() % 7 ), 0 ) );
        if ( iter % 13 == 0 ) { next.clear(); next.push_back( AI::getGem(2,0) ); }
        int cn = 1 + (int)( rnd.rand() % 7 );
        int sp = (int)( rnd.rand() % 4 );
        int x = 3, y = 1;
        if ( iter % 17 == 0 ) { x = -4; }
        if ( iter % 29 == 0 ) { x = 9; y = h - 1; sp = 1; }
        if ( iter % 31 == 0 ) { y = h; }
        AI::setSoftdrop( ( iter % 5 ) != 0 );
        AI::setAllSpin( ( iter % 3 ) == 0 );
        int lvl = iter % 11;
        int deep = iter % 9;
        int sd = 0;
        AI::MovingSimple best = AI::AISearch( ap, pool, ( iter % 6 == 0 ) ? 1 : 0, AI::getGem(cn, sp), x, y, next,
            ( iter % 4 ) != 0, (int)( rnd.rand() % 30 ), deep, sd, lvl, iter % 2 );
        if ( best.x != AI::MovingSimple::INVALID_POS ) {
            std::vector<AI::Moving> pathmovs;
            AI::FindPathMoving( pool, pathmovs, AI::getGem(cn, sp), x, y, false );
        }
        std::vector<AI::MovingSimple> smovs;
        AI::GenMoving( pool, smovs, AI::getGem(cn, sp), x, y, ( iter % 10 ) == 0 );
        ++calls;
    }
    printf("app ai stress ok, calls=%ld\n", calls);
    return 0;
}
