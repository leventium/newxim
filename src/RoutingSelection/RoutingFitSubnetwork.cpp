#include "RoutingFitSubnetwork.hpp"
#include "../Hardware/Routers/Router.hpp"



RoutingFitSubnetwork::RoutingFitSubnetwork(const RoutingTable& table, const RoutingTable& subnetwork) :
	Table(table), SubnetworkTable(subnetwork)
{
}

std::vector<Connection> RoutingFitSubnetwork::Route(const Router& router, const Flit& flit) const
{
	std::vector<Connection> result;

	const auto& ports = Table[router.LocalID][flit.dst_id];
	for (auto port : ports)
	{
		Connection con = { port, 0 };
		if (router.DestinationFreeSlots(con) >= flit.sequence_length)
		{
			result.push_back(con);
		}
	}

	if (result.empty())
	{
		const auto& ports = SubnetworkTable[router.LocalID][flit.dst_id];
		for (auto port : ports)
		{
			Connection con = { port, 1 };
			if (router.DestinationFreeSlots(con) >= flit.sequence_length)
			{
				result.push_back(con);
			}
		}
	}

	return result;
}
