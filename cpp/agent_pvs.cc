#include <vector>
#include <algorithm>
#include <limits>

#include "patch.h"
#include "patchwork_player.h"
#include "patchwork_state.h"
#include "evaluate.h"
#include "agent_pvs.h"

using namespace std;

#define DEPTH_TH 3

// auto solver_mm = agent_mm::Solver();

solve_t agent_pvs::Solver::solve(
    const PatchworkState& ps, int depth, double alpha, double beta, int player_id
) {
    if (depth == 0 || ps.is_terminal()) {
        // average score
        // auto avg = ps.get_player(player_id).avg_score() 
        //     - ps.get_player(1 - player_id).avg_score();
        // return solve_t(int(avg * 100.0), -2);

        // future score
        auto future = ps.get_player(player_id).future_score() 
            - ps.get_player(1 - player_id).future_score();
        return solve_t(int(future * 100.0), -2);
    }

    auto& player = ps.get_cur_player();

    if (ps.get_cur_player_id() == player_id) {
        double max_eval = -std::numeric_limits<double>::infinity();
        action_t max_action = -2;
        auto legal_actions = vector<action_t>();

        if (ps.get_phase() == Phase::sew) {
            auto placements = placer.BL(player.board, ps.next_sew);
            copy(placements.begin(), placements.end(), back_inserter(legal_actions));
        }
        else if (ps.get_phase() == Phase::choose) {
            auto nodes = ps.legal_actions();
            auto chooses = ps.chooses2pid(nodes);
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

        auto action = legal_actions[0];
        PatchworkState ps_next = PatchworkState(ps);
        ps_next.do_apply_action(action);
        auto solve_val = solve(ps_next, depth, alpha, beta, player_id);
        auto eval = solve_val.first;

        alpha = max(alpha, eval);
                
        if (eval > max_eval) {
            max_action = action;
            max_eval = eval;
        }

        if (beta <= alpha) {
            return solve_t(max_eval, max_action);
        }

        // parallel siblings
        if (legal_actions.size() > 1) {
            #pragma omp parallel for num_threads(legal_actions.size() - 1) if (depth >= DEPTH_TH)
            for (int i = 1; i < legal_actions.size(); ++i) {
                auto action = legal_actions[i];
                auto ps_next = PatchworkState(ps);
                ps_next.do_apply_action(action);
                auto solve_val = solve(ps_next, depth, alpha, beta, player_id);
                auto eval = solve_val.first;
                alpha = max(alpha, eval);
                
                if (eval > max_eval) {
                    max_action = action;
                    max_eval = eval;
                }

                // if (beta <= alpha) {
                //     break;
                // }
            }
        }

        return solve_t(max_eval, max_action);
    }
    else {
        auto min_eval = std::numeric_limits<double>::infinity();
        auto legal_actions = vector<action_t>();

        if (ps.get_phase() == Phase::sew) {
            auto placements = placer.BL(player.board, ps.next_sew);
            copy(placements.begin(), placements.end(), back_inserter(legal_actions));
        }
        else if (ps.get_phase() == Phase::choose) {
            // auto chooses = ps.legal_actions();
            // copy(chooses.begin(), chooses.end(), back_inserter(legal_actions));

            auto nodes = ps.legal_actions();
            auto chooses = ps.chooses2pid(nodes);
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

        auto action = legal_actions[0];
        PatchworkState ps_next = PatchworkState(ps);
        ps_next.do_apply_action(action);
        auto depth_next = (ps_next.get_cur_player_id() == player_id) ? depth - 1 : depth;
        auto solve_val = solve(ps_next, depth_next, alpha, beta, player_id);
        auto eval = solve_val.first;

        beta = min(beta, eval);
                
        if (eval < min_eval) {
            min_eval = eval;
        }

        if (beta <= alpha) {
            return solve_t(min_eval, -2);
        }

        // parallel siblings
        if (legal_actions.size() > 1) {
            #pragma omp parallel for num_threads(legal_actions.size() - 1) if (depth >= DEPTH_TH)
            for (int i = 1; i < legal_actions.size(); ++i) {
                auto action = legal_actions[i];
                auto ps_next = PatchworkState(ps);
                ps_next.do_apply_action(action);

                auto depth_next = (ps_next.get_cur_player_id() == player_id) ? depth - 1 : depth;
                auto solve_val = solve(ps_next, depth_next, alpha, beta, player_id);
                auto eval = solve_val.first;

                beta = min(beta, eval);
                
                if (eval < min_eval) {
                    min_eval = eval;
                }

                // if (beta <= alpha) {
                //     break;
                // }
            }
        }

        return solve_t(min_eval, -2);
    }
}

action_t agent_pvs::Agent::get_action(
    const PatchworkState& ps
) {
    action_t action;
    auto la = ps.legal_actions();
    if (la.size() == 1) {
        action = la[0];
    }
    else {
        auto solve_val = solver.solve(
            ps, depth, -std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity(), player_id
        );
        action = solve_val.second;
    }
    return action;
}