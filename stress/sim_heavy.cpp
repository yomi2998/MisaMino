#include <atomic>
#include <stdio.h>
#include <vector>
#include <numeric>
using namespace std;

struct Slot {
    atomic<int> flag;
    vector<int> plan;
    int env_change;
    int plan_started;
    int plan_replans;
    int snapshot_upcome;
    int upcome;
    int ai_delay;
    Slot() : flag(-1), env_change(0), plan_started(0), plan_replans(0), snapshot_upcome(0), upcome(0), ai_delay(0) {}
};

struct Res {
    long drops, stale_drops, midplan_requeries, preempts, replan_thrash_stops, frames;
};

// mode: 0 = HEAD (no env gate at all), 1 = previous round's fix (blanket env gate), 2 = current fix
Res run(int T, int move_delay, const vector<int>& atk_frames, const vector<int>& atk_amts, int mode)
{
    Res R = {0,0,0,0,0,0};
    Slot s;
    bool worker_running = false;
    int done_frame = 0;
    bool land_after_C = false;
    size_t next_atk = 0;
    bool new_piece_pending = true; // piece 1 spawn
    for ( int frame = 1; frame <= 20000; ++frame ) {
        ++R.frames;
        // ---- block B: new piece spawn + attack arrival ----
        if ( new_piece_pending ) {
            new_piece_pending = false;
            s.env_change = 1;
            s.plan_started = 0;
            s.plan_replans = 0;
        }
        if ( next_atk < atk_frames.size() && frame == atk_frames[next_atk] ) {
            s.upcome += atk_amts[next_atk];
            if ( mode >= 1 ) s.env_change = 2; // previous/current fix arms unconditionally
            ++next_atk;
        }
        // worker completion, phase before C
        if ( worker_running && frame >= done_frame && !land_after_C ) { s.flag = -1; worker_running = false; }
        // ---- block C: re-plan ----
        if ( s.env_change && s.flag == -1 ) {
            bool may_fire;
            if ( mode == 2 ) may_fire = ( s.env_change != 2 || ( s.plan_started == 0 && s.plan_replans < 2 ) );
            else if ( mode == 3 ) may_fire = ( s.env_change != 2 || s.plan_replans < 2 );
            else may_fire = true;
            if ( may_fire ) {
                s.plan.clear();
                s.ai_delay = 0;
                s.flag = 0;
                s.snapshot_upcome = s.upcome;
                for ( int i = 0; i < 5; ++i ) s.plan.push_back(0);
                s.plan.push_back(1);          // 5 moves then MOV_DROP
                if ( mode >= 2 && s.env_change == 2 && ( mode == 3 || s.plan_started == 0 ) ) {
                    ++s.plan_replans;
                    ++R.preempts;
                }
                worker_running = true;
                land_after_C = true;          // worst case: always lands after C
                done_frame = frame + T;
            }
            s.env_change = 0;
        }
        // worker completion, lethal phase: after C, before E
        if ( worker_running && frame >= done_frame && land_after_C ) { s.flag = -1; worker_running = false; }
        // ---- block E: executor ----
        bool gate;
        if ( mode == 0 ) gate = ( s.env_change == 0 );
        else if ( mode == 1 ) gate = ( s.env_change == 0 );
        else gate = ( s.env_change == 0 || s.plan_started || s.plan_replans >= 2 );
        if ( mode == 3 ) gate = ( s.env_change == 0 || s.plan_replans >= 2 );
        if ( s.flag == -1 && gate && ! s.plan.empty() ) {
            if ( s.ai_delay > 0 ) --s.ai_delay;
            while ( s.flag == -1 && gate && s.ai_delay == 0 && ! s.plan.empty() ) {
                int mov = s.plan[0];
                s.plan.erase( s.plan.begin() );
                if ( mode >= 1 ) s.plan_started = s.plan.empty() ? 0 : 1;
                if ( mov == 1 ) {
                    ++R.drops;
                    if ( s.snapshot_upcome != s.upcome ) ++R.stale_drops;
                    s.upcome = 0;
                    s.env_change = 1;         // drop arms next-piece replan; here spawn happens immediately
                    new_piece_pending = true;
                    s.ai_delay = move_delay;
                    break;
                } else {
                    s.ai_delay = move_delay;
                }
            }
        }
        if ( R.drops >= 40 ) break;
    }
    return R;
}

int main()
{
    printf("mode0=HEAD mode1=blanket-gate mode2=current (start-aware gate, cap 2)\n");
    printf("stale = drop executed with outdated snapshot | drops40 = reached 40 placements within 20000 frames\n");
    int T_list[] = {2, 8, 24};
    for ( int ti = 0; ti < 3; ++ti ) {
        int T = T_list[ti];
        for ( int move_delay : {0, 7} ) {
            vector<int> atk_frames, atk_amts;
            for ( int k = 0; k < 200; ++k ) { atk_frames.push_back(3 + k * ( 2 + T / 2 )); atk_amts.push_back(4); }
            for ( int mode = 0; mode < 4; ++mode ) {
                Res r = run(T, move_delay, atk_frames, atk_amts, mode);
                printf("HEAVY T=%2d move=%d mode=%d | drops=%3ld stale=%3ld preempts=%3ld frames=%5ld%s\n",
                    T, move_delay, mode, r.drops, r.stale_drops, r.preempts, r.frames,
                    r.drops < 40 ? "  << STALLED << " : "");
            }
        }
    }
    return 0;
}