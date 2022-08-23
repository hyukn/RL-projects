#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <map>

#include "patch.h"

#define INIT_BUTTONS 6

using namespace std;

using action_t = int;

class TimeLine {
public:
    vector<int> SQUARE_PATCH_POS = {25, 31, 37, 43, 49};
    vector<int> BUTTON_REWARD_POS = {4, 10, 16, 22, 28, 34, 40, 46, 52};    
    int TIMELINE_END = 53;

    vector<int> spp;
    vector<int> brp;

    TimeLine();
};

class PatchworkPlayer {
public:
    int id = 0;
    int cur_pos = 0;
    int prev_pos = 0;
    int buttons = 5;

    int square7 = 0;
    int br = 0;
    int area = 0;
    
    vector<action_t> action_history;
    place_t board = 0;
    
    static const TimeLine tl;
    static const PatchLib pl;

    PatchworkPlayer() {}
    PatchworkPlayer(int given_id):
        id(given_id) {}

    PatchworkPlayer(const PatchworkPlayer& player) {
        id = player.id;
        cur_pos = player.cur_pos;
        prev_pos = player.prev_pos;
        buttons = player.buttons;
        square7 = player.square7;
        br = player.br;
        area = player.area;

        action_history.reserve(player.action_history.size());
        copy(player.action_history.begin(), player.action_history.end(), back_inserter(action_history));
        board = player.board;
    }

    inline bool is_terminal() const { return cur_pos == tl.TIMELINE_END; }
    inline void move_reward(int move) { buttons += move; }

    bool cross_spp(const PatchworkPlayer& opponent) {
        return (tl.spp[cur_pos] != tl.spp[prev_pos])
            && (tl.spp[cur_pos] > tl.spp[opponent.cur_pos]);
    }
    inline bool cross_brp() { return tl.brp[cur_pos] != tl.brp[prev_pos]; };

    inline void choose(const Patch& patch) { 
        buttons -= patch.bc; 
    }
    
    void sew(int patch_id, action_t action) {
        action_history.push_back(action);
        auto& p = pl.get_patch(patch_id);
        area += p.area;
        br += p.br;

        board = board | pl.patch_alter_vec[action];
    }
    
    inline void move(int move) {
        prev_pos = cur_pos;
        cur_pos = min(tl.TIMELINE_END, cur_pos + move);
        if (cross_brp()) {
            buttons += br;
        }
    }

    bool is_sewable(int patch_id) const {
        // auto array_idx = pl.id_map.find(patch_id)->second;
        for (auto& p: pl.place_map.find(patch_id)->second) {
            if ((p & board) == 0) {
                return true;
            }
        }
        return false;
    }

    vector<action_t> all_sewable_ways(int patch_id) const {
        // cout << "patch_id: " << patch_id << endl;
        auto array_idx = pl.id_map.find(patch_id)->second;
        auto ways = decltype(array_idx)();
        copy_if(array_idx.begin(), array_idx.end(), back_inserter(ways), 
            [&](int i) {
                // cout << pl.patch_alter_vec[i] << " " << (pl.patch_alter_vec[i] & board) << endl;
                return (pl.patch_alter_vec[i] & board) == 0;
            }
        );
            
        return ways;
    }

    inline string observe() const {
        string obs = "";
        obs += "id: " + to_string(id) + "\n";
        obs += "board: " + board2string(board) + "\n";
        obs += "score " + to_string(score()) + "\n";
        obs += "future score " + to_string(future_score()) + "\n";
        obs += "cur_pos " + to_string(cur_pos) + "\n";
        obs += "br " + to_string(br) + "\n";
        obs += "buttons: " + to_string(buttons) + "\n";
        obs += "area: " + to_string(area) + "\n";
        return obs;
    }

    inline std::string board2string(const place_t& b) const {
        std::string s = "";
        s += std::bitset<64>(uint64_t(b >> 64)).to_string();
        s += std::bitset<64>(uint64_t(b)).to_string();
        std::reverse(s.begin(), s.end());
        std::string bs = "";
        for (int i = 0; i < BOARD_SIZE; ++i) {
            bs += s.substr(i * BOARD_SIZE, BOARD_SIZE) + "\n";
        }
        return bs;
    }

    inline int future_score() const {
        return -162 + 2 * area + square7 + br * (9 - tl.brp[cur_pos]) + buttons;
    }

    inline int score() const { 
        return -162 + 2 * area + buttons + square7;
    }

    inline double avg_score() const {
        return (cur_pos > 0) ? double(future_score() + 162) / double(cur_pos) : 0;
    }
};