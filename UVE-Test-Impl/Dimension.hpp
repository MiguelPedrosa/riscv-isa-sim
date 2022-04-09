#ifndef DIMENSION_HPP
#define DIMENSION_HPP


#include <cstddef> // std::size_t
#include <unordered_map>// TODO: Remove this after testing
#include <vector>// TODO: Remove this after testing
#include <optional>
#include "helpers.hpp"
using std::size_t;


struct Dimension
{
  Dimension(size_t offset, size_t size, size_t stride)
      : offset(offset), size(size), stride(stride) {
    iter_offset = offset;
    iter_size = size;
    iter_stride = stride;
    iter_index = 0;
  }

  void resetIterValues() {
    // iter_offset = offset;
    // iter_size = size;
    // iter_stride = stride;
    iter_index = 0;
  }

  void advance() {
    iter_index++;
  }

  bool isEOD() const {
    return iter_index >= iter_size;
  }

  size_t calcOffset() const {
    return iter_offset + iter_stride * iter_index;
  }

  bool isLastDimIteration() const {
    return iter_index + 1 == iter_size;
  }

private:
  const size_t offset; // TODO: change type during implementation
  const size_t size;
  const size_t stride;
  size_t iter_offset; // TODO: change type during implementation
  size_t iter_size;
  size_t iter_stride;
  size_t iter_index;

  friend class Modifier;
};

struct Modifier
{
  enum class Type { Static, Indirect, CfgVec };
  enum class Target { None, Offset, Size, Stride };
  enum class Behaviour { None, Increment, Decrement };

  Modifier(Type type, Target target = Target::None, Behaviour behaviour = Behaviour::None, size_t displacement = 0)
      : type(type), target(target), behaviour(behaviour), displacement(displacement)
  {
  }

  void modDimension(Dimension &dim) const
  {
    switch (type)
    {
    case Type::Static:
      modStatic(dim);
      break;
    case Type::Indirect:
      modIndirect(dim);
      break;
    case Type::CfgVec:
      /* Do nothing. This case gets handled in generateOffset */
      break;
    default:
      assert_msg("Unhandled Type case in modifiers's modDimension", false);
    }
  }

private:
  /* This should be const, but we cannot create an array<Modifier> and postpone
    the assignment if they are, so for all intents they shouldn't be altered */
  Type type;
  Target target;
  Behaviour behaviour;
  size_t displacement;

  void modStatic(Dimension &dim) const {
    size_t valueChange = displacement;
    if (behaviour == Behaviour::Increment) {
      /* Nothing changes */
    } else if (behaviour == Behaviour::Decrement) {
      valueChange *= -1;
    } else {
      assert_msg("Unexpect behaviour type for a static modifier", false);
    }

    if (target == Target::Offset) {
      dim.iter_offset += valueChange;
    } else if (target == Target::Size) {
      dim.iter_size += valueChange;
    } else if (target == Target::Stride) {
      dim.iter_stride += valueChange;
    } else {
      assert_msg("Unexpect target for a static modifier", false);
    }
  }

  void modIndirect(Dimension &dim) const
  {
  }
};

using ModsType = std::unordered_map<int, Modifier>;



std::optional<size_t>
generateOffset(std::vector<Dimension> &dims, ModsType &mods)
{
	auto dimsSize = dims.size();
	/* We cannot generate any more content after the last iteration of the last dimensions */
	if (dims.at(dimsSize - 1).isEOD()) {
		return std::nullopt;
	}

	/* result will be the final accumulation of all offset calculated per dimension */
	size_t result = 0;
	/* Just like a nested for loop, we start calculating offset from the outermost loop/dimension
		Dimension 0 is calculated outside, because it will always be calculated and it cannot be associated 
		with a modifier that will alterate a dimension below it */
	for (ssize_t i = dimsSize - 1; i > 0; i--) {
		/* calculate current dimension's offset */
		auto& currDim = dims.at(i);
		result += currDim.calcOffset();
	}
	auto& dim0 = dims.at(0);
	result += dim0.calcOffset();

	/* Incrementing works by always stating with the lower dimension and incrementing the next if
		the current reaches an overflow, at which point for the current dimension, values are reset */
	dim0.advance();
	if (dimsSize > 1) {
		for (size_t i = 0; i < dimsSize - 1; i++) {
			auto& currDim = dims.at(i);
			auto& nextDim = dims.at(i + 1);
			if (currDim.isEOD()) {
				// TODO: HANDLE VecCfg modifiers. Be careful of dim0.advance()
				/* A CfgVec modifier prevents us from generating new offsets when the current dimension is done.
					As such, we can only return content after an exterior call to resetIterValues() */
				currDim.resetIterValues();
				nextDim.advance();
				/* */
				auto currMod = mods.find(i);
				// if (currMod.type == Modifier::Type::CfgVec) {
				// 	return std::nullopt;
				// }
				if (currMod != mods.end()) {
					/* If modifiers exists, we have to modify this dimension before its next iteration */
					currMod->second.modDimension(currDim);
				}
			}
		}
	}

	return result;
}


#endif // DIMENSION_HPP