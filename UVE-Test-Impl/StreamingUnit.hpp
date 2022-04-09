#ifndef STREAMING_UNIT_HPP
#define STREAMING_UNIT_HPP

#include <array>
#include <cstddef> // std::size_t
#include <unordered_map>
#include <variant>
#include <vector>
#include <type_traits>
#include "helpers.hpp"

/* Forward declaration of all classes so that they can appear in
  a more natural order */
struct StreamingUnit;
template <typename T>
struct StreamRegister;
struct Dimension;
struct Modifier;

struct StreamingUnit
{
  /* UVE specification is to have 32 streaming/vectorial registers */
  static constexpr std::size_t registerCount = 32;
  /* There are 2 types at play when implementing the UVE specification. A storage
  type, which is how values get stored, and a computation type, the type a value
  should have when doing computations. Using a variant allows us to have almost
  full type-safety when storing/retriving values, including how many elements can
  be contained in a register at a given moment. During computations, we might need
  a raw cast to a signed or floating-point value.  */
  using RegisterType = std::variant<StreamRegister<std::uint8_t>,
                                    StreamRegister<std::uint16_t>,
                                    StreamRegister<std::uint32_t>,
                                    StreamRegister<std::uint64_t>>;
  /* Property defined as public so that this class can be small and have most
  operations happen outside of it */
  std::array<RegisterType, registerCount> registers;
};

/* T is one of std::uint8_t, std::uint16_t, std::uint32_t or std::uint64_t and
  represents the type of the elements of a stream at a given moment. It is the
  type configured to store a value at a given time */
template <typename T>
struct StreamRegister
{
  /* The gem5 implementation was made with 64 bytes, so this value mirrors it.
  It can be changed to another value that is a power of 2, with atleast 8 bytes
  to support at the 64-bit operations */
  static constexpr size_t registerLength = 64; // in Bytes
  /* Last dimension cannot have a modifier */
  static constexpr size_t maxDimensions = 8;
  static constexpr size_t maxModifiers = maxDimensions - 1;

  using elementsType = T;
  /* During computations, we test if two streams have the same element width
  using this property */
  static constexpr size_t elementsWidth = sizeof(elementsType);
  /* This property limits how many elements can be manipulated during a
  computation and also how many can be loaded/stored at a time */
  static constexpr size_t maxAmountElements = registerLength / elementsWidth;

  enum class RegisterType
  {
    Temporary,
    Load,
    Store,
    Duplicate
  };
  enum class RegisterStatus
  {
    NotConfigured,
    Running,
    Finished
  };

  void addDimension(Dimension dim)
  {
    assert_msg("Cannot append more dimensions as the max value was reached", dimensions.size() < maxDimensions);
    dimensions.push_back(dim);
  }

  void startConfiguration(Dimension dim)
  {
    status = RegisterStatus::NotConfigured;
    dimensions.clear();
    dimensions.push_back(dim);
  }

  void endConfiguration()
  {
    status = RegisterStatus::Running;
    if (this->type == RegisterType::Load)
    {
      updateAsLoad();
    }
  }

  void updateStreamValues()
  {
    if (this->type == RegisterType::Load)
    {
      updateAsLoad();
    }
    else if (this->type == RegisterType::Store)
    {
      updateAsStore();
    }
    else if (this->type == RegisterType::Temporary)
    {
      // Do nothing ...
    }
    else if (this->type == RegisterType::Duplicate)
    {
      // TODO: Duplicate value
    }
    else
    {
      assert_msg("Unhandled type of stream was asked to update", false);
    }
  }

private:
  /* Although the more semantically correct choice would be an array, using a
  vector allows us to avoid an index pointer to the last valid element. This design
  allows us to assume every element in the container is valid and any index checking
  operations can be done by calling the vector::size() method */
  std::vector<elementsType> elements;
  /* Same ordeal as above. Although the amount of dimensions is capped, we can avoid
  indexing by just calling the size method */
  std::vector<Dimension> dimensions;
  /* Modifiers are diferent in that they don't have to scale linearly in a stream
  configuration. As such, it is better to have a container that maps a dimension's
  index to its modifier. When updating stream the iterators, we can test if a dimension
  for the given index exists before the calculations */
  std::unordered_map<int, Modifier> modifiers;
  RegisterType type = RegisterType::Temporary;
  RegisterStatus status = RegisterStatus::NotConfigured;

  void updateAsLoad()
  {
    /* TODO: For now, only focus on first dimension */
    auto &dim = dimensions.at(0);
    if (!dim.canProduceElements())
    {
      status = RegisterStatus::Finished;
      return;
    }
    // Get how many elements this dimension can still produce
    const size_t remaindingProducibleElements = dim.size - dim.iter.index;
    // Calculate the max amount elements we can produce per iteration
    const std::size_t eCount = std::min(remaindingProducibleElements, maxAmountElements);
    elements.clear();
    for (std::size_t i = 0; i < eCount; i++)
    {
      auto *offset = static_cast<elementsType *>(dim.offset);
      auto value = offset[(i + dim.iter.index) * dim.stride];
      elements.push_back(value);
    }
    dim.iter.index += eCount;
    if (!dim.canProduceElements())
    {
      status = RegisterStatus::Finished;
    }
  }

  void updateAsStore()
  {
    /* TODO: For now, only focus on first dimension */
    auto &dim = dimensions.at(0);
    if (!dim.canProduceElements())
    {
      status = RegisterStatus::Finished;
      return;
    }
    // Get how many elements this dimension can still produce
    const size_t remaindingProducibleElements = dim.size - dim.iter.index;
    // Calculate the max amount elements we can extract per iteration
    const std::size_t eCount = std::min(remaindingProducibleElements, elements.size());
    for (std::size_t i = 0; i < eCount; i++)
    {
      auto *offset = static_cast<elementsType *>(dim.offset);
      auto value = elements.at(i);
      offset[(i + dim.iter.index) * dim.stride] = value;
    }
    elements.clear();
    dim.iter.index += eCount;
    if (!dim.canProduceElements())
    {
      status = RegisterStatus::Finished;
    }
  }
};

struct Dimension
{
  Dimension(size_t offset, size_t size, size_t stride)
      : offset(offset), size(size), stride(stride)
  {
    iter_offset = offset;
    iter_size = size;
    iter_stride = stride;
    iter_index = 0;
  }

  void resetIterValues()
  {
    // iter_offset = offset;
    // iter_size = size;
    // iter_stride = stride;
    iter_index = 0;
  }

  void advance()
  {
    iter_index++;
  }

  bool isEOD() const
  {
    return iter_index >= iter_size;
  }

  size_t calcOffset() const
  {
    return iter_offset + iter_stride * iter_index;
  }

  bool isLastDimIteration() const
  {
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

  void modStatic(Dimension &dim) const
  {
    size_t valueChange = displacement;
    if (behaviour == Behaviour::Increment)
    {
      /* Nothing changes */
    }
    else if (behaviour == Behaviour::Decrement)
    {
      valueChange *= -1;
    }
    else
    {
      assert_msg("Unexpect behaviour type for a static modifier", false);
    }
    if (target == Target::Offset)
    {
      dim.iter_offset += valueChange;
    }
    else if (target == Target::Size)
    {
      dim.iter_size += valueChange;
    }
    else if (target == Target::Stride)
    {
      dim.iter_stride += valueChange;
    }
    else
    {
      assert_msg("Unexpect target for a static modifier", false);
    }
  }

  void modIndirect(Dimension &dim) const
  {
  }
};

template <typename T> auto makeStreamRegister() {
  if constexpr (std::is_same_v<T, std::uint8_t>) {
    return StreamRegister<std::uint8_t>{};
  } else if constexpr (std::is_same_v<T, std::uint16_t>) {
    return StreamRegister<std::uint16_t>{};
  } else if constexpr (std::is_same_v<T, std::uint32_t>) {
    return StreamRegister<std::uint32_t>{};
  } else if constexpr (std::is_same_v<T, std::uint64_t>) {
    return StreamRegister<std::uint64_t>{};
  } else {
    static_assert(always_false_v<T>,
                  "Cannot create register with this element width");
  }
}

#endif // STREAMING_UNIT_HPP