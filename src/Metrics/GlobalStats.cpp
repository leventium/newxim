#include "GlobalStats.hpp"

#include <iomanip>
#include <iostream>

std::size_t GlobalStats::GetActualFlitsReceived() const {
  std::size_t n = 0;
  for (const auto& t : net_.Tiles)
    n += t.ProcessorDevice->ActualFlitsReceived();
  return n;
}
std::size_t GlobalStats::GetActualFlitsAccepted() const {
  std::size_t n = 0;
  for (const auto& t : net_.Tiles) n += t.ProcessorDevice->ActualFlitsSent();
  return n;
}
std::size_t GlobalStats::GetFlitsInBuffers() const {
  std::size_t count = 0;
  for (const auto& t : net_.Tiles)
    count += t.RouterDevice->TotalBufferedFlits();
  return count;
}
std::size_t GlobalStats::GetFlitsInTransmission() const {
  std::size_t count = 0;
  for (const auto& t : net_.Tiles) {
    for (std::size_t r = 0; r < t.RouterDevice->Size(); r++)
      if ((*t.RouterDevice)[r].CanReceive()) count++;

    if ((*t.ProcessorDevice).relay.CanReceive()) count++;
  }
  return count;
}

std::size_t GlobalStats::GetFlitsProduced() const {
  std::size_t result = 0;
  for (const auto& t : net_.Tiles)
    result += t.ProcessorDevice->FlitsProduced();
  return result;
}
std::size_t GlobalStats::GetFlitsAccepted() const {
  std::size_t n = 0;
  for (const auto& t : net_.Tiles) n += t.ProcessorDevice->FlitsSent();
  return n;
}
std::size_t GlobalStats::GetFlitsReceived() const {
  std::size_t n = 0;
  for (const auto& t : net_.Tiles) n += t.ProcessorDevice->FlitsReceived();
  return n;
}
double GlobalStats::GetProduction() const {
  std::size_t total_cycles = Config.SimulationTime() - Config.StatsWarmUpTime();
  return static_cast<double>(GetFlitsProduced()) /
         static_cast<double>(total_cycles);
}
double GlobalStats::GetThroughput() const {
  std::size_t total_cycles = Config.SimulationTime() - Config.StatsWarmUpTime();
  return static_cast<double>(GetFlitsReceived()) /
         static_cast<double>(total_cycles);
}
double GlobalStats::GetIPThroughput() const {
  return GetThroughput() / static_cast<double>(net_.Tiles.size());
}
double GlobalStats::GetAcceptance() const {
  std::size_t total_cycles = Config.SimulationTime() - Config.StatsWarmUpTime();
  return static_cast<double>(GetFlitsAccepted()) /
         static_cast<double>(total_cycles);
}

std::size_t GlobalStats::GetLastReceivedFlitTime() const {
  double result = 0.0;
  for (const auto& t : net_.Tiles)
    if (t.ProcessorDevice->LastReceivedFlitTime() > result)
      result = t.ProcessorDevice->LastReceivedFlitTime();
  return static_cast<std::size_t>(result);
}
std::size_t GlobalStats::GetMaxBufferStuckDelay() const {
  double max_delay = -1;
  for (const auto& t : net_.Tiles) {
    double delay = t.RouterDevice->stats.GetMaxBufferStuckDelay();
    if (delay > max_delay) max_delay = delay;
  }
  return static_cast<std::size_t>(max_delay);
}
std::size_t GlobalStats::GetMaxTimeFlitInNetwork() const {
  double result = 0.0;
  for (const auto& t : net_.Tiles) {
    if (t.ProcessorDevice->MaxTimeFlitInNetwork() > result)
      result = t.ProcessorDevice->MaxTimeFlitInNetwork();

    for (std::size_t r = 0; r < t.RouterDevice->Size(); r++) {
      const auto& relay = (*t.RouterDevice)[r];

      for (std::size_t vc = 0; vc < relay.Size(); vc++) {
        const auto& buffer = relay[vc];
        if (!buffer.Empty()) {
          double span = Config.SimulationTime() - Config.StatsWarmUpTime() -
                        relay[vc].GetOldestAccepted();
          if (span > result) result = span;
        }
      }
    }
  }

  return static_cast<std::size_t>(result);
}

std::size_t GlobalStats::GetPacketsReceived() const {
  std::int32_t n = 0;

  for (const auto& t : net_.Tiles) n += t.ProcessorDevice->PacketsReceived();

  return n;
}
std::size_t GlobalStats::GetFlitsLost() const {
  std::size_t accepted = GetActualFlitsAccepted();
  std::size_t received = GetActualFlitsReceived();
  std::size_t buffered = GetFlitsInBuffers();
  std::size_t transmitting = GetFlitsInTransmission();
  return accepted - received - buffered - transmitting;
}

double GlobalStats::GetAverageDelay() const {
  std::size_t total_packets = 0;
  double avg_delay = 0.0;

  for (const auto& t : net_.Tiles) {
    std::size_t received_packets = t.ProcessorDevice->PacketsReceived();
    if (received_packets) {
      avg_delay += received_packets * t.ProcessorDevice->AverageDelay();
      total_packets += received_packets;
    }
  }

  if (total_packets != 0) {
    avg_delay /= static_cast<double>(total_packets);
  }

  return avg_delay;
}
double GlobalStats::GetMaxDelay() const {
  double maxd = -1.0;
  for (std::size_t i = 0; i < net_.Tiles.size(); i++) {
    double d = -1;
    std::size_t received_packets =
        net_.Tiles[i].ProcessorDevice->PacketsReceived();
    if (received_packets) d = net_.Tiles[i].ProcessorDevice->MaxDelay();
    ;
    if (d > maxd) maxd = d;
  }
  return maxd;
}

double GlobalStats::GetAverageBufferLoad(std::size_t relay,
                                         std::size_t vc) const {
  double sum = 0;
  for (const auto& t : net_.Tiles)
    sum += t.RouterDevice->stats.GetAverageBufferLoad(relay, vc);
  return sum / net_.Tiles.size();
}
double GlobalStats::GetAverageBufferLoad() const {
  double sum = 0;
  for (const auto& t : net_.Tiles)
    sum += t.RouterDevice->stats.GetAverageBufferLoad();
  return sum / net_.Tiles.size();
}

void GlobalStats::ShowBuffers(std::ostream& out) const {
  out << "% Buffer statuses:\n";
  for (std::size_t i = 0; i < Config.TopologyGraph().size(); i++) {
    auto& node = Config.TopologyGraph()[i];
    auto& stats = net_.Tiles[i].RouterDevice->stats;
    for (std::size_t r = 0; r < net_.Tiles[i].RouterDevice->Size(); r++) {
      const auto& relay = (*net_.Tiles[i].RouterDevice)[r];
      for (std::size_t vc = 0; vc < relay.Size(); vc++) {
        if (r < node.size())
          out << '[' << node[r];
        else
          out << "[L";
        out << "--(" << r << ':' << vc << ")->" << i << ']';
        out << ": R(" << stats.GetBufferFlitsReceived(r, vc) << ") D(";
        out << static_cast<std::int32_t>(stats.GetMaxBufferStuckDelay(r, vc))
            << ") ";
        out << relay[vc] << '\n';
      }
    }
  }
}

void GlobalStats::ShowDistribution(std::ostream& out) const {
  out << "% Flits distribution:\n";
  for (auto& tile : net_.Tiles) {
    out << std::setfill('0') << std::setw(4) << tile.ProcessorDevice->local_id
        << ": R(" << tile.ProcessorDevice->FlitsReceived() << ") S("
        << tile.ProcessorDevice->FlitsSent() << ")\n";
  }
}

void GlobalStats::Update() {
  // if (net_.Timer.SimulationTime() < Config.SimulationTime() - 10) return;
  //
  // std::cout << "Cycle " << net_.Timer.SimulationTime() << ":\n";
  // ShowBuffers(std::cout);
  //
  // for (const auto& tile : net_.Tiles)
  //{
  //	auto rt = tile.RouterDevice->GetReservationTable();
  //	if (rt)
  //	{
  //		std::cout << "Reservation table " << tile.RouterDevice->LocalID <<
  //":\n"; 		std::cout << *rt;
  //	}
  // }
}

void GlobalStats::FinishStats() const {
  for (std::size_t i = 0; i < Config.TopologyGraph().size(); i++) {
    auto& node = Config.TopologyGraph()[i];
    auto& stats = net_.Tiles[i].RouterDevice->stats;
    for (std::size_t r = 0; r < net_.Tiles[i].RouterDevice->Size(); r++) {
      const auto& relay = (*net_.Tiles[i].RouterDevice)[r];
      for (std::size_t vc = 0; vc < relay.Size(); vc++) {
        stats.StopStuckTimer(r, vc);
      }
    }
  }
}

GlobalStats::GlobalStats(sc_module_name name, const ::Network& network,
                         const Configuration& config)
    : net_(network), Config(config) {
  if (config.ReportCycleResult()) {
    SC_METHOD(Update);
    sensitive << reset << clock.pos();
  }

  clock(network.clock);
  reset(network.reset);
}

std::ostream& operator<<(std::ostream& out, const GlobalStats& gs) {
  gs.FinishStats();

  if (gs.Config.JsonResult()) {
    out << "% Result: ";
    out << "{";
    out << "\"total_produced_flits\":" << gs.GetFlitsProduced() << ",";
    out << "\"total_accepted_flits\":" << gs.GetFlitsAccepted() << ",";
    out << "\"total_received_flits\":" << gs.GetFlitsReceived() << ",";
    out << "\"network_production_flits_cycle\":" << gs.GetProduction() << ",";
    out << "\"network_acceptance_flits_cycle\":" << gs.GetAcceptance() << ",";
    out << "\"network_throughput_flits_cycle\":" << gs.GetThroughput() << ",";
    out << "\"ip_throughput_flits_cycle_ip\":" << gs.GetIPThroughput() << ",";
    out << "\"last_time_flit_received_cycles\":" << gs.GetLastReceivedFlitTime()
        << ",";
    out << "\"max_buffer_stuck_delay_cycles\":" << gs.GetMaxBufferStuckDelay()
        << ",";
    out << "\"max_time_flit_in_network_cycles\":"
        << gs.GetMaxTimeFlitInNetwork() << ",";
    out << "\"total_received_packets\":" << gs.GetPacketsReceived() << ",";
    out << "\"total_flits_lost\":" << gs.GetFlitsLost() << ",";
    out << "\"global_average_delay_cycles\":" << gs.GetAverageDelay() << ",";
    out << "\"max_delay_cycles\":" << gs.GetMaxDelay() << ",";
    out << "\"average_buffer_utilization\":" << gs.GetAverageBufferLoad() << "";
    out << "}";
  } else {
    out << "% Total produced flits: " << gs.GetFlitsProduced() << '\n';
    out << "% Total accepted flits: " << gs.GetFlitsAccepted() << '\n';
    out << "% Total received flits: " << gs.GetFlitsReceived() << '\n';
    out << "% Network production (flits/cycle): " << gs.GetProduction() << '\n';
    out << "% Network acceptance (flits/cycle): " << gs.GetAcceptance() << '\n';
    out << "% Network throughput (flits/cycle): " << gs.GetThroughput() << '\n';
    out << "% IP throughput (flits/cycle/IP): " << gs.GetIPThroughput() << '\n';
    out << "% Last time flit received (cycles): "
        << gs.GetLastReceivedFlitTime() << '\n';
    out << "% Max buffer stuck delay (cycles): " << gs.GetMaxBufferStuckDelay()
        << '\n';
    out << "% Max time flit in network (cycles): "
        << gs.GetMaxTimeFlitInNetwork() << '\n';
    out << "% Total received packets: " << gs.GetPacketsReceived() << '\n';
    out << "% Total flits lost: " << gs.GetFlitsLost() << '\n';
    out << "% Global average delay (cycles): " << gs.GetAverageDelay() << '\n';
    out << "% Max delay (cycles): " << gs.GetMaxDelay() << '\n';
    out << "% Average buffer utilization: " << gs.GetAverageBufferLoad()
        << '\n';
    if (gs.Config.ReportFlitTrace()) {
      out << *gs.net_.Tracer;
    }
    if (gs.Config.ReportBuffers()) {
      gs.ShowBuffers(out);
    }
    if (gs.Config.ReportDistribution()) {
      gs.ShowDistribution(out);
    }
  }

  return out;
}
