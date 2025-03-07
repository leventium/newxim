#pragma once
#include <random>
#include <string>
#include <cstdint>

#include "TrafficManager.hpp"

// TODO: Rewrite this shit
class TableTrafficManager : public TrafficManager {
 private:
  mutable std::default_random_engine Random;
  std::uniform_int_distribution<std::int32_t> DestDistribution;
  std::uniform_real_distribution<double> FireDistribution;

  struct Communication {
    std::int32_t src;       // ID of the source node (PE)
    std::int32_t dst;       // ID of the destination node (PE)
    double pir;             // Packet Injection Rate for the link
    std::int32_t t_on;      // Time (in cycles) at which activity begins
    std::int32_t t_off;     // Time (in cycles) at which activity ends
    std::int32_t t_period;  // Period after which activity starts again
  };
  std::vector<Communication> TrafficTable;

 public:
  TableTrafficManager(std::uint32_t seed, std::int32_t count,
                      const std::string& file, double default_pir,
                      double total_global_time);

  bool FirePacket(std::int32_t from, double time) const override;
  std::int32_t FindDestination(std::int32_t from) const override;
};
