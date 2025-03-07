#pragma once
#include <random>

#include "TrafficManager.hpp"

class HotspotTrafficManager : public TrafficManager {
 private:
  mutable std::default_random_engine Random;
  std::uniform_int_distribution<std::int32_t> DestDistribution;
  std::uniform_real_distribution<double> FireDistribution;
  const double PacketInjectionRate;

  std::vector<std::pair<std::int32_t, std::int32_t>> TrafficLoad;
  std::vector<std::int32_t> Destinations;

 public:
  HotspotTrafficManager(
      std::uint32_t seed, std::int32_t count, double pir,
      const std::vector<
          std::pair<std::int32_t, std::pair<std::int32_t, std::int32_t>>>&
          hotspots);

  virtual bool FirePacket(std::int32_t from, double time) const override;
  virtual std::int32_t FindDestination(std::int32_t from) const override;
};