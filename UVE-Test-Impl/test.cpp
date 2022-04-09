
#include <cassert>
#include <cstdint>
#include <type_traits>
#define assert_msg(msg, cond) assert(((void)msg, cond))



template <typename Out, typename In> Out readAS(In src) {
  assert_msg("Types Out and In differ in size. Was expected the same", sizeof(In) == sizeof(Out));
  return *reinterpret_cast<Out *>(&src);
}


#include <iostream>
int main()
{
    using Stype = std::uint16_t;
    using Ctype = std::int16_t;
    Ctype OG_1 = -4;
    Ctype OG_2 = -7;

    Stype temp_1 = readAS<Stype>(OG_1);
    Stype temp_2 = readAS<Stype>(OG_2);
    Ctype temp_3 = readAS<Ctype>(temp_1) + readAS<Ctype>(temp_2);
    Stype temp_4 = readAS<Stype>(temp_3);
    Ctype out = readAS<Ctype>(temp_4);

    printf("%d\n", out);
}
