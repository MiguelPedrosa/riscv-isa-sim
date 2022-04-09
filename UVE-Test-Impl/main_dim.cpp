#include <iostream>
#include <vector>
#include <cstddef> //size_t
#include <optional>
#include "Dimension.hpp"
using std::size_t;


int main()
{
	using Type = float;
	constexpr size_t size = 8;
	Type vec[size][size];
	Type i = 0;
	for (size_t y = 0; y < size; y++)
		for (size_t x = 0; x < size; x++)
			vec[y][x] = i++;

	std::vector<Dimension> dimensions;
	ModsType modifiers;
	dimensions.push_back({0, 1, 1});
	modifiers.insert({0, Modifier(Modifier::Type::Static, Modifier::Target::Size, Modifier::Behaviour::Increment, 1)});
	dimensions.push_back({0, size, size});

	for (size_t i = 0; i < (size * size + size) / 2; i++) {
		if (auto offset = generateOffset(dimensions, modifiers)) {
			auto value = *(((Type*) vec) + *offset);
			printf("%f\n", value);
		} else {
			printf("Out of content\n");
		}
	}
}
