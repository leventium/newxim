#include "RoutingTable.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>

RoutingTable::RoutingTable() {}
RoutingTable::RoutingTable(const std::string& path) { Load(path); }

void RoutingTable::Init(const Graph& graph) {
  Nodes.resize(graph.size(), Node(graph.size()));
}

std::vector<std::string> split(const std::string& str, char s) {
  std::vector<std::string> result;
  size_t first = -1, ind = 0;
  do {
    ind = str.find(s, first + 1);
    if (ind != std::string::npos && ind - first > 1)
      result.push_back(std::move(str.substr(first + 1, ind - first - 1)));
    else if (ind == std::string::npos && str.size() - first > 1)
      result.push_back(std::move(str.substr(first + 1)));
    first = ind;
  } while (first != std::string::npos);
  return result;
}
bool RoutingTable::Load(const std::string& path) {
  std::ifstream fin(path, std::ios::in);

  if (!fin) return false;

  Nodes.clear();

  std::string line;
  while (getline(fin, line)) {
    if (line.size() > 0 && line[0] != '%') {
      std::vector<std::vector<std::int32_t>> node;
      auto routes = split(line, ';');
      for (std::string& route : routes) {
        auto relays = split(route, ',');
        std::vector<std::int32_t> res_relays;
        for (std::string& relay : relays)
          res_relays.push_back(std::stoi(relay));
        node.push_back(std::move(res_relays));
      }
      Nodes.push_back(std::move(node));
    }
  }

  return true;
}

std::pair<std::int32_t, std::int32_t> PerformPairExchange(
    std::int32_t nodes_count, std::int32_t generator, std::int32_t source_node,
    std::int32_t target_node) {
  // d - generator
  // N - nodes_count
  // s - source_node
  // j - dest_node
  std::int32_t k = std::abs(source_node - target_node);
  std::int32_t sgn = target_node <= source_node ? -1 : 1;
  if (k > nodes_count / 2) {
    sgn = -sgn;
    k = nodes_count - k;
  }
  std::int32_t beta = k % generator;
  std::int32_t alpha = k / generator - beta;

  std::int32_t xok, yok;

  // formula (3)
  if (beta - generator <= alpha && alpha <= generator) {
    xok = alpha;
    yok = beta;
  } else if (alpha < beta - generator) {
    xok = alpha + generator + 1;
    yok = beta - generator;
  } else {
    xok = alpha - (generator + 1);
    yok = beta + generator;
  }
  return std::make_pair(sgn * xok, sgn * yok);
}
std::int32_t PerformMultiplicative(std::int32_t nodes_count,
                                   const std::vector<std::int32_t>& generators,
                                   std::int32_t source_node,
                                   std::int32_t target_node) {
  std::int32_t flag = 1;
  if (target_node > source_node)
    target_node -= source_node;
  else
    target_node += (nodes_count - source_node);
  if (target_node > nodes_count / 2) {
    target_node = nodes_count - target_node;
    flag = -1;
  }
  std::int32_t i = generators.size() - 2;
  while (target_node < generators[i]) i--;

  std::int32_t result;
  if (std::abs(target_node - generators[i]) >
      std::abs(target_node - generators[i + 1]))
    result = source_node + flag * generators[i + 1];
  else
    result = source_node + flag * generators[i];

  result %= nodes_count;
  if (result < 0) result += nodes_count;
  return result;
}
std::int32_t PerformClockwise(std::int32_t nodes_count,
                              std::int32_t generator_1,
                              std::int32_t generator_2,
                              std::int32_t source_node,
                              std::int32_t target_node) {
  std::int32_t s = target_node - source_node;
  if (s == 0) return source_node;
  if (s < 0) s += nodes_count;
  if (s <= nodes_count / 2) {
    if (s >= generator_2)
      return (source_node + generator_2) % nodes_count;
    else
      return (source_node + generator_1) % nodes_count;
  } else {
    s = nodes_count - s;
    if (s >= generator_2)
      return (source_node + nodes_count - generator_2) % nodes_count;
    else
      return (source_node + nodes_count - generator_1) % nodes_count;
  }
}
std::int32_t PerformAdaptiveStepCycles(std::int32_t nodes_count,
                                       std::int32_t generator_1,
                                       std::int32_t generator_2,
                                       std::int32_t source_node,
                                       std::int32_t target_node) {
  std::int32_t best_way_r, step_r, best_way_l, step_l;
  std::int32_t s = target_node - source_node;

  std::int32_t r1 = s / generator_2 + s % generator_2;
  std::int32_t r2 = s / generator_2 - s % generator_2 + generator_2 + 1;
  if (s % generator_2 == 0) {
    best_way_r = r1;
    step_r = generator_2;
  } else {
    if (r1 < r2) {
      best_way_r = r1;
      step_r = generator_1;
    } else {
      best_way_r = r2;
      step_r = generator_2;
    }
  }

  std::int32_t r5 =
      (s + nodes_count) / generator_2 + (s + nodes_count) % generator_2;
  std::int32_t r6 = (s + nodes_count) / generator_2 -
                    (s + nodes_count) % generator_2 + generator_2 + 1;
  if (r5 < best_way_r) {
    best_way_r = r5;
    step_r = generator_2;
  }
  if (r6 < best_way_r) {
    best_way_r = r6;
    step_r = generator_2;
  }

  std::int32_t r9 =
      (s + nodes_count * 2) / generator_2 + (s + nodes_count * 2) % generator_2;
  std::int32_t r10 = (s + nodes_count * 2) / generator_2 -
                     (s + nodes_count * 2) % generator_2 + generator_2 + 1;
  if (r9 < best_way_r) {
    best_way_r = r9;
    step_r = generator_2;
  }
  if (r10 < best_way_r) {
    best_way_r = r10;
    step_r = generator_2;
  }

  s = source_node - target_node + nodes_count;
  std::int32_t l1 = s / generator_2 + s % generator_2;
  std::int32_t l2 = s / generator_2 - s % generator_2 + generator_2 + 1;
  if (s % generator_2 == 0) {
    best_way_l = l1;
    step_l = -generator_2;
  } else {
    if (l1 < l2) {
      best_way_l = l1;
      step_l = -generator_1;
    } else {
      best_way_l = l2;
      step_l = -generator_2;
    }
  }

  std::int32_t r7 =
      (s + nodes_count) / generator_2 + (s + nodes_count) % generator_2;
  std::int32_t r8 = (s + nodes_count) / generator_2 -
                    (s + nodes_count) % generator_2 + generator_2 + 1;
  if (r7 < best_way_l) {
    best_way_l = r7;
    step_l = -generator_2;
  }
  if (r8 < best_way_l) {
    best_way_l = r8;
    step_l = -generator_2;
  }

  std::int32_t r11 =
      (s + nodes_count * 2) / generator_2 + (s + nodes_count * 2) % generator_2;
  std::int32_t r12 = (s + nodes_count * 2) / generator_2 -
                     (s + nodes_count * 2) % generator_2 + generator_2 + 1;
  if (r11 < best_way_l) {
    best_way_l = r11;
    step_l = -generator_2;
  }
  if (r12 < best_way_l) {
    best_way_l = r12;
    step_l = -generator_2;
  }
  if (best_way_r < best_way_l)
    return step_r;
  else
    return step_l;
}
std::int32_t PerformAdaptive(std::int32_t nodes_count, std::int32_t generator_1,
                             std::int32_t generator_2, std::int32_t source_node,
                             std::int32_t target_node) {
  std::int32_t result;
  if (source_node > target_node)
    result = source_node - PerformAdaptiveStepCycles(nodes_count, generator_1,
                                                     generator_2, target_node,
                                                     source_node);
  else
    result = source_node + PerformAdaptiveStepCycles(nodes_count, generator_1,
                                                     generator_2, source_node,
                                                     target_node);
  if (result >= nodes_count)
    result -= nodes_count;
  else if (result < 0)
    result += nodes_count;
  return result;
}

bool make_step(const std::vector<std::vector<std::int32_t>>& graph,
               std::vector<bool>& visited, std::int32_t prev,
               std::int32_t from) {
  visited[from] = true;
  for (std::int32_t i = 0; i < graph[from].size(); i++) {
    std::int32_t link_to = graph[from][i];
    if (link_to == prev) continue;

    if (visited[link_to])
      return true;
    else
      return make_step(graph, visited, from, link_to);
  }
  return false;
}
bool no_cycles(const std::vector<std::vector<std::int32_t>>& graph) {
  std::vector<bool> visited(graph.size());

  for (std::int32_t i = 0; i < graph.size(); i++) {
    std::fill(visited.begin(), visited.end(), false);
    if (make_step(graph, visited, -1, i)) return false;
  }

  return true;
}
void mark_weights(const Graph& graph, std::vector<std::int32_t>& weights,
                  std::int32_t node) {
  std::vector<bool> mask(graph[node].size(), false);
  for (std::int32_t i = 0; i < graph[node].size(); i++) {
    std::int32_t n = graph[node][i];
    if (weights[n] >= 0 && weights[n] <= weights[node] + 1) continue;
    weights[n] = weights[node] + 1;
    mask[i] = true;
  }
  for (std::int32_t i = 0; i < graph[node].size(); i++)
    if (mask[i]) mark_weights(graph, weights, graph[node][i]);
}

bool RoutingTable::LoadDijkstra(const Graph& graph) {
  constexpr std::int32_t inf = std::numeric_limits<std::int32_t>::max();

  for (std::int32_t i = 0; i < graph.size(); i++) {
    std::vector<std::int32_t> weights(graph.size(), inf);
    std::vector<bool> visited(graph.size(), false);
    weights[i] = 0;

    std::int32_t min_index, min;
    do {
      min_index = min = inf;

      for (std::int32_t j = 0; j < graph.size(); j++) {
        if (!visited[j] && weights[j] < min) {
          min = weights[j];
          min_index = j;
        }
      }

      if (min_index != inf) {
        for (std::int32_t j = 0; j < graph[min_index].size(); j++) {
          std::int32_t id = graph[min_index][j];
          if (id >= 0) {
            std::int32_t t = min + 1;  // distance may be here...
            if (t < weights[id]) weights[id] = t;
          }
        }
        visited[min_index] = true;
      }
    } while (min_index != inf);

    for (std::int32_t j = 0; j < graph.size(); j++) {
      if (j == i)
        Nodes[j][i].push_back(graph[i].size());
      else {
        auto& current_relay = Nodes[j][i];

        for (std::int32_t k = 0; k < graph[j].size(); k++) {
          std::int32_t id = graph[j][k];
          if (id >= 0) {
            std::int32_t t = weights[j] - 1;
            if (t == weights[id]) current_relay.push_back(k);
          }
        }
      }
    }
  }
  return true;
}
bool RoutingTable::LoadUpDown(const Graph& graph) {
  std::vector<std::int32_t> weights(graph.size(), -1);
  weights[0] = 0;
  mark_weights(graph, weights, 0);

  for (std::int32_t i = 0; i < graph.size(); i++) {
    for (std::int32_t j = 0; j < graph.size(); j++) {
      if (i == j)
        Nodes[i][j].push_back(graph[i].size());
      else {
        auto paths = graph.get_simple_paths(i, j);

        auto it = paths.begin();
        while (it != paths.end()) {
          bool found = false;
          for (std::int32_t p = 1; p < it->size() - 1 && !found; p++) {
            std::int32_t pv = weights[it->at(p - 1)];
            std::int32_t cv = weights[it->at(p)];
            std::int32_t nv = weights[it->at(p + 1)];
            if (pv == nv && pv + 1 == cv) found = true;
          }
          if (found)
            it = paths.erase(it);
          else
            it++;
        }
        if (paths.empty()) return false;
        for (const auto& path : paths) {
          auto links = graph[path[0]].links_to(path[1]);
          for (std::int32_t l : links)
            if (std::find(Nodes[i][j].begin(), Nodes[i][j].end(), l) ==
                Nodes[i][j].end())
              Nodes[i][j].push_back(l);
        }
      }
    }
  }
  return true;
}
bool RoutingTable::LoadMeshXY(const Graph& graph) {
  Nodes.resize(graph.size(), Node(graph.size()));
  if (graph.size() < 1) return true;

  std::int32_t w = 1;
  std::int32_t h = 1;
  while (graph[w - 1].links_to(w).size() > 0) w++;
  while (graph[w * (h - 1)].links_to(w * h).size() > 0) h++;

  for (std::int32_t x = 0; x < w; x++) {
    for (std::int32_t y = 0; y < h; y++) {
      std::int32_t id = y * w + x;
      std::int32_t du = -1;
      std::int32_t dl = -1;
      std::int32_t dd = -1;
      std::int32_t dr = -1;
      if (y + 1 < h) du = graph[id].links_to((y + 1) * w + x)[0];
      if (x - 1 >= 0) dl = graph[id].links_to(y * w + x - 1)[0];
      if (y - 1 >= 0) dd = graph[id].links_to((y - 1) * w + x)[0];
      if (x + 1 < w) dr = graph[id].links_to(y * w + x + 1)[0];

      for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
          std::int32_t did = dy * w + dx;

          if (dx > x)
            Nodes[id][did].push_back(dr);  // return (int)Direction.East;
          else if (dx < x)
            Nodes[id][did].push_back(dl);  // return (int)Direction.West;
          else if (dy > y)
            Nodes[id][did].push_back(du);  // return (int)Direction.South;
          else if (dy < y)
            Nodes[id][did].push_back(dd);  // return (int)Direction.North;
          else
            Nodes[id][did].push_back(graph[id].size());
        }
      }
    }
  }

  return true;
}
bool RoutingTable::LoadCirculantPairExchange(const Graph& graph) {
  if (graph.size() < 1) return true;
  if (graph[0].size() < 2) return false;
  std::int32_t generator = graph[0][0];
  for (std::int32_t i = 1; i < graph[0].size(); i++)
    generator = std::min(generator, graph[0][i]);

  for (std::int32_t i = 0; i < graph.size(); i++) {
    for (std::int32_t j = 0; j < graph.size(); j++) {
      if (i == j) {
        Nodes[i][j].push_back(graph[i].size());
        continue;
      }

      auto vec = PerformPairExchange(graph.size(), generator, i, j);
      if (vec.first > 0) {
        std::int32_t src = (i + generator) % graph.size();
        for (std::int32_t l : graph[i].links_to(src)) Nodes[i][j].push_back(l);
      }
      if (vec.first < 0) {
        std::int32_t src = i - generator;
        if (src < 0) src += graph.size();
        for (std::int32_t l : graph[i].links_to(src)) Nodes[i][j].push_back(l);
      }
      if (vec.second > 0) {
        std::int32_t src = (i + generator + 1) % graph.size();
        for (std::int32_t l : graph[i].links_to(src)) Nodes[i][j].push_back(l);
      }
      if (vec.second < 0) {
        std::int32_t src = i - (generator + 1);
        if (src < 0) src += graph.size();
        for (std::int32_t l : graph[i].links_to(src)) Nodes[i][j].push_back(l);
      }
    }
  }

  for (std::int32_t i = 0; i < graph.size(); i++) {
    auto vec = PerformPairExchange(graph.size(), generator, 0, i);
    if (vec.first > 0) {
      std::int32_t from_zero_to_i = generator;

      for (std::int32_t j = 0; j < graph.size(); j++) {
        std::int32_t target = (i + j) % graph.size();
        if (j == target)
          Nodes[j][target].push_back(graph[j].size());
        else
          for (std::int32_t l :
               graph[j].links_to((from_zero_to_i + j) % graph.size())) {
            // if (std::find(Nodes[j][target].begin(), Nodes[j][target].end(),
            // l) == Nodes[j][target].end()) std::cout << "SHIT";
            Nodes[j][target].push_back(l);
          }
      }
    }
    if (vec.first < 0) {
      std::int32_t from_zero_to_i = graph.size() - generator;

      for (std::int32_t j = 0; j < graph.size(); j++) {
        std::int32_t target = (i + j) % graph.size();
        if (j == target)
          Nodes[j][target].push_back(graph[j].size());
        else
          for (std::int32_t l :
               graph[j].links_to((from_zero_to_i + j) % graph.size())) {
            // if (std::find(Nodes[j][target].begin(), Nodes[j][target].end(),
            // l) == Nodes[j][target].end()) std::cout << "SHIT";
            Nodes[j][target].push_back(l);
          }
      }
    }
    if (vec.second > 0) {
      std::int32_t from_zero_to_i = generator + 1;

      for (std::int32_t j = 0; j < graph.size(); j++) {
        std::int32_t target = (i + j) % graph.size();
        if (j == target)
          Nodes[j][target].push_back(graph[j].size());
        else
          for (std::int32_t l :
               graph[j].links_to((from_zero_to_i + j) % graph.size())) {
            // if (std::find(Nodes[j][target].begin(), Nodes[j][target].end(),
            // l) == Nodes[j][target].end()) std::cout << "SHIT";
            Nodes[j][target].push_back(l);
          }
      }
    }
    if (vec.second < 0) {
      std::int32_t from_zero_to_i = graph.size() - (generator + 1);

      for (std::int32_t j = 0; j < graph.size(); j++) {
        std::int32_t target = (i + j) % graph.size();
        if (j == target)
          Nodes[j][target].push_back(graph[j].size());
        else
          for (std::int32_t l :
               graph[j].links_to((from_zero_to_i + j) % graph.size())) {
            // if (std::find(Nodes[j][target].begin(), Nodes[j][target].end(),
            // l) == Nodes[j][target].end()) std::cout << "SHIT";
            Nodes[j][target].push_back(l);
          }
      }
    }
  }

  return true;
}
bool RoutingTable::LoadCirculantClockwise(const Graph& graph) {
  if (graph.size() < 1) return true;
  if (graph[0].size() < 2) return false;
  std::int32_t generator_1 = std::min(graph[0][0], graph[0][1]);
  std::int32_t generator_2 = std::max(graph[0][0], graph[0][1]);

  for (std::int32_t i = 2; i < graph[0].size(); i++) {
    generator_1 = std::min(generator_1, graph[0][i]);
    if (graph[0][i] > generator_1)
      generator_2 = std::min(generator_2, graph[0][i]);
  }

  for (std::int32_t i = 0; i < graph.size(); i++) {
    for (std::int32_t j = 0; j < graph.size(); j++) {
      if (i == j)
        Nodes[i][j].push_back(graph[i].size());
      else {
        for (std::int32_t l : graph[i].links_to(PerformClockwise(
                 graph.size(), generator_1, generator_2, i, j)))
          Nodes[i][j].push_back(l);
      }
    }
  }

  for (std::int32_t i = 0; i < graph.size(); i++) {
    std::int32_t from_zero_to_i =
        PerformClockwise(graph.size(), generator_1, generator_2, 0, i);

    for (std::int32_t j = 0; j < graph.size(); j++) {
      std::int32_t target = (i + j) % graph.size();
      if (j == target)
        Nodes[j][target].push_back(graph[j].size());
      else
        for (std::int32_t l :
             graph[j].links_to((from_zero_to_i + j) % graph.size())) {
          // if (Nodes[j][target][0] != l) std::cout << "SHIT";
          Nodes[j][target].push_back(l);
        }
    }
  }

  return true;
}
bool RoutingTable::LoadCirculantAdaptive(const Graph& graph) {
  if (graph.size() < 1) return true;
  if (graph[0].size() < 2) return false;
  std::int32_t generator_1 = std::min(graph[0][0], graph[0][1]);
  std::int32_t generator_2 = std::max(graph[0][0], graph[0][1]);

  for (std::int32_t i = 2; i < graph[0].size(); i++) {
    generator_1 = std::min(generator_1, graph[0][i]);
    if (graph[0][i] > generator_1)
      generator_2 = std::min(generator_2, graph[0][i]);
  }

  for (std::int32_t i = 0; i < graph.size(); i++) {
    for (std::int32_t j = 0; j < graph.size(); j++) {
      if (i == j)
        Nodes[i][j].push_back(graph[i].size());
      else {
        for (std::int32_t l : graph[i].links_to(
                 PerformAdaptive(graph.size(), generator_1, generator_2, i, j)))
          Nodes[i][j].push_back(l);
      }
    }
  }

  for (std::int32_t i = 0; i < graph.size(); i++) {
    std::int32_t from_zero_to_i =
        PerformAdaptive(graph.size(), generator_1, generator_2, 0, i);

    for (std::int32_t j = 0; j < graph.size(); j++) {
      std::int32_t target = (i + j) % graph.size();
      if (j == target)
        Nodes[j][target].push_back(graph[j].size());
      else
        for (std::int32_t l :
             graph[j].links_to((from_zero_to_i + j) % graph.size())) {
          // if (Nodes[j][target][0] != l) std::cout << "SHIT";
          Nodes[j][target].push_back(l);
        }
    }
  }

  return true;
}
bool RoutingTable::LoadCirculantMultiplicative(const Graph& graph) {
  if (graph.size() < 1) return true;

  std::vector<std::int32_t> generators;
  for (std::int32_t i = 0; i < graph[0].size(); i++)
    if (std::find(generators.begin(), generators.end(), graph[0][i]) ==
        generators.end())
      generators.push_back(graph[0][i]);
  std::sort(generators.begin(), generators.end());
  generators.resize(generators.size() / 2);

  for (std::int32_t i = 0; i < graph.size(); i++) {
    for (std::int32_t j = 0; j < graph.size(); j++) {
      if (i == j)
        Nodes[i][j].push_back(graph[i].size());
      else {
        for (std::int32_t l : graph[i].links_to(
                 PerformMultiplicative(graph.size(), generators, i, j)))
          Nodes[i][j].push_back(l);
      }
    }
  }

  // for (std::int32_t i = 0; i < graph.size(); i++)
  //{
  //	std::int32_t from_zero_to_i = -1;
  //	if (i != 0) PerformMultiplicative(graph.size(), generators, 0, i);
  //
  //	for (std::int32_t j = 0; j < graph.size(); j++)
  //	{
  //		std::int32_t target = (i + j) % graph.size();
  //		if (j == target) Nodes[j][target].push_back(graph[j].size());
  //		else for (std::int32_t l : graph[j].links_to((from_zero_to_i + j) %
  //graph.size()))
  //		{
  //			//if (Nodes[j][target][0] != l) std::cout << "SHIT";
  //			Nodes[j][target].push_back(l);
  //		}
  //	}
  // }

  return true;
}
template <std::size_t sz>
void mark_distances(const Graph& graph,
                    std::vector<std::array<std::int32_t, sz>>& distances,
                    const std::array<std::int32_t, sz>& basis, std::size_t b) {
  distances[basis[b]][b] = 0;
  for (std::size_t d = 1; d < graph.size(); d++) {
    for (std::size_t n = 0; n < graph.size(); n++) {
      if (d < distances[n][b]) {
        for (std::int32_t s : graph[n]) {
          if (distances[s][b] == d - 1) {
            distances[n][b] = d;
            break;
          }
        }
      }
    }
  }
}
bool RoutingTable::LoadGreedyPromotion(const Graph& graph) {
  constexpr std::int32_t inf = std::numeric_limits<std::int32_t>::max();

  std::array<std::int32_t, 4> basis = {0, -1, -1, -1};  // A, B, C, D

  std::vector<std::array<std::int32_t, basis.size()>> distances(
      graph.size(), {inf, inf, inf, inf});

  mark_distances(graph, distances, basis, 0);
  std::size_t select = 0;
  for (std::size_t n = 0; n < distances.size(); n++)
    if (distances[n][0] > distances[select][0]) select = n;

  basis[1] = select;
  mark_distances(graph, distances, basis, 1);
  select = 0;
  for (std::size_t n = 0; n < distances.size(); n++) {
    if (distances[n][0] + distances[n][1] >
        distances[select][0] + distances[select][1])
      select = n;
    else if (distances[n][0] + distances[n][1] ==
                 distances[select][0] + distances[select][1] &&
             std::abs(distances[n][0] - distances[n][1]) <
                 std::abs(distances[select][0] - distances[select][1]))
      select = n;
  }

  basis[2] = select;
  mark_distances(graph, distances, basis, 2);
  select = 0;
  for (std::size_t n = 0; n < distances.size(); n++)
    if (std::min({distances[n][0], distances[n][1], distances[n][2]}) >
        std::min(
            {distances[select][0], distances[select][1], distances[select][2]}))
      select = n;

  basis[3] = select;
  mark_distances(graph, distances, basis, 3);

  std::size_t row = 4;
  for (std::size_t i = 0; i < distances.size(); i++) {
    std::cout << "{ ";
    for (std::size_t b = 0; b < basis.size() - 1; b++)
      std::cout << std::setw(2) << std::setfill('0') << distances[i][b] << ", ";
    if (distances[i].empty())
      std::cout << "}";
    else
      std::cout << std::setw(2) << std::setfill('0')
                << distances[i][basis.size() - 1] << " }";
    if (i % row == row - 1)
      std::cout << '\n';
    else
      std::cout << ';';
  }
  for (std::size_t b = 0; b < basis.size(); b++) {
    for (std::size_t i = 0; i < distances.size(); i++) {
      std::cout << std::setw(2) << std::setfill('0') << distances[i][b];
      if (i % row == row - 1)
        std::cout << '\n';
      else
        std::cout << ";";
    }
    std::cout << '\n';
  }

  for (std::size_t s = 0; s < graph.size(); s++) {
    for (std::size_t d = 0; d < graph.size(); d++) {
      auto& node = Nodes[s][d];
      if (s == d)
        node.push_back(graph[s].size());
      else {
        std::array<std::int32_t, basis.size()> delta;
        for (std::size_t b = 0; b < basis.size(); b++)
          delta[b] = distances[d][b] - distances[s][b];

        std::int32_t max_len = std::numeric_limits<std::int32_t>::min();
        for (std::int32_t r = 0; r < graph[s].size(); r++) {
          std::int32_t n = graph[s][r];

          std::array<std::int32_t, basis.size()> delta_n;
          for (std::size_t b = 0; b < basis.size(); b++)
            delta_n[b] = distances[n][b] - distances[s][b];

          std::int32_t len = 0;
          for (std::size_t b = 0; b < basis.size(); b++)
            len += delta[b] * delta_n[b];

          if (len > max_len) {
            max_len = len;
            node.clear();
            node.push_back(r);
          } else if (len == max_len) {
            node.push_back(r);
          }
        }
      }
    }
  }

  return true;
}

void RoutingTable::Adjust(const Graph& src_graph, const Graph& dst_graph) {
  for (std::size_t s = 0; s < Nodes.size(); s++) {
    for (std::size_t d = 0; d < Nodes[s].size(); d++) {
      auto paths = Nodes[s][d];
      Nodes[s][d].clear();
      for (std::int32_t p : paths) {
        if (p < src_graph[s].size()) {
          for (std::int32_t l : dst_graph[s].links_to(src_graph[s][p]))
            Nodes[s][d].push_back(l);
        } else
          Nodes[s][d].push_back(dst_graph[s].size());
      }
    }
  }
}
void RoutingTable::Promote(const Graph& graph) {
  for (std::size_t i = 0; i < Nodes.size(); i++)
    for (std::size_t d = 0; d < Nodes[i].size(); d++)
      for (std::size_t r = 0; r < Nodes[i][d].size(); r++)
        Nodes[i][d][r] += graph[i].size();
}

void RoutingTable::push_back(Node&& node) { Nodes.push_back(std::move(node)); }

RoutingTable::Node& RoutingTable::operator[](std::int32_t node_id) {
  return Nodes[node_id];
}
const RoutingTable::Node& RoutingTable::operator[](std::int32_t node_id) const {
  return Nodes[node_id];
}

bool RoutingTable::IsValid() const { return Nodes.size(); }

void RoutingTable::get_paths_helper(
    const Graph& graph, std::vector<std::vector<std::int32_t>>& paths,
    std::vector<std::int32_t> path, std::int32_t next, std::int32_t d) const {
  path.push_back(next);
  if (next == d) {
    paths.push_back(std::move(path));
    return;
  }
  for (std::int32_t r : Nodes[next][d])
    get_paths_helper(graph, paths, path, graph[next][r], d);
}
std::vector<std::vector<std::int32_t>> RoutingTable::GetPaths(
    const Graph& graph, std::int32_t s, std::int32_t d) const {
  std::vector<std::vector<std::int32_t>> result;
  get_paths_helper(graph, result, std::vector<std::int32_t>(), s, d);
  return result;
}

std::ostream& operator<<(std::ostream& os, const RoutingTable& rt) {
  os << "[\n";
  for (const auto& row : rt.Nodes) {
    os << '[';
    for (std::int32_t i = 0; i < row.size() - 1; i++) {
      const auto& cell = row[i];
      if (cell.size() > 1) {
        os << '[';
        for (std::int32_t j = 0; j < cell.size() - 1; j++)
          os << cell[j] << ", ";
        os << cell[cell.size() - 1];
        os << "], ";
      } else if (cell.size() == 1)
        os << cell[0] << ", ";
      else
        os << "x, ";
    }
    if (row.size()) {
      const auto& cell = row[row.size() - 1];
      if (cell.size() > 1) {
        os << '[';
        for (std::int32_t j = 0; j < cell.size() - 1; j++)
          os << cell[j] << ", ";
        os << cell[cell.size() - 1];
        os << ']';
      } else if (cell.size() == 1)
        os << cell[0];
      else
        os << 'x';
    }
    os << "],\n";
  }
  os << ']';
  return os;
}
