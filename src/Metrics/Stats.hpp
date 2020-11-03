#pragma once
#include <iostream>
#include <iomanip>
#include <vector>
#include "DataStructs.hpp"
#include "Power.hpp"
#include "Hardware/SimulationTimer.hpp"



struct BufferSignature
{
	std::size_t relay_id;
	std::size_t vc_id;

	bool operator<(const BufferSignature& sig) const
	{
		if (relay_id < sig.relay_id) return true;
		if (vc_id < sig.vc_id) return true;
		return false;
	}
};
struct BufferStats
{
	double StuckTimer = -1;
	double MaxStuckDelay = 0;
	double TotalLoad = 0;
	double LoadSamples = 0;
};

class Stats 
{
private:
	const SimulationTimer Timer;
	
	
	std::map<BufferSignature, BufferStats> Buffers;

public:
	Stats(const SimulationTimer& timer);

	void StartStuckTimer(std::size_t relay, std::size_t vc);
	void StopStuckTimer(std::size_t relay, std::size_t vc);
	void PushLoad(std::size_t relay, std::size_t vc, double load);

	double GetMaxBufferStuckDelay(std::size_t relay, std::size_t vc) const;
	double GetMaxBufferStuckDelay() const;
	double GetAverageBufferLoad(std::size_t relay, std::size_t vc) const;
	double GetAverageBufferLoad() const;
};
