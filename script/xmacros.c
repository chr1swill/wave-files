#include <stdio.h>

// long add_64(long a, long b) {
//   return a+b;
// }
//
// int add_32(int a, int b) {
//   return a+b;
// }
//
// short add_16(short a, short b) {
//   return a+b;
// }
//
// char add_8(char a, char b) {
//   return a+b;
// }

#define ADDS                                                                   \
  X(long)                                                                      \
  X(int)                                                                       \
  X(short)                                                                     \
  X(char)

#define X(type)                                                                \
  type add_##type(type a, type b) { return a + b; }                            \
// this space here is important or the macro will not work
ADDS
#undef X

int main(void) {
  long a = 1, b = 2;
  int c = 3, d = 4;
  short e = 5, f = 6;
  char g = 7, h = 8;
  printf("add_64(%lu, %lu) = %lu\n", a, b, add_long(a, b));
  printf("add_32(%d, %d) = %d\n", c, d, add_int(c, d));
  printf("add_16(%d, %d) = %d\n", e, f, add_short(e, f));
  printf("add_8(%d, %d) = %d\n", g, h, add_char(g, h));
}
