#include <iostream>
#include <vector>
#include <cstddef> //size_t
#include <optional>
#include "Dimension.hpp"
using std::size_t;


int main()
{
	using Type = float;
	constexpr size_t size = 6;
	Type vec[size][size];
	Type i = 0;
	for (size_t y = 0; y < size; y++)
		for (size_t x = 0; x < size; x++)
			vec[y][x] = i++;

	std::vector<Dimension> dimensions;
	ModsType modifiers;
	dimensions.push_back({0, 4, 1});
	dimensions.push_back({0, 4, size});
	// modifiers.insert({0, Modifier(Modifier::Type::Static, Modifier::Target::Size, Modifier::Behaviour::Increment, 1)});
	modifiers.insert({0, Modifier(Modifier::Type::CfgVec) });
 
	for (size_t i = 0; i < (size * size + size); i++) {
	// for (size_t i = 0; i < 100; i++) {
		if (isStreamDone(dimensions)) {
			printf("Iter %lu: End of Stream\n", i);
			break;
		}
		if (canGenerateOffset(dimensions, modifiers)) {
			auto offset = generateOffset(dimensions);
			auto value = *(((Type*) vec) + offset);
			printf("Iter %lu: %f\n", i, value);
		} else {
			printf("Iter %lu: Cannot generate content\n", i);
			dimensions.at(0).setEndOfDimension(false);
		}
		if (canIterate(dimensions, modifiers))
			updateIteration(dimensions, modifiers);
	}
}
