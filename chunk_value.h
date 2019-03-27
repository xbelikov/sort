#ifndef CHUNK_VALUE_H
#define CHUNK_VALUE_H

struct ChunkValue
{
	int value;
	int chunk;
};

class ChunkValueCpm
{
public:
	bool operator()(const ChunkValue& lhs, const ChunkValue& rhs) const
	{
		return lhs.value > rhs.value;
	}
};

#endif //CHUNL_VALUE_H