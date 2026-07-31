#include "game/SaveLoad.hpp"

#include "fov/Fov.hpp"

#include <fstream>
#include <string>

namespace descente {

namespace {

constexpr std::uint32_t kMagic = 0x44534354U;  // DSCT
constexpr std::uint32_t kVersion = 2;

template <typename T>
void write_pod(std::ostream& out, const T& value) {
  out.write(reinterpret_cast<const char*>(&value), static_cast<std::streamsize>(sizeof(T)));
}

template <typename T>
bool read_pod(std::istream& in, T& value) {
  in.read(reinterpret_cast<char*>(&value), static_cast<std::streamsize>(sizeof(T)));
  return static_cast<bool>(in);
}

void write_string(std::ostream& out, const std::string& s) {
  const std::uint32_t len = static_cast<std::uint32_t>(s.size());
  write_pod(out, len);
  out.write(s.data(), static_cast<std::streamsize>(len));
}

bool read_string(std::istream& in, std::string& s) {
  std::uint32_t len = 0;
  if (!read_pod(in, len)) {
    return false;
  }
  if (len > 4096) {
    return false;
  }
  s.assign(static_cast<std::size_t>(len), '\0');
  in.read(s.data(), static_cast<std::streamsize>(len));
  return static_cast<bool>(in);
}

void write_item(std::ostream& out, const Item& item) {
  write_pod(out, item.kind);
  write_string(out, item.name);
  write_pod(out, item.glyph);
}

bool read_item(std::istream& in, Item& item) {
  if (!read_pod(in, item.kind)) {
    return false;
  }
  if (!read_string(in, item.name)) {
    return false;
  }
  return read_pod(in, item.glyph);
}

}  // namespace

bool save_game(const GameState& state, const std::string& path) {
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    return false;
  }

  write_pod(out, kMagic);
  write_pod(out, kVersion);
  write_pod(out, state.seed);
  write_pod(out, state.floor);
  write_pod(out, state.turn);

  const int w = state.map.width();
  const int h = state.map.height();
  write_pod(out, w);
  write_pod(out, h);
  for (const Tile& tile : state.map.tiles()) {
    write_pod(out, tile.type);
    write_pod(out, tile.explored);
  }

  // Player
  const Position* pos = state.positions.get(state.player);
  const Stats* stats = state.stats.get(state.player);
  const Inventory* inv = state.inventories.get(state.player);
  if (pos == nullptr || stats == nullptr || inv == nullptr) {
    return false;
  }
  write_pod(out, pos->x);
  write_pod(out, pos->y);
  write_pod(out, *stats);

  const std::uint32_t inv_count = static_cast<std::uint32_t>(inv->items.size());
  write_pod(out, inv_count);
  for (const Item& item : inv->items) {
    write_item(out, item);
  }

  // Monsters
  std::uint32_t monster_count = 0;
  state.monsters.for_each([&](const Entity, const MonsterTag&) { ++monster_count; });
  write_pod(out, monster_count);
  state.monsters.for_each([&](const Entity e, const MonsterTag& tag) {
    const Position* mp = state.positions.get(e);
    const Stats* ms = state.stats.get(e);
    const Renderable* mr = state.renderables.get(e);
    const Name* mn = state.names.get(e);
    const Ai* mai = state.ais.get(e);
    if (mp == nullptr || ms == nullptr || mr == nullptr || mn == nullptr ||
        mai == nullptr) {
      return;
    }
    write_pod(out, mp->x);
    write_pod(out, mp->y);
    write_pod(out, *ms);
    write_pod(out, tag);
    write_pod(out, mai->state);
    write_pod(out, mr->glyph);
    write_string(out, mn->value);
  });

  // Ground items
  std::uint32_t ground_count = 0;
  state.items.for_each([&](const Entity e, const Item&) {
    if (!state.blockers.has(e)) {
      ++ground_count;
    }
  });
  write_pod(out, ground_count);
  state.items.for_each([&](const Entity e, const Item& item) {
    if (state.blockers.has(e)) {
      return;
    }
    const Position* ip = state.positions.get(e);
    if (ip == nullptr) {
      return;
    }
    write_pod(out, ip->x);
    write_pod(out, ip->y);
    write_item(out, item);
  });

  return static_cast<bool>(out);
}

bool load_game(GameState& state, const std::string& path, std::mt19937& rng) {
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    return false;
  }

  std::uint32_t magic = 0;
  std::uint32_t version = 0;
  if (!read_pod(in, magic) || magic != kMagic) {
    return false;
  }
  if (!read_pod(in, version) || version != kVersion) {
    return false;
  }

  std::uint32_t seed = 0;
  int floor = 0;
  int turn = 0;
  if (!read_pod(in, seed) || !read_pod(in, floor) || !read_pod(in, turn)) {
    return false;
  }

  int w = 0;
  int h = 0;
  if (!read_pod(in, w) || !read_pod(in, h) || w <= 0 || h <= 0 || w > 512 ||
      h > 512) {
    return false;
  }

  Map map(w, h);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      TileType type{};
      bool explored = false;
      if (!read_pod(in, type) || !read_pod(in, explored)) {
        return false;
      }
      map.at(x, y).type = type;
      map.at(x, y).explored = explored;
      map.at(x, y).visible = false;
    }
  }

  int px = 0;
  int py = 0;
  Stats pstats{};
  if (!read_pod(in, px) || !read_pod(in, py) || !read_pod(in, pstats)) {
    return false;
  }

  std::uint32_t inv_count = 0;
  if (!read_pod(in, inv_count) || inv_count > Inventory::kCapacity) {
    return false;
  }
  Inventory inv;
  for (std::uint32_t i = 0; i < inv_count; ++i) {
    Item item;
    if (!read_item(in, item)) {
      return false;
    }
    inv.items.push_back(std::move(item));
  }

  state.clear_entities();
  state.map = std::move(map);
  state.rooms.clear();
  state.seed = seed;
  state.floor = floor;
  state.turn = turn;
  state.phase = GamePhase::Playing;
  state.messages.clear();
  rng.seed(seed + static_cast<std::uint32_t>(turn));

  const Entity player = state.world.create_entity();
  state.player = player;
  state.positions.set(player, Position{px, py});
  state.renderables.set(player, Renderable{'@', 1});
  state.stats.set(player, pstats);
  state.names.set(player, Name{"Vous"});
  state.players.set(player, PlayerTag{});
  state.blockers.set(player, BlocksMovement{});
  state.inventories.set(player, std::move(inv));

  std::uint32_t monster_count = 0;
  if (!read_pod(in, monster_count) || monster_count > 500) {
    return false;
  }
  for (std::uint32_t i = 0; i < monster_count; ++i) {
    int mx = 0;
    int my = 0;
    Stats ms{};
    MonsterTag tag{};
    AiState ai_state{};
    char glyph = 'r';
    std::string name;
    if (!read_pod(in, mx) || !read_pod(in, my) || !read_pod(in, ms) ||
        !read_pod(in, tag) || !read_pod(in, ai_state) || !read_pod(in, glyph) ||
        !read_string(in, name)) {
      return false;
    }
    const Entity e = state.world.create_entity();
    state.positions.set(e, Position{mx, my});
    state.renderables.set(e, Renderable{glyph, 2});
    state.stats.set(e, ms);
    state.names.set(e, Name{name});
    state.monsters.set(e, tag);
    state.ais.set(e, Ai{ai_state});
    state.blockers.set(e, BlocksMovement{});
  }

  std::uint32_t ground_count = 0;
  if (!read_pod(in, ground_count) || ground_count > 500) {
    return false;
  }
  for (std::uint32_t i = 0; i < ground_count; ++i) {
    int ix = 0;
    int iy = 0;
    Item item;
    if (!read_pod(in, ix) || !read_pod(in, iy) || !read_item(in, item)) {
      return false;
    }
    const Entity e = state.world.create_entity();
    state.positions.set(e, Position{ix, iy});
    state.renderables.set(e, Renderable{item.glyph, 3});
    state.items.set(e, item);
    state.names.set(e, Name{item.name});
  }

  // Restore FOV
  if (const Position* p = state.positions.get(state.player)) {
    Fov::compute(state.map, p->as_point(), 10);
  }
  state.needs_redraw = true;
  return true;
}

}  // namespace descente
