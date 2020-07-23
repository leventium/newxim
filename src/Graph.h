#pragma once
#include <ostream>
#include <string>
#include <vector>
#include <map>



class Graph
{
public:
	static constexpr int32_t EmptyLink = -1;

	class Node
	{
	private:
		std::vector<int32_t> Links;

	public:
		void push_back(int32_t id, int32_t ch = 1) { while (ch--) Links.push_back(id); }
		int32_t size() const { return Links.size(); }
		int32_t operator[](int32_t i) const { return Links[i]; }
		int32_t& operator[](int32_t i) { return Links[i]; }
		std::vector<int32_t> links_to(int32_t id) const 
		{
			std::vector<int32_t> result;
			result.reserve(2);
			for (int32_t i = 0; i < Links.size(); i++)
				if (Links[i] == id) result.push_back(i);
			return std::move(result);
		}
	};
	struct PathNode
	{
		int32_t IRelay;
		int32_t NodeID;
		int32_t ORelay;
	};

private:
	std::vector<Node> Nodes;

	void find_shortest(std::vector<std::vector<PathNode>>& paths, std::vector<PathNode> path, const std::vector<int32_t>& weights, int32_t dest) const;
	void find_shortest(std::vector<std::vector<int32_t>>& paths, std::vector<int32_t> path, const std::vector<int32_t>& weights, int32_t dest) const;

public:
	Graph(const std::string& path);
	Graph();

	void resize(int32_t size);
	void push_back(Node&& node);

	int32_t size() const;

	Node& operator[](int32_t i);
	const Node& operator[](int32_t i) const;

	friend std::ostream& operator<<(std::ostream& os, const Graph& g);

	std::vector<std::vector<PathNode>> get_paths(int32_t from, int32_t to) const;
	std::vector<std::vector<int32_t>> get_simple_paths(int32_t from, int32_t to) const;
};

