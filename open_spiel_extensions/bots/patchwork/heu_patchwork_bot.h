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

#ifndef OPEN_SPIEL_BOTS_PATCHWORK_HEU_PATCHWORK_BOT_H_
#define OPEN_SPIEL_BOTS_PATCHWORK_HEU_PATCHWORK_BOT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "open_spiel/games/patchwork/patch.h"
#include "open_spiel/games/patchwork/patchwork_player.h"
#include "open_spiel/spiel.h"
#include "open_spiel/spiel_bots.h"

namespace open_spiel {
namespace patchwork {

class PWPlacer {
public:
    static const PatchLib& pl;

    std::vector<open_spiel::Action> BL(const place_t& board, int next_sew);
    std::vector<Action> paretoBL(
        const place_t& board, int next_sew
    );

    std::vector<double> evaluate(
        const place_t& board, const std::vector<Action>& places, int next_sew, const std::vector<bool>& patch_remain
    );
    // std::vector<double> pggr(
    //     const place_t& board, const std::vector<Action>& places, int next_sew, const std::vector<bool>& patch_remain
    // ); 

    // std::vector<double> place_regrets(
    //     const place_t& board, const std::vector<Action>& places, int next_sew, const std::vector<bool>& patch_remain
    // ); 

    std::vector<double> local_regrets(
        const place_t& board, const std::vector<Action>& places, int next_sew, const std::vector<bool>& patch_remain
    ); 

    // double board_evaluate(
    //     const place_t& board, const std::vector<bool>& patch_remain
    // ); 
};

class PWSewEvaluator {
public:

    double pggr(const place_t& board, int pid);

};

class PWChooser {
public:
    static const PatchLib& pl;
    static const TimeLine& tl;
    std::vector<double> evaluate(
        const PatchworkPlayer& player, const std::vector<int>& patch_ids
    );
};


class HeuPatchworkBot : public Bot {
  public:
  Player player_id_;

  PWPlacer placer;
  PWChooser chooser;
  
  explicit HeuPatchworkBot(int player_id);
  Action Step(const State& state);


};
}
}

#endif