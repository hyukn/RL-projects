#pragma once

#include <algorithm>
#include <vector>

#include "patch.h"
#include "patchwork_player.h"
#include "patch.h"

#define PATCH_NUM 33

enum Phase {
    choose = 0,
    sew = 1
};

class PatchworkState {
public:
    static const place_t full_board;
    static const vector<int> post_node_init;
    static const vector<int> pre_node_init;

    vector<int> patch_queue;
    PatchworkPlayer cur_player;
    PatchworkPlayer opponent;

    static const PatchLib& pl;

    Phase phase = Phase::choose;
    bool square7 = false;

    vector<int> post_node;
    // int post_node[PATCH_NUM];
    vector<int> pre_node;
    // int pre_node[PATCH_NUM];
    vector<bool> patch_remain;
    // unordered_map<int, bool> patch_remain;

    int cur_node = 0;
    
    int next_sew;
    
    PatchworkState();
    PatchworkState(const PatchworkState&);

    vector<action_t> legal_actions() const;
    void do_apply_action(action_t action);
    void apply_choose_action(action_t action);
    void apply_sew_action(action_t action);
    void player_move(int move);

    vector<int> chooses2pid(vector<action_t>& chooses) const {
        auto pid = vector<int>();
        for (auto i : chooses) {
            pid.push_back((i == -1) ? -1 : patch_queue[i]);
        }
        return pid;
    }
    inline Phase get_phase() const { return phase; }
    inline const PatchworkPlayer& get_cur_player() const { return cur_player; }

    inline const PatchworkPlayer& get_player(int player_id) const {
        return (player_id == cur_player.id)? cur_player : opponent;
    };

    int get_cur_player_id() const {
        return cur_player.id;
    }

    bool is_terminal() const {
        return cur_player.is_terminal() && opponent.is_terminal();
    }

    int winner() {
        return (cur_player.score() > opponent.score())? cur_player.id :opponent.id;
    }

    string observe() {}
    
};