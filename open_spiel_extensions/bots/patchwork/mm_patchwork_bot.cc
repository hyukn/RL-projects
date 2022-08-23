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
#include "open_spiel/bots/patchwork/mm_patchwork_bot.h"

namespace open_spiel {
namespace patchwork {

double MMPatchworkBot::alpha_beta_search(
    const PatchworkState& ps, int depth, double alpha, double beta, int player_id,
    std::function<double(const State&)> value_function, Action* best_action
) {
    if (depth == 0 || ps.IsTerminal()) {
        return value_function(ps);
    }

    const PatchworkPlayer& player = ps.players[ps.CurrentPlayer()];

    if (ps.CurrentPlayer() == player_id) {
        double max_eval = -std::numeric_limits<double>::infinity();
        Action max_action = -2;
        auto legal_actions = std::vector<Action>();

        if (ps.phase == Phase::sew) {
            // auto placements = placer.BL(player.board, ps.next_sew);
            auto placements = placer.paretoBL(player.board, ps.next_sew);
            if (placements.size() > 1) {
                auto evals = placer.evaluate(
                    player.board, placements, ps.next_sew, ps.patch_remain
                );
                std::vector<int> indices(placements.size());
                std::iota(indices.begin(), indices.end(), 0);
                std::sort(indices.begin(), indices.end(),
                    [&](int A, int B) -> bool {
                        return evals[A] > evals[B];
                });
                for (auto i : indices) {
                    legal_actions.push_back(placements[i]);
                    if (legal_actions.size() >= 2) {
                        break;
                    }
                }
            }
            else {
                legal_actions.push_back(placements[0]);
            }
        }
        else if (ps.phase == Phase::choose) {
            auto nodes = ps.LegalActions();
            auto chooses = ps.choose2pid(nodes);
            auto evals = chooser.evaluate(player, chooses);
            // sorted chooses by evals
            std::vector<int> indices(chooses.size());
            std::iota(indices.begin(), indices.end(), 0);
            std::sort(indices.begin(), indices.end(),
                [&](int A, int B) -> bool {
                    return evals[A] > evals[B];
            });
            for (auto i : indices) {
                legal_actions.push_back(nodes[i]);
            }
        }

        for (auto action : legal_actions) {
            PatchworkState ps_next = PatchworkState(ps);
            ps_next.DoApplyAction(action);
            auto solve_val = alpha_beta_search(
                ps_next, depth, alpha, beta, player_id, value_function, nullptr);
            auto eval = solve_val;
            alpha = std::max(alpha, eval);

            if (eval > max_eval) {
                max_action = action;
                max_eval = eval;
                if (best_action != nullptr) {
                    *best_action = action;
                }
            }

            if (beta <= alpha) {
                break;
            }
        }
        return max_eval;
    }
    else {
        auto min_eval = std::numeric_limits<double>::infinity();
        auto legal_actions = std::vector<Action>();

        if (ps.phase == Phase::sew) {
            auto placements = placer.BL(player.board, ps.next_sew);
            // auto placements = placer.paretoBL(player.board, ps.next_sew);
            if (placements.size() > 1) {
                auto evals = placer.evaluate(
                    player.board, placements, ps.next_sew, ps.patch_remain
                );
                std::vector<int> indices(placements.size());
                std::iota(indices.begin(), indices.end(), 0);
                std::sort(indices.begin(), indices.end(),
                    [&](int A, int B) -> bool {
                        return evals[A] > evals[B];
                });
                for (auto i : indices) {
                    legal_actions.push_back(placements[i]);
                    if (legal_actions.size() >= 2) {
                        break;
                    }
                }
            }
            else {
                legal_actions.push_back(placements[0]);
            }
        }
        else if (ps.phase == Phase::choose) {
            auto nodes = ps.LegalActions();
            auto chooses = ps.choose2pid(nodes);
            auto evals = chooser.evaluate(player, chooses);
            // sorted chooses by evals
            std::vector<int> indices(chooses.size());
            std::iota(indices.begin(), indices.end(), 0);
            std::sort(indices.begin(), indices.end(),
                [&](int A, int B) -> bool {
                    return evals[A] > evals[B];
            });
            for (auto i : indices) {
                legal_actions.push_back(nodes[i]);
            }
        }

        for (auto action : legal_actions) {
            PatchworkState ps_next = PatchworkState(ps);
            ps_next.DoApplyAction(action);
            auto depth_next = (ps_next.CurrentPlayer() == player_id) ? depth - 1 : depth;
            auto solve_val = alpha_beta_search(ps_next, depth_next, alpha, beta, player_id, value_function, nullptr);
            auto eval = solve_val;
            beta = std::min(beta, eval);
            
            if (eval < min_eval) {
                min_eval = eval;
                if (best_action != nullptr) {
                    *best_action = action;
                }
            }

            if (beta <= alpha) {
                break;
            }
        }
        return min_eval;
    }
}

MMPatchworkBot::MMPatchworkBot(Player player_id, int depth_limit, std::function<double(const State&)> value_function)
  : player_id_(player_id),
  depth_limit_(depth_limit),
  value_function_(value_function) {
}

Action MMPatchworkBot::Step(const State& state) {
    double infinity =std::numeric_limits<double>::infinity();
    Action best_action = kInvalidAction;
    const PatchworkState& pstate = down_cast<const PatchworkState&>(state);
    double value = alpha_beta_search(
        pstate,
        depth_limit_,
        -infinity,
        infinity,
        player_id_,
        value_function_,
        &best_action
    );
    return best_action;
}

}
}