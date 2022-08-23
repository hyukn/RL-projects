
#include "patchwork.h"

using namespace std;

void Patchwork::play(AgentBase& agent0, AgentBase& agent1) {
    init();
    while (!ps.is_terminal()) {
        auto pid = ps.get_cur_player_id();
        auto action = (pid == 0)? agent0.get_action(ps) : agent1.get_action(ps);
        if (ps.get_cur_player_id() == 0) {
            // cout << "phase: " << ps.get_phase() << ", action: " << action << endl;
            auto observe = ps.cur_player.observe();
            cout << observe << endl;
        }
        ps.do_apply_action(action);
    }
}