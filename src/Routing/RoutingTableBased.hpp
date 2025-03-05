#pragma once
#include "Configuration/RoutingTable.hpp"
#include "RoutingAlgorithm.hpp"

class RoutingTableBased : public RoutingAlgorithm {
 private:
  const RoutingTable& Table;

 public:
  RoutingTableBased(const RoutingTable& table) : Table(table) {}

  void Route(const Router& router, const Flit& flit,
             std::vector<Connection>& result) const override {
    for (std::size_t i = 0; i < Table[router.LocalId][flit.dst_id].size();
         i++) {
      result.push_back({Table[router.LocalId][flit.dst_id][i], 0});
    }
  }
};
