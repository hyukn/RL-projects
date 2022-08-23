// Copyright 2021 DeepMind Technologies Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#include "open_spiel/games/patchwork.h"
#include "open_spiel/games/patchwork/patch.h"
#include "open_spiel/games/patchwork/patchwork_player.h"
#include "open_spiel/bots/patchwork/heu_patchwork_bot.h"

namespace open_spiel {
namespace patchwork {

const PatchLib& PWPlacer::pl = PatchworkPlayer::pl;

std::vector<open_spiel::Action> PWPlacer::BL(const place_t& board, int next_sew) {
    auto places = std::vector<open_spiel::Action>();
    auto& pi = pl.patch_xy2pi_sorted.find(next_sew)->second;

    for (auto& p: pi) {
        if ((board & p.second) == 0) {
            places.push_back(p.first);
            return places;
        }
    }
    
    std::cout << "Could not find legal placements." << std::endl;
}

std::vector<Action> PWPlacer::paretoBL(const place_t& board, int next_sew) {
    auto places = std::vector<Action>();
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

std::vector<double> PWPlacer::evaluate(
    const place_t& board, const std::vector<Action>& place_ids, int next_sew, const std::vector<bool>& patch_remain) {
    // return place_regrets(board, place_ids, next_sew, patch_remain);
    return local_regrets(board, place_ids, next_sew, patch_remain);
    // return pggr(board, place_ids, next_sew, patch_remain);
}

// vector<double> PWPlacer::place_regrets(
//     const place_t& board, const vector<action_t>& place_ids, int next_sew, const vector<bool>& patch_remain) {
//     auto regrets = vector<double>();
//     regrets.reserve(place_ids.size());

//     for (auto& place_id: place_ids) {
//         int regret = 0;
//         auto board_next = board | pl.patch_alter_vec[place_id];
//         for (int patch_id = 0; patch_id < patch_remain.size(); ++patch_id) {
//             bool is_remained = patch_remain[patch_id];
//             if (is_remained && (patch_id != next_sew)) {
//                 const auto& alter_places = pl.place_map.find(patch_id)->second;
//                 // major loops
//                 for (auto& p: alter_places) {
//                     regret += (((board_next & p) != 0) && ((board & p) == 0));
//                 }
//             }
//         }
//         regrets.push_back(-regret);
//     }
//     return regrets;
// }

// local regrets
std::vector<double> PWPlacer::local_regrets(
    const place_t& board, const std::vector<Action>& place_ids, int next_sew, const std::vector<bool>& patch_remain) {
    auto regrets = std::vector<double>();

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

// double PWPlacer::board_evaluate(
//     const place_t& board, const vector<bool>& patch_remain
// ) {
//     int eval = 0;
//     for (int patch_id = 0; patch_id < patch_remain.size(); ++patch_id) {
//         bool is_remained = patch_remain[patch_id];
//         if (is_remained) {
//             for (auto& place: pl.place_map.find(patch_id)->second) {
//                 eval += ((place & board) == 0);
//             }
//         }
//     }
//     return eval;
// }

const PatchLib& PWChooser::pl = PatchworkPlayer::pl;
const TimeLine& PWChooser::tl = PatchworkPlayer::tl;

std::vector<double> PWChooser::evaluate(const PatchworkPlayer& player, const std::vector<int>& patch_ids) {
    auto evals = std::vector<double>();
    evals.reserve(patch_ids.size());
    for (auto p: patch_ids) {
        if (p == passAction) {
            evals.push_back(1);
        }
        else if (p == 0) {
            evals.push_back(2);
        }
        else {
            auto patch = pl.get_patch(p);

            auto val = patch.br * (9 - tl.brp[player.cur_pos]) + 2 * patch.area - patch.bc;
            // double val_avg = double(val) / double(patch.tc);
            double val_avg = double(val) / double(std::min(patch.tc, 53 - player.cur_pos));
            evals.push_back(val_avg);
        }
    }
    return evals;
}


HeuPatchworkBot::HeuPatchworkBot(Player player_id)
  : player_id_(player_id) {
}

Action HeuPatchworkBot::Step(const State&state) {
    std::cout << "player_id " << player_id_ << std::endl;
    const PatchworkState& pstate = down_cast<const PatchworkState&>(state);
    const PatchworkPlayer& player = pstate.players[player_id_];

    if (pstate.phase == Phase::sew) {
        return placer.BL(player.board, pstate.next_sew)[0];
    }
    else if (pstate.phase == Phase::choose) {
        std::vector<Action> nodes = pstate.LegalActions();
        std::vector<int> chooses = pstate.choose2pid(nodes);
        std::vector<double> evals = chooser.evaluate(player, chooses);
        std::vector<int> indices(chooses.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::sort(indices.begin(), indices.end(), 
            [&](int A, int B) -> bool{
                return evals[A] > evals[B];
            }
        );
        return nodes[indices[0]];
    }
}
}
}
