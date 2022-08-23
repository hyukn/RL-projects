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

#ifndef OPEN_SPIEL_GAMES_PATCHWORK_H_
#define OPEN_SPIEL_GAMES_PATCHWORK_H_

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "open_spiel/spiel.h"

#include <algorithm>
#include <vector>

#include "open_spiel/games/patchwork/patch.h"
#include "open_spiel/games/patchwork/patchwork_player.h"

// Simple game of Noughts and Crosses:
// https://en.wikipedia.org/wiki/Tic-tac-toe
//
// Parameters: none

namespace open_spiel {
namespace patchwork {

// Constants.
inline constexpr int kNumPatch = 33;
inline constexpr int kNumPlayers = 2;
inline constexpr int kNumRows = 9;
inline constexpr int kNumCols = 9;
inline constexpr int kNumCells = kNumRows * kNumCols;
inline constexpr int kCellStates = 1 + kNumPlayers;  // empty, 'x', and 'o'.
inline constexpr int kNumPlace = 7480;

inline constexpr int passAction = 100;

// https://math.stackexchange.com/questions/485752/tictactoe-state-space-choose-calculation/485852
inline constexpr int kNumberActions = kNumPlace + kNumPatch;


#define PATCH_NUM 33

enum Phase {
    choose = 0,
    sew = 1
};

// State of an in-play game.
class PatchworkState : public State {
 public:
  explicit PatchworkState(std::shared_ptr<const Game> game,
    std::vector<int> patch_queue);
  
  // PatchworkState(std::shared_ptr<const Game> game);

  PatchworkState(const PatchworkState&) = default;
  PatchworkState& operator=(const PatchworkState&) = default;
  // explicit PatchworkState(const PatchworkState&);
  // PatchworkState& operator=(const PatchworkState&);

  Player CurrentPlayer() const override {
    return IsTerminal() ? kTerminalPlayerId : current_player_;
  }
  std::string ActionToString(Player player, Action action_id) const override;
  std::string ToString() const override;
  bool IsTerminal() const override;
  std::vector<double> Returns() const override;
  double Advantage(Player player) const;


  std::string InformationStateString(Player player) const override;
  std::string ObservationString(Player player) const override;
  void ObservationTensor(Player player,
                         absl::Span<float> values) const override;
  std::unique_ptr<State> Clone() const override;
  void UndoAction(Player player, Action move) override;
  std::vector<Action> LegalActions() const override;
  Player outcome() const { return outcome_; }

  // Only used by Ultimate Tic-Tac-Toe.
  void SetCurrentPlayer(Player player) { current_player_ = player; }
  void DoApplyAction(Action move) override;

  std::vector<int> choose2pid(std::vector<Action>& chooses) const {
    auto pid = std::vector<int>();
    for (auto i : chooses) {
      pid.push_back((i == passAction) ? passAction : patch_queue[i]);
    }
    return pid;
  }


  static place_t full_board;

  std::vector<int> patch_queue;
  PatchworkPlayer players[2];

  static const PatchLib& pl;

  Phase phase = Phase::choose;
  bool square7 = false;
  std::vector<int> post_node;
  std::vector<int> pre_node;
  std::vector<bool> patch_remain;

  int cur_node = 0;
  int next_sew;

 protected:

 private:
  void ApplyChooseAction(Action action);
  void ApplySewAction(Action action);  
  void PlayerMove(int move);

  Player current_player_ = 0;         // Player zero goes first
  Player opponent_ = 1;

  Player outcome_ = kInvalidPlayer;
  int num_moves_ = 0;

};

// Game object.
class PatchworkGame : public Game {
 public:
  explicit PatchworkGame(const GameParameters& params);
  int NumDistinctActions() const override { 
    return kNumberActions; 
  }
  std::unique_ptr<State> NewInitialState() const override {
    return std::unique_ptr<State>(
      new PatchworkState(shared_from_this(), patch_queue));
  }
  int NumPlayers() const override { return kNumPlayers; }
  double MinUtility() const override { return -1; }
  double UtilitySum() const override { return 0; }
  double MaxUtility() const override { return 1; }
  std::vector<int> ObservationTensorShape() const override {
    // return {kCellStates, kNumRows, kNumCols};
    return {0, 0, 0};
  }
  int MaxGameLength() const override { return kNumCells; }

 private:
  std::vector<int> patch_queue;
};

// inline std::ostream& operator<<(std::ostream& stream, const CellState& state) {
//   return stream << StateToString(state);
// }

}  // namespace patchwork
}  // namespace open_spiel

#endif  // OPEN_SPIEL_GAMES_PATCHWORK_H_
