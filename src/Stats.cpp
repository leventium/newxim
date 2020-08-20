#include "Stats.hpp"



Stats::Stats(const SimulationTimer& timer, std::int32_t node_id, std::int32_t buffers) :
	Timer(timer),
	AVGBufferLoad(buffers, std::make_pair(0.0, 0.0)),
	LastBufferPopOrEmptyTime(buffers, -1.0)
{
	id = node_id;
	total_flits_accepted = 0;
	last_received_flit_time = 0.0;
	max_time_flit_in_network = 0.0;
}

void Stats::UpdateBufferPopOrEmptyTime(std::int32_t buffer)
{
	//if (pop_time - GlobalParams::reset_time < warm_up_time)	return;
	LastBufferPopOrEmptyTime[buffer] = Timer.SystemTime();
}

double Stats::GetBufferPopOrEmptyTime(std::int32_t buffer) const
{
	return LastBufferPopOrEmptyTime[buffer];
}

double Stats::GetMinBufferPopOrEmptyTime() const
{
	double min = LastBufferPopOrEmptyTime[0];
	for (std::int32_t i = 1; i < LastBufferPopOrEmptyTime.size(); i++)
		if (min > LastBufferPopOrEmptyTime[i]) min = LastBufferPopOrEmptyTime[i];
	return min;
}

double Stats::GetMaxTimeFlitInNetwork() const
{
	return max_time_flit_in_network;
}

void Stats::AcceptFlit()
{
	if (Timer.StatisticsTime() < 0.0) return;

	total_flits_accepted++;
}
std::int32_t Stats::GetAcceptedFlits() const
{
	return total_flits_accepted;
}

void Stats::UpdateBufferLoad(std::int32_t buffer, double load)
{
	if (Timer.StatisticsTime() < 0.0) return;

	AVGBufferLoad[buffer].first += 1.0;
	AVGBufferLoad[buffer].second += load;
}
double Stats::GetAVGBufferLoad(std::int32_t channel, std::int32_t channels_count)
{
	auto load = std::make_pair(0.0, 0.0);
	for (int i = 0; i < (AVGBufferLoad.size() - 1) / channels_count; i++)
	{
		auto cl = AVGBufferLoad[channel + i * channels_count];
		load.first += cl.first;
		load.second += cl.second;
	}
	return load.second / load.first;
}

double Stats::getLastReceivedFlitTime() const
{
	return last_received_flit_time;
}

void Stats::receivedFlit(const Flit& flit)
{
	if (Timer.StatisticsTime() < 0.0) return;

	last_received_flit_time = Timer.SimulationTime();

	max_time_flit_in_network = std::max(max_time_flit_in_network, 
		Timer.StatisticsTime() - flit.accept_timestamp);

	std::int32_t i = searchCommHistory(flit.src_id);

	if (i == -1)
	{
		// first flit received from a given source
		// initialize CommHist structure
		CommHistory ch;

		ch.src_id = flit.src_id;
		ch.total_received_flits = 0;

		ch.total_received_packets = 0;
		ch.total_packets_delay = 0.0;
		ch.packets_max_delay = 0.0;

		chist.push_back(ch);

		i = chist.size() - 1;
	}

	if (flit.flit_type == FLIT_TYPE_HEAD)
	{
		double delay = Timer.SystemTime() - flit.timestamp;
		chist[i].total_received_packets++;
		chist[i].total_packets_delay += delay;
		chist[i].packets_max_delay = std::max(chist[i].packets_max_delay, delay);
		//chist[i].delays.push_back(arrival_time - flit.timestamp);
	}

	chist[i].total_received_flits++;
	chist[i].last_received_flit_time = Timer.StatisticsTime();
}

double Stats::getAverageDelay(const int src_id)
{
	//double sum = 0.0;

	int i = searchCommHistory(src_id);

	assert(i >= 0);

	//for (unsigned int j = 0; j < chist[i].delays.size(); j++)
	//	sum += chist[i].delays[j];

	return chist[i].total_packets_delay / chist[i].total_received_packets;// sum / (double)chist[i].delays.size();
}

double Stats::getAverageDelay()
{
	double avg = 0.0;

	for (unsigned int k = 0; k < chist.size(); k++) {
		//unsigned int samples = chist[k].delays.size();
		//if (samples)
		//	avg += (double)samples * getAverageDelay(chist[k].src_id);
		avg += chist[k].total_packets_delay;
	}

	return avg / (double)getReceivedPackets();
}

double Stats::getMaxDelay(const int src_id)
{
	double maxd = -1.0;

	int i = searchCommHistory(src_id);

	assert(i >= 0);

	//for (unsigned int j = 0; j < chist[i].delays.size(); j++)
	//	if (chist[i].delays[j] > maxd) {
	//		maxd = chist[i].delays[j];
	//	}
	//return maxd;
	return chist[i].packets_max_delay;
}

double Stats::getMaxDelay()
{
	double maxd = -1.0;

	for (unsigned int k = 0; k < chist.size(); k++) {
		//unsigned int samples = chist[k].delays.size();
		//if (samples) {
		if (chist[k].total_received_packets)
		{
			//double m = getMaxDelay(chist[k].src_id);
			double m = chist[k].packets_max_delay;
			if (m > maxd) maxd = m;
		}
	}

	return maxd;
}

double Stats::getAverageThroughput(const int src_id)
{
	int i = searchCommHistory(src_id);

	assert(i >= 0);

	// not using GlobalParams::simulation_time since 
	// the value must takes into account the invokation time
	// (when called before simulation ended, e.g. turi signal)
	int current_sim_cycles = Timer.StatisticsTime();

	if (chist[i].total_received_flits == 0) return -1.0;
	else return (double)chist[i].total_received_flits / current_sim_cycles;
	//(double) chist[i].last_received_flit_time;
}

double Stats::getAverageThroughput()
{
	double sum = 0.0;

	for (unsigned int k = 0; k < chist.size(); k++) {
		double avg = getAverageThroughput(chist[k].src_id);
		if (avg > 0.0) sum += avg;
	}

	return sum;
}

unsigned int Stats::getReceivedPackets()
{
	int n = 0;

	for (unsigned int i = 0; i < chist.size(); i++)
		//n += chist[i].delays.size();
		n += chist[i].total_received_packets;

	return n;
}

unsigned int Stats::getReceivedFlits()
{
	int n = 0;

	for (unsigned int i = 0; i < chist.size(); i++)
		n += chist[i].total_received_flits;

	return n;
}

unsigned int Stats::getTotalCommunications()
{
	return chist.size();
}

double Stats::getCommunicationEnergy(int src_id, int dst_id)
{
	// NOT YET IMPLEMENTED
	  // Assumptions: minimal path routing, constant packet size
	/*
	  Coord src_coord = id2Coord(src_id);
	  Coord dst_coord = id2Coord(dst_id);

	  int hops =
	  abs(src_coord.x - dst_coord.x) + abs(src_coord.y - dst_coord.y);

	  double energy =
	  hops * (power.getPwrArbitration() + power.getPwrCrossbar() +
		   power.getPwrBuffering() *
		  (GlobalParams::min_packet_size +
		   GlobalParams::max_packet_size) / 2 +
		  power.getPwrRouting() + power.getPwrSelection()
	  );

	  return energy;
	*/
	return -1.0;
}

int Stats::searchCommHistory(int src_id)
{
	for (unsigned int i = 0; i < chist.size(); i++)
		if (chist[i].src_id == src_id)
			return i;

	return -1;
}

void Stats::showStats(int curr_node, std::ostream& out, bool header)
{
	if (header) {
		out << "%"
			<< std::setw(5) << "src"
			<< std::setw(5) << "dst"
			<< std::setw(10) << "delay avg"
			<< std::setw(10) << "delay max"
			<< std::setw(15) << "throughput"
			<< std::setw(13) << "energy"
			<< std::setw(12) << "received" << std::setw(12) << "received\n";
		out << "%"
			<< std::setw(5) << ""
			<< std::setw(5) << ""
			<< std::setw(10) << "cycles"
			<< std::setw(10) << "cycles"
			<< std::setw(15) << "flits/cycle"
			<< std::setw(13) << "Joule"
			<< std::setw(12) << "packets" << std::setw(12) << "flits\n";
	}
	for (unsigned int i = 0; i < chist.size(); i++) {
		out << " "
			<< std::setw(5) << chist[i].src_id
			<< std::setw(5) << curr_node
			<< std::setw(10) << getAverageDelay(chist[i].src_id)
			<< std::setw(10) << getMaxDelay(chist[i].src_id)
			<< std::setw(15) << getAverageThroughput(chist[i].src_id)
			<< std::setw(13) << getCommunicationEnergy(chist[i].src_id,
				curr_node)
			<< std::setw(12) << chist[i].total_received_packets//chist[i].delays.size()
			<< std::setw(12) << chist[i].total_received_flits << '\n';
	}

	out << "% Aggregated average delay (cycles): " << getAverageDelay() << '\n';
	out << "% Aggregated average throughput (flits/cycle): " << getAverageThroughput() << '\n';
}
