
#include <limits>
#include <algorithm>

#include "patch.h"
#include "patchwork_player.h"
#include "patchwork_state.h"
#include "evaluate.h"

using namespace std;

const PatchLib& PWPlacer::pl = PatchworkPlayer::pl;

vector<action_t> PWPlacer::BL(const place_t& board, int next_sew) {
    auto places = vector<action_t>();
    auto& pi = pl.patch_xy2pi_sorted.find(next_sew)->second;

    for (auto& p: pi) {
        if ((board & p.second) == 0) {
            places.push_back(p.first);
            return places;
        }
    }
    
    cout << "Could not find legal placements." << endl;
}

vector<action_t> PWPlacer::paretoBL(const place_t& board, int next_sew) {
    auto places = vector<action_t>();
    const auto& pi = pl.patch_x2y2pi_sorted.find(next_sew)->second;

    for (auto& p_row: pi) {
        for (auto& p: p_row) {
            if ((board & p.second) == 0) {
                places.push_back(p.first);
                break;
            }
        }
    }

    return places;
}

vector<double> PWPlacer::evaluate(
    const place_t& board, const vector<action_t>& place_ids, int next_sew, const vector<bool>& patch_remain) {
    // return place_regrets(board, place_ids, next_sew, patch_remain);
    return local_regrets(board, place_ids, next_sew, patch_remain);
    // return pggr(board, place_ids, next_sew, patch_remain);
}

// vector<double> PWPlacer::pggr(
//     const place_t& board, const vector<action_t>& place_ids, int next_sew, const vector<bool>& patch_remain) {

//     for (auto& place_id: place_ids) {
//         auto& place = pl.patch_alter_vec[place_id];
//         auto board_next = board | place;
//         for (int x = 0; x < BOARD_SIZE; ++x) {
//             for (int y = 0; y < BOARD_SIZE; ++y) {
//                 auto& p = pl.regrets[x][y];
//                 if ((p.second != 1) && (place & board_next == 0)) {
//                     regrets += 0；
//                 }
//             }
//         }
//     }
// }

vector<double> PWPlacer::place_regrets(
    const place_t& board, const vector<action_t>& place_ids, int next_sew, const vector<bool>& patch_remain) {
    auto regrets = vector<double>();
    regrets.reserve(place_ids.size());

    for (auto& place_id: place_ids) {
        int regret = 0;
        auto board_next = board | pl.patch_alter_vec[place_id];
        for (int patch_id = 0; patch_id < patch_remain.size(); ++patch_id) {
            bool is_remained = patch_remain[patch_id];
            if (is_remained && (patch_id != next_sew)) {
                const auto& alter_places = pl.place_map.find(patch_id)->second;
                // major loops
                for (auto& p: alter_places) {
                    regret += (((board_next & p) != 0) && ((board & p) == 0));
                }
            }
        }
        regrets.push_back(-regret);
    }
    return regrets;
}

// local regrets
vector<double> PWPlacer::local_regrets(
    const place_t& board, const vector<action_t>& place_ids, int next_sew, const vector<bool>& patch_remain) {
    auto regrets = vector<double>();

    for (auto& place_id: place_ids) {
        int regret = 0;
        for (int patch_id = 0; patch_id < patch_remain.size(); ++patch_id) {
            bool is_remained = patch_remain[patch_id];
            if (is_remained && (patch_id != next_sew)) {
                const auto& a = pl.overlap[place_id].find(patch_id);
                if (a != pl.overlap[place_id].end()) {
                    for (auto& ol_place: a->second) {
                        regret += ((ol_place & board) == 0);
                    }
                }

            }
        }
        regrets.push_back(-regret);
    }
    return regrets;
}

double PWPlacer::board_evaluate(
    const place_t& board, const vector<bool>& patch_remain
) {
    int eval = 0;
    for (int patch_id = 0; patch_id < patch_remain.size(); ++patch_id) {
        bool is_remained = patch_remain[patch_id];
        if (is_remained) {
            for (auto& place: pl.place_map.find(patch_id)->second) {
                eval += ((place & board) == 0);
            }
        }
    }
    return eval;
}

const PatchLib& PWChooser::pl = PatchworkPlayer::pl;
const TimeLine& PWChooser::tl = PatchworkPlayer::tl;

vector<double> PWChooser::evaluate(const PatchworkPlayer& player, const vector<int>& patch_ids) {
    auto evals = vector<double>();
    evals.reserve(patch_ids.size());
    for (auto p: patch_ids) {
        if (p == -1) {
            evals.push_back(1);
        }
        else if (p == 0) {
            evals.push_back(2);
        }
        else {
            auto patch = pl.get_patch(p);
            auto val = patch.br * (9 - tl.brp[player.cur_pos]) + 2 * patch.area - patch.bc;
            // double val_avg = double(val) / double(patch.tc);
            double val_avg = double(val) / double(min(patch.tc, 53 - player.cur_pos));
            evals.push_back(val_avg);
        }
    }
    return evals;
}