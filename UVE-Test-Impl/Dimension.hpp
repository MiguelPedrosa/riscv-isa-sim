#ifndef DIMENSION_HPP
#define DIMENSION_HPP

#include <algorithm>
#include <cstddef> // std::size_t
#include <optional>
#include <numeric>
#include <unordered_map> //TODO: Remove this after testing
#include <vector>        //TODO: Remove this after testing
#include "helpers.hpp"
using std::size_t;

struct Dimension
{
  Dimension(size_t offset, size_t size, size_t stride);

  void resetIndex();

  void resetIterValues();

  void advance();

  bool isLastIteration() const;

  bool triggerIterationUpdate() const;

  bool isEndOfDimension() const;

  void setEndOfDimension(bool b);

  size_t calcOffset() const;

private:
  const size_t offset; // TODO: change type during implementation
  const size_t size;
  const size_t stride;
  size_t iter_offset; // TODO: change type during implementation
  size_t iter_size;
  size_t iter_stride;
  size_t iter_index;
  bool endOfDimension;

  friend class Modifier;
};

struct Modifier
{
  enum class Type
  {
    Static,
    Indirect,
    CfgVec
  };
  enum class Target
  {
    None,
    Offset,
    Size,
    Stride
  };
  enum class Behaviour
  {
    None,
    Increment,
    Decrement
  };

  Modifier(Type type, Target target = Target::None, Behaviour behaviour = Behaviour::None, size_t displacement = 0)
      : type(type), target(target), behaviour(behaviour), displacement(displacement)
  {
  }

  void modDimension(Dimension &dim) const;

  Type getType() const;

private:
  const Type type;
  const Target target;
  const Behaviour behaviour;
  const size_t displacement;

  void modStatic(Dimension &dim) const;

  void modIndirect(Dimension &dim) const;
};

using ModsType = std::unordered_map<int, Modifier>;

size_t generateOffset(std::vector<Dimension> &dimensions)
{
  /* Result will be the final accumulation of all offset calculated per dimension */
  return std::accumulate(dimensions.begin(), dimensions.end(), 0, [](size_t acc, Dimension &dim)
                         {
      dim.setEndOfDimension(dim.isLastIteration());
      return acc + dim.calcOffset(); });
}

bool isDimensionFullyDone(std::vector<Dimension>::const_iterator start, std::vector<Dimension>::const_iterator end)
{
  return std::accumulate(start, end, true, [](bool acc, const Dimension &dim)
                         { return acc && dim.isEndOfDimension(); });
}

bool isStreamDone(const std::vector<Dimension> &dimensions)
{
  return isDimensionFullyDone(dimensions.begin(), dimensions.end());
}

bool canGenerateOffset(const std::vector<Dimension> &dimensions, const ModsType &mods)
{
  /* There are two situations that prevent us from generating offsets:
  1) We are at the last iteration of the outermost dimension
  2) We just finished the last iteration of a dimension and there is a configure
  stream vector modifier at that same dimension. In these cases, the generation can
  only resume after an exterior call to resetIndex() */

  /* The outermost dimension is the last one in the container */
  if (isStreamDone(dimensions))
  {
    return false;
  }

  /* We don't check the last dimension as it cannot have a modifier attached */
  for (size_t i = 0; i < dimensions.size() - 1; i++)
  {
    // auto& currDim = dimensions.at(i);
    auto currentModifierIterator = mods.find(i);
    const bool dimensionsDone = isDimensionFullyDone(dimensions.begin(), dimensions.begin() + i + 1);
    if (dimensionsDone && currentModifierIterator != mods.end())
    {
      auto type = currentModifierIterator->second.getType();
      if (type == Modifier::Type::CfgVec)
      {
        return false;
      }
    }
  }

  return true;
}

void updateIteration(std::vector<Dimension> &dimensions, const ModsType &mods)
{
  if (isStreamDone(dimensions))
  {
    return;
  }
  /* Iteration starts from the innermost dimension and updates the next if the current
  reaches an overflow; at which point, for the current dimension, the values are reset */
  dimensions.at(0).advance();

  /* No extra processing is needed if there is only 1 dimension */
  if (dimensions.size() == 1)
  {
    return;
  }

  for (size_t i = 0; i < dimensions.size() - 1; i++)
  {
    auto &currDim = dimensions.at(i);
    /* The following calculation are only necessary if we ARE in the
    last iteration of a dimension */
    if (!currDim.triggerIterationUpdate())
      continue;

    auto currentModifierIter = mods.find(i);
    const bool modifierExists = currentModifierIter != mods.end();
    currDim.resetIndex();
    currDim.setEndOfDimension(false);
    dimensions.at(i + 1).advance();
    if (modifierExists)
    {
      currentModifierIter->second.modDimension(currDim);
    }
    /* The values at lower dimensions might have been modified. As such, we need
    to reset them before next iteration */
    for (size_t j = 0; j < i; j++)
    {
      dimensions.at(j).resetIterValues();
    }
  }
}

bool canIterate(const std::vector<Dimension> &dimensions, const ModsType &modifiers)
{
  if (isStreamDone(dimensions))
  {
    return false;
  }

  for (const auto &[key, modifier] : modifiers)
  {
    if (modifier.getType() == Modifier::Type::CfgVec)
    {
      if (isDimensionFullyDone(dimensions.begin(), dimensions.begin() + key + 1))
      {
        return false;
      }
    }
  }

  return true;
}

void clearAllEODFlags(std::vector<Dimension> &dimensions)
{
  for (auto &dim : dimensions)
  {
    dim.setEndOfDimension(false);
  }
}

#endif // DIMENSION_HPP