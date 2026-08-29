#include <atomic>
#include <stdio.h>
#include <vector>
#include <random>
using namespace std;

struct Slot {
    atomic<int> flag;
    vector<int> plan;
    int plan_upcome;
    int env_change;
    int upcome;
    int ai_delay;
    Slot() : flag(-1), plan_upcome(-1), env_change(0), upcome(0), ai_delay(0) {}
};

struct Res { long drops; long stale_drops; long guard_hits; long reissues; };

Res run(mt19937& g, int T, int atk_frame, int atk_amt, int move_delay, bool fixed)
{
    Res R = {0,0,0,0};
    Slot s;
    bool worker_running = false;
    bool land_after_C = false;
    int done_frame = 0;
    bool atk_done = false;
    s.env_change = 1; // piece 1 spawn arms the first call
    for ( int frame = 1; frame <= 600; ++frame ) {
        // B) incoming attack routes into the slot, arms env_change=2
        if ( !atk_done && frame == atk_frame ) {
            s.upcome += atk_amt;
            s.env_change = 2;
            atk_done = true;
        }
        // worker completion, phase A: before C
        if ( worker_running && frame >= done_frame && !land_after_C ) { s.flag = -1; worker_running = false; }
        // C) re-plan block
        if ( s.env_change != 0 && s.flag == -1 ) {
            s.plan.clear();
            s.ai_delay = 0;
            ++R.reissues;
            s.flag = 0;
            s.plan_upcome = s.upcome;
            for ( int i = 0; i < 3; ++i ) s.plan.push_back(0);
            s.plan.push_back(1);
            worker_running = true;
            land_after_C = ( g() & 1 ) != 0;
            done_frame = frame + T;
            s.env_change = 0;
        }
        // worker completion, phase B: after C, before E (lethal)
        if ( worker_running && frame >= done_frame && land_after_C ) { s.flag = -1; worker_running = false; }
        // E) move executor
        bool exec_ok = ( s.flag == -1 );
        if ( fixed ) exec_ok = exec_ok && ( s.env_change == 0 );
        if ( exec_ok && ! s.plan.empty() ) {
            if ( s.ai_delay > 0 ) --s.ai_delay;
            while ( s.flag == -1 && s.ai_delay == 0 && ! s.plan.empty() ) {
                if ( fixed && s.env_change != 0 ) { ++R.guard_hits; break; }
                int mov = s.plan[0];
                s.plan.erase( s.plan.begin() );
                if ( mov == 1 ) {
                    ++R.drops;
                    if ( s.plan_upcome != s.upcome ) ++R.stale_drops;
                    s.upcome = 0;
                    s.env_change = 1;
                    s.ai_delay = move_delay;
                    break;
                } else {
                    s.ai_delay = move_delay;
                }
            }
        }
    }
    return R;
}

int main()
{
    mt19937 g(7u);
    printf("stale% = pieces placed by dll using a plan whose upcomeAtt snapshot was outdated\n");
    for ( int T0 : {2, 8, 24} ) {
        for ( int move_delay : {0, 20} ) {
            long od=0, os=0, fd=0, fs=0, fg=0;
            for ( int trial = 0; trial < 4000; ++trial ) {
                int T = T0 + ( int )( g() % ( T0 > 2 ? 9 : 2 ) );
                int atk_frame = 2 + ( int )( g() % ( T > 3 ? T - 2 : 1 ) );
                int atk_amt = 4 + ( int )( g() % 20 );
                Res a = run(g, T, atk_frame, atk_amt, move_delay, false);
                Res b = run(g, T, atk_frame, atk_amt, move_delay, true);
                od += a.drops; os += a.stale_drops;
                fd += b.drops; fs += b.stale_drops; fg += b.guard_hits;
            }
            printf("T~%2d move=%2d | BEFORE stale %5ld/%6ld (%4.1f%%) | AFTER stale %5ld/%6ld (%4.1f%%) guard %4ld\n",
                T0, move_delay, os, od, 100.0*os/(od?od:1), fs, fd, 100.0*fs/(fd?fd:1), fg);
        }
    }
    return 0;
}
