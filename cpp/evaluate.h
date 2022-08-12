#pragma once

#include <limits>
#include <algorithm>

#include "patch.h"
#include "patchwork_player.h"
#include "patchwork_state.h"


class PWPlacer {
public:
    static const PatchLib& pl;

    vector<action_t> BL(const place_t& board, int next_sew);
    vector<action_t> paretoBL(
        const place_t& board, int next_sew
    );

    vector<double> evaluate(
        const place_t& board, const vector<action_t>& places, int next_sew, const vector<bool>& patch_remain
    );
    vector<double> pggr(
        const place_t& board, const vector<action_t>& places, int next_sew, const vector<bool>& patch_remain
    ); 

    vector<double> place_regrets(
        const place_t& board, const vector<action_t>& places, int next_sew, const vector<bool>& patch_remain
    ); 

    vector<double> local_regrets(
        const place_t& board, const vector<action_t>& places, int next_sew, const vector<bool>& patch_remain
    ); 

    double board_evaluate(
        const place_t& board, const vector<bool>& patch_remain
    ); 
};

class PWSewEvaluator {
public:

    double pggr(const place_t& board, int pid);

};

class PWChooser {
public:
    static const PatchLib& pl;
    static const TimeLine& tl;
    vector<double> evaluate(
        const PatchworkPlayer& player, const vector<int>& patch_ids
    );
};