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

#ifndef OPEN_SPIEL_BOTS_PATCHWORK_MM_PATCHWORK_BOT_H_
#define OPEN_SPIEL_BOTS_PATCHWORK_MM_PATCHWORK_BOT_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "open_spiel/games/patchwork/patch.h"
#include "open_spiel/games/patchwork/patchwork_player.h"
#include "open_spiel/games/patchwork.h"
#include "open_spiel/bots/patchwork/heu_patchwork_bot.h"
#include "open_spiel/spiel.h"
#include "open_spiel/spiel_bots.h"

namespace open_spiel {
namespace patchwork {

using solve_t = std::pair<double, Action>;

class MMPatchworkBot : public Bot {
  public:
  Player player_id_;
  int depth_limit_ = 1;
  std::function<double(const State&)> value_function_;

  PWPlacer placer;
  PWChooser chooser;
  
  MMPatchworkBot(Player player_id, int depth_limit, std::function<double(const State&)> value_function_);
  double alpha_beta_search(const PatchworkState& ps, int depth, double alpha, double beta, int player_id,
    std::function<double(const State&)> value_function, Action* best_action);
  Action Step(const State& state);
};

}
}

#endif