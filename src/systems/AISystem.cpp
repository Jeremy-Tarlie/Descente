#include "systems/Systems.hpp"

#include "pathfinding/AStar.hpp"

#include <random>

namespace descente {

namespace {

int roll(std::mt19937& rng, const int lo, const int hi) {
  std::uniform_int_distribution<int> dist(lo, hi);
  return dist(rng);
}

}  // namespace

void run_ai_turn(GameState& state, const Entity monster, std::mt19937& rng) {
  if (!state.world.alive(monster) || !state.monsters.has(monster)) {
    return;
  }

  Position* mpos = state.positions.get(monster);
  Stats* mstats = state.stats.get(monster);
  Ai* ai = state.ais.get(monster);
  const MonsterTag* tag = state.monsters.get(monster);
  const Position* ppos = state.positions.get(state.player);
  const Stats* pstats = state.stats.get(state.player);

  if (mpos == nullptr || mstats == nullptr || ai == nullptr || tag == nullptr ||
      ppos == nullptr || pstats == nullptr) {
    return;
  }

  if (mstats->hp <= 0) {
    return;
  }

  const int dist = AStar::manhattan(mpos->as_point(), ppos->as_point());
  const bool can_see_player = state.map.in_bounds(mpos->as_point()) &&
                              state.map.at(mpos->as_point()).visible &&
                              dist <= tag->detection_range;

  if (!tag->is_boss && tag->flee_hp_threshold > 0 &&
      mstats->hp <= tag->flee_hp_threshold && can_see_player) {
    ai->state = AiState::Flee;
  } else if (can_see_player) {
    ai->state = AiState::Chase;
  } else if (ai->state == AiState::Chase && dist > tag->detection_range + 2) {
    ai->state = AiState::Idle;
  } else if (ai->state == AiState::Flee &&
             (tag->flee_hp_threshold <= 0 ||
              mstats->hp > tag->flee_hp_threshold)) {
    ai->state = can_see_player ? AiState::Chase : AiState::Idle;
  }

  if (dist == 1 && ai->state != AiState::Flee) {
    resolve_attack(state, monster, state.player);
    return;
  }

  Point target = mpos->as_point();

  if (ai->state == AiState::Chase) {
    const auto blocked = [&](const Point& p) {
      const Entity e = state.blocker_at(p);
      return e != kInvalidEntity && e != state.player && e != monster;
    };
    const PathResult path =
        AStar::find_path(state.map, mpos->as_point(), ppos->as_point(), blocked);
    if (path.found && !path.path.empty()) {
      target = path.path.front();
    }
  } else if (ai->state == AiState::Flee) {
    int best_dist = dist;
    Point best = mpos->as_point();
    for (const Point& n : state.map.neighbors4(mpos->as_point())) {
      if (!state.map.walkable(n)) {
        continue;
      }
      if (state.blocker_at(n) != kInvalidEntity) {
        continue;
      }
      const int d = AStar::manhattan(n, ppos->as_point());
      if (d > best_dist) {
        best_dist = d;
        best = n;
      }
    }
    target = best;
  } else if (roll(rng, 0, 3) == 0) {
    const auto neighbors = state.map.neighbors4(mpos->as_point());
    if (!neighbors.empty()) {
      const Point n = neighbors[static_cast<std::size_t>(
          roll(rng, 0, static_cast<int>(neighbors.size()) - 1))];
      if (state.map.walkable(n) && state.blocker_at(n) == kInvalidEntity) {
        target = n;
      }
    }
  }

  if (target != mpos->as_point()) {
    if (target.x == ppos->x && target.y == ppos->y) {
      resolve_attack(state, monster, state.player);
    } else if (state.blocker_at(target) == kInvalidEntity &&
               state.map.walkable(target)) {
      mpos->set(target);
    }
  }
}

}  // namespace descente
