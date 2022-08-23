// Copyright 2019 DeepMind Technologies Limited
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

#include "open_spiel/games/patchwork/patch.h"
#include "open_spiel/games/patchwork/patchwork_player.h"
#include "open_spiel/games/patchwork.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "open_spiel/spiel_utils.h"
#include "open_spiel/utils/tensor_view.h"

namespace open_spiel {
namespace patchwork {
namespace {


constexpr char kDefaultPatchQueue[] = "1;2;3;4;5;6;7;8;9;10;11;12;13;14;15;16;17;18;19;20;21;22;23;24;25;26;27;28;29;30;31;32;33";

std::vector<int> ParsePQString(const std::string &str) {
  std::vector<std::string> patch_ids = absl::StrSplit(str, ';');
  std::vector<int> patch_queue;
  for (const auto &sz : patch_ids) {
    int patch;
    if (!absl::SimpleAtoi(sz, &patch)) {
      SpielFatalError(absl::StrCat("Could not parse size '", sz,
                                   "' of patch_queue string '", str,
                                   "' as an integer"));
    }
    patch_queue.push_back(patch);
  }
  return patch_queue;
}

// Facts about the game.
const GameType kGameType{
    /*short_name=*/"patchwork",
    /*long_name=*/"Patchwork",
    GameType::Dynamics::kSequential,
    GameType::ChanceMode::kDeterministic,
    GameType::Information::kPerfectInformation,
    GameType::Utility::kZeroSum,
    GameType::RewardModel::kTerminal,
    /*max_num_players=*/2,
    /*min_num_players=*/2,
    /*provides_information_state_string=*/true,
    /*provides_information_state_tensor=*/false,
    /*provides_observation_string=*/true,
    /*provides_observation_tensor=*/false,
    /*parameter_specification=*/{
      {"patch_queue", GameParameter(std::string(kDefaultPatchQueue))},
    }
};

std::shared_ptr<const Game> Factory(const GameParameters& params) {
  return std::shared_ptr<const Game>(new PatchworkGame(params));
}

REGISTER_SPIEL_GAME(kGameType, Factory);

}  // namespace


// patchwork game
PatchworkGame::PatchworkGame(const GameParameters& params)
    : Game(kGameType, params), 
    patch_queue(ParsePQString(ParameterValue<std::string>("patch_queue"))) {
}


// patchwork state
const PatchLib& PatchworkState::pl = PatchworkPlayer::pl;
place_t PatchworkState::full_board = place_t("2417851639229258349412351");

void PatchworkState::DoApplyAction(Action move) {
  // std::cout << "do apply action " << move << std::endl;

  switch (phase) {
    case Phase::choose: {
      ApplyChooseAction(move);
      break;
    }
    case Phase::sew: {
      ApplySewAction(move);
      break;
    }
    default: {
      std::cout << "[ERROR]: Undefined action occurs in PatchworkState." << std::endl;
      break;
    }
  }
}


void PatchworkState::ApplyChooseAction(Action action) {
  // std::cout << "in apply choose action " << action << std::endl;
  if (action == passAction) {
      int move = players[opponent_].cur_pos - players[current_player_].cur_pos + 1;
      players[current_player_].move_reward(move);
      PlayerMove(move);
  }
  else {
      int node = action;
      int patch_id = patch_queue[node];
      const Patch& patch = PatchworkPlayer::pl.get_patch(patch_id);
      
      players[current_player_].choose(patch);

      int post = post_node[node];
      int pre = pre_node[node];
      
      post_node[pre] = post;
      pre_node[post] = pre;

      cur_node = post;
      next_sew = patch_id;
      
      // remove patches
      patch_remain[patch_id] = false;
      phase = Phase::sew;
  }
}

void PatchworkState::ApplySewAction(Action action) {
  // std::cout << "in apply sew action " << action << std::endl;

  int patch_id = pl.pl_id_2_pa_id(action);
  players[current_player_].sew(patch_id, action);
  if ((!square7) && players[current_player_].area > 49) {
      for (auto& p: pl.square7) {
          if (p ^ players[current_player_].board == p) {
              players[current_player_].square7 = 7;
              square7 = true;
              break;
          }
      }
  }

  PlayerMove(pl.get_patch(patch_id).tc);
}

void PatchworkState::PlayerMove(int move) {
    auto& cur = players[current_player_];
    auto& opp = players[opponent_];
    cur.move(move);
    if (cur.cross_spp(opp)) {
        if (cur.board == full_board) {
            if (cur.cur_pos > opp.cur_pos) {
                // swap two player;
                std::swap(opponent_, current_player_);
                // opponent_ = current_player_;
                // current_player_ = 1 - current_player_;
            }
            phase = Phase::choose;
        }
        else {
            phase = Phase::sew;
            next_sew = pl.get_patch(PatchLib::ONExONE_ID).id;
        }
    }
    else {
        if (cur.cur_pos > opp.cur_pos) {
            // swap two player;
            std::swap(current_player_, opponent_);
        }
        phase = Phase::choose;
    }
}

std::vector<Action> PatchworkState::LegalActions() const {
  if (IsTerminal()) {
    return {};
  }
  auto& cur = players[current_player_];

  std::vector<Action> moves;
  switch (phase) {
      case Phase::choose: {
          moves.reserve(4);
          int node = cur_node;
          for (int i = 0; i < 3; ++i) {
              int patch_id = patch_queue[node];
              const Patch& p = pl.get_patch(patch_id);
              if ((p.bc <= cur.buttons) && cur.is_sewable(patch_id)) {
                  moves.push_back(node);
              }
              node = post_node[node];
          }
          moves.push_back(100);
          break;
      }
      case Phase::sew: {
          moves = cur.all_sewable_ways(next_sew);
          break;
      }
      default: {
          break;
      }
  }
  std::sort(moves.begin(), moves.end());
  return moves;
}

std::string PatchworkState::ActionToString(Player player,
                                           Action action_id) const {
  // return absl::StrCat(StateToString(PlayerToState(player)), "(",
  //                     action_id / kNumCols, ",", action_id % kNumCols, ")");
  // return "action2string";
  return absl::StrCat(
    std::to_string(player), "(",
    (phase == Phase::choose) ? "c" : "s", 
    std::to_string(action_id), ")"
  );
}

PatchworkState::PatchworkState(std::shared_ptr<const Game> game, 
                               std::vector<int> patch_queue) 
  : State(game),
    patch_queue(patch_queue) {
  players[0] = PatchworkPlayer(0);
  players[1] = PatchworkPlayer(1);
  
  post_node = std::vector<int>(kNumPatch, 0);
  std::iota(post_node.begin(), post_node.end(), 1);
  post_node.back() = 0;

  pre_node = std::vector<int>(kNumPatch, 0);
  std::iota(pre_node.begin()+1, pre_node.end(), 0);
  pre_node.front() = kNumPatch - 1;

  patch_remain = std::vector<bool>(kNumPatch, true);
}

// PatchworkState::PatchworkState(const PatchworkState& other) {
//   current_player_ = other.current_player_;
//   opponent_ = other.opponent_;
//   phase = other.phase;
//   square7 = other.square7;
//   cur_node = other.cur_node;
//   next_sew = other.next_sew;

//   patch_queue.reserve(PATCH_NUM);
//   post_node.reserve(PATCH_NUM);
//   pre_node.reserve(PATCH_NUM);
//   patch_remain.reserve(PATCH_NUM);

//   copy(other.patch_queue.begin(), other.patch_queue.end(), back_inserter(patch_queue));
//   copy(other.post_node.begin(), other.post_node.end(), back_inserter(post_node));
//   copy(other.pre_node.begin(), other.pre_node.end(), back_inserter(pre_node));
//   copy(other.patch_remain.begin(), other.patch_remain.end(), back_inserter(patch_remain));
// }

// PatchworkState& operator=(const PatchworkState& other) {
//   current_player_ = other.current_player_;
//   opponent_ = other.opponent_;
//   phase = other.phase;
//   square7 = other.square7;
//   cur_node = other.cur_node;
//   next_sew = other.next_sew;

//   patch_queue.reserve(PATCH_NUM);
//   post_node.reserve(PATCH_NUM);
//   pre_node.reserve(PATCH_NUM);
//   patch_remain.reserve(PATCH_NUM);

//   copy(other.patch_queue.begin(), other.patch_queue.end(), back_inserter(patch_queue));
//   copy(other.post_node.begin(), other.post_node.end(), back_inserter(post_node));
//   copy(other.pre_node.begin(), other.pre_node.end(), back_inserter(pre_node));
//   copy(other.patch_remain.begin(), other.patch_remain.end(), back_inserter(patch_remain));
//   return *this;
// }

std::string PatchworkState::ToString() const {
  return absl::StrCat(
    players[0].to_string(), "\n",
    players[1].to_string()
  );
}

bool PatchworkState::IsTerminal() const {
  return players[0].is_terminal() && players[1].is_terminal();
}

std::vector<double> PatchworkState::Returns() const {
  if (IsTerminal()) {
    if (players[0].future_score() > players[1].future_score()) {
    // if (players[0].avg_score() > players[1].avg_score()) {
      return {1.0, -1.0};
    }
    else {
      return {-1.0, 1.0};
    }
    // return {double(players[0].score()), double(players[1].score())};
  }
  else {
    return {0.0, 0.0};
  }
}

double PatchworkState::Advantage(Player player) const {
  return double(players[player].future_score() - players[1 - player].future_score());
}

std::string PatchworkState::InformationStateString(Player player) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);
  return players[player].to_string();
}

std::string PatchworkState::ObservationString(Player player) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);
  return players[player].to_string();
}

void PatchworkState::ObservationTensor(Player player,
                                       absl::Span<float> values) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);
}

// ?
void PatchworkState::UndoAction(Player player, Action move) {
}

std::unique_ptr<State> PatchworkState::Clone() const {
  // std::cout << "in clone" << std::endl;
  return std::unique_ptr<State>(new PatchworkState(*this));
}


}  // namespace tic_tac_toe
}  // namespace open_spiel
