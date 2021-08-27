#ifndef COROTEST_H_
#define COROTEST_H_

#include <stdio.h>
#include <stdlib.h>

#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#define EXPECT(x) do { if (!(x)) { fputs("EXPECT(" #x ") failed @ " __FILE__ ":" STRINGIFY(__LINE__) "\n", stderr); exit(1); } } while(0)

#endif
