#include <vector>
#include <algorithm>
#include <limits>

#include "patch.h"
#include "patchwork_player.h"
#include "patchwork_state.h"
#include "evaluate.h"
#include "agent_heu.h"

action_t agent_heu::Solver::solve(
    const PatchworkState& ps, int player_id
) {
    auto& player = ps.get_player(player_id);
    action_t action;
    if (ps.get_phase() == Phase::sew) {
        action = placer.BL(player.board, ps.next_sew)[0];
    }  
    else if (ps.get_phase() == Phase::choose) {
        auto nodes = ps.legal_actions();
        auto chooses = ps.chooses2pid(nodes);
        auto evals = chooser.evaluate(player, chooses);
        std::vector<int> indices(chooses.size());
        std::iota(indices.begin(), indices.end(), 0);
        std:sort(indices.begin(), indices.end(), 
            [&](int A, int B) -> bool{
                return evals[A] > evals[B];
            }
        );
        action = nodes[indices[0]];
    }
    return action;
}

action_t agent_heu::Agent::get_action(
    const PatchworkState& ps
) {
    return solver.solve(ps, player_id);
}