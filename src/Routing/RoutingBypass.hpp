#pragma once
#include "Configuration/Graph/Graph.hpp"
#include "Configuration/RoutingTable.hpp"
#include "RoutingAlgorithm.hpp"

class RoutingBypass : public RoutingAlgorithm {
 private:
  const RoutingTable &Table;
  const RoutingTable &SubnetworkTable;

  static bool VectorContains(const std::vector<std::int32_t> &vec,
                             std::int32_t v) {
    return std::find(vec.begin(), vec.end(), v) != vec.end();
  }
  static bool VectorContains(const std::vector<std::vector<std::int32_t>> &vec,
                             std::int32_t v) {
    for (const auto &sub_vec : vec) {
      if (std::find(sub_vec.begin(), sub_vec.end(), v) != sub_vec.end())
        return true;
    }
    return false;
  }

 public:
  RoutingBypass(const RoutingTable &table, const RoutingTable &subnetwork)
      : Table(table), SubnetworkTable(subnetwork) {}

  void Route(const Router &router, const Flit &flit,
             std::vector<Connection> &result) const override {
    const auto &ports = Table[router.LocalId][flit.dst_id];
    for (auto port : ports) {
      Connection con = {port, 0};
      if (!router.CanSend(con)) continue;

      if (router.DestinationFreeSlots(con) > flit.sequence_length + 4) {
        result.push_back(con);
      }
    }

    if (result.empty()) {
      const auto &ports = SubnetworkTable[router.LocalId][flit.dst_id];
      for (auto port : ports) {
        Connection con = {port, 0};
        if (!router.CanSend(con)) continue;

        if (VectorContains(SubnetworkTable[router.LocalId], flit.port_in)) {
          if (router.DestinationFreeSlots(con) >= flit.sequence_length) {
            result.push_back(con);
          }
        } else {
          if (router.DestinationFreeSlots(con) > flit.sequence_length + 4) {
            result.push_back(con);
          }
        }
      }
    }
  }
};
