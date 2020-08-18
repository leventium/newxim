#pragma once
#include <vector>
#include "../DataStructs.hpp"
#include "../Utils.hpp"



class Router;
class RoutingAlgorithm
{
public:
	virtual std::vector<std::int32_t> Route(Router& router, const RouteData& routeData) const = 0;
};
