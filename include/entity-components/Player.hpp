#pragma once

enum class Player { PLAYER1, PLAYER2 };

class PlayerET {
public:
  Player player;

  PlayerET(Player p = Player::PLAYER1) : player(p) {}

  bool IsOpponent(const PlayerET &other) const {
    return player != other.player;
  }
};
