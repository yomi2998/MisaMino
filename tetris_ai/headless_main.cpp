#include "tetris_ai.h"
#include "gamepool.h"
#include "random.h"
#include <atomic>
#include <stdio.h>
#include <string.h>
#include <vector>

int main()
{
    AI::Random rnd( 1234567u );
    AI::AI_Param ap;
    memset( &ap, 0, sizeof(ap) );
    ap.miny_factor = 13; ap.hole = 9; ap.open_hole = 17; ap.v_transitions = 10; ap.tspin3 = 29;
    ap.clear_efficient = 25; ap.upcomeAtt = 39; ap.h_factor = 2; ap.hole_dis_factor2 = 12;
    ap.hole_dis = 19; ap.hole_dis_factor = 7; ap.tspin = 24; ap.hold_T = 21; ap.hold_I = 16;
    ap.clear_useless_factor = 14; ap.dif_factor = 19; ap.strategy_4w = 0;
    int combo_a[] = {0,0,0,1,1,2};
    AI::setComboList( std::vector<int>(combo_a, combo_a + 6) );
    unsigned long long checksum = 0;
    for ( int iter = 0; iter < 300; ++iter ) {
        AI::GameField pool( 10, 22 );
        for ( int y = 0; y <= 22; ++y ) pool.row[y] = (int)( rnd.rand() % 1024 );
        pool.b2b = (signed short)( iter % 9 );
        pool.combo = (signed short)( iter % 9 );
        pool.m_hold = (int)( rnd.rand() % 8 );
        std::vector<AI::Gem> next;
        for ( int i = 0; i < 8; ++i ) next.push_back( AI::getGem( 1 + (int)( rnd.rand() % 7 ), 0 ) );
        int cn = 1 + (int)( rnd.rand() % 7 );
        int sp = (int)( rnd.rand() % 4 );
        int deep = iter % 6;
        int sd = 0;
        std::atomic<int> flag;
        AI::Moving mov;
        AI::RunAI( mov, flag, ap, pool, 0, AI::getGem(cn, sp), 3, 1, next, true,
            (int)( rnd.rand() % 20 ), deep, sd, iter % 11, 0 );
        while ( flag != -1 ) { }
        for ( size_t i = 0; i < mov.movs.size(); ++i ) {
            checksum = checksum * 131u + (unsigned)mov.movs[i];
        }
    }
    printf("headless ok, checksum=%llu\n", checksum);
    return 0;
}
