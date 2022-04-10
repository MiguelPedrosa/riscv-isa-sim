#ifndef DIMENSION_HPP
#define DIMENSION_HPP

#include <cstddef> // std::size_t
#include "helpers.hpp"

struct Dimension
{
  /* TODO: Change offset type when moving to spike */
  Dimension(void* offset, std::size_t size, std::size_t stride);

  void* getOffset() const
  {
    /* TODO: Delete this when moving to spike */
    return offset;
  }

  void resetIndex();

  void resetIterValues();

  void advance();

  bool isLastIteration() const;

  bool triggerIterationUpdate() const;

  bool isEndOfDimension() const;

  void setEndOfDimension(bool b);

  std::size_t calcOffset() const;

private:
  void* const offset; // TODO: change type during implementation
  const std::size_t size;
  const std::size_t stride;
  void* iter_offset; // TODO: change type during implementation
  std::size_t iter_size;
  std::size_t iter_stride;
  std::size_t iter_index;
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

  Modifier(Type type, Target target = Target::None, Behaviour behaviour = Behaviour::None, std::size_t displacement = 0)
    : type(type), target(target), behaviour(behaviour), displacement(displacement)
  {}

  void modDimension(Dimension& dim) const;

  Type getType() const;

private:
  const Type type;
  const Target target;
  const Behaviour behaviour;
  const std::size_t displacement;

  void modStatic(Dimension& dim) const;

  void modIndirect(Dimension& dim) const;
};

#endif // DIMENSION_HPP