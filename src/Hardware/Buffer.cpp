#include "Buffer.hpp"
#include <cassert>



void Buffer::Reserve(std::size_t bms)
{
	assert(bms > 0);

	max_buffer_size = bms;
}
std::size_t Buffer::GetCapacity() const
{
	return max_buffer_size;
}
std::size_t Buffer::GetFreeSlots() const
{
	return max_buffer_size - buffer.size();
}

bool Buffer::Full() const
{
	return buffer.size() == max_buffer_size;
}
bool Buffer::Empty() const
{
	return buffer.size() == 0;
}

void Buffer::Clear()
{
	buffer = std::queue<Flit>();
}
void Buffer::Push(const Flit& flit)
{
	if (Full()) assert(false);
	else buffer.push(flit);
}
Flit Buffer::Pop()
{
	Flit f;

	if (Empty()) assert(false);
	else 
	{
		f = buffer.front();
		buffer.pop();
	}

	return f;
}
Flit Buffer::Front() const
{
	Flit f;

	if (Empty()) assert(false);
	else f = buffer.front();
	
	return f;
}
std::int32_t Buffer::Size() const
{
	return static_cast<std::int32_t>(buffer.size());
}

double Buffer::GetOldest() const
{
	auto copy = buffer;
	double result = copy.front().timestamp;
	copy.pop();
	while (!copy.empty())
	{
		if (copy.front().timestamp < result) result = copy.front().timestamp;
		copy.pop();
	}
	return result;
}
double Buffer::GetOldestAccepted() const
{
	auto copy = buffer;
	double result = copy.front().accept_timestamp;
	copy.pop();
	while (!copy.empty())
	{
		if (copy.front().accept_timestamp < result) result = copy.front().accept_timestamp;
		copy.pop();
	}
	return result;
}
double Buffer::GetLoad() const
{
	return static_cast<double>(Size()) / static_cast<double>(GetCapacity());
}

std::ostream& operator<<(std::ostream& os, const Buffer& b)
{
	std::queue<Flit> m = b.buffer;

	constexpr char t[] = "HBT";

	os << '[';
	while (m.size() > 1)
	{
		Flit f = m.front();
		m.pop();
		os << t[static_cast<std::int32_t>(f.flit_type)] << f.sequence_no << '(' << f.src_id << "->" << f.dst_id << ") | ";
	}
	if (!m.empty())
	{
		Flit f = m.front();
		os << t[static_cast<std::int32_t>(f.flit_type)] << f.sequence_no << '(' << f.src_id << "->" << f.dst_id << ')';
	}
	os << ']';

	return os;
}
