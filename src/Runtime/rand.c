#include <SDKVersion.h>
#include <stdlib.h>

static unsigned long int next = 1;

#if SDK_VERSION < SDK_VERSION_DOLPHIN_2004_04
int rand(void) {
  next = next * 1103515245 + 12345;
  return (next >> 16) & 0x7FFF;
}

void srand(unsigned int seed) { next = seed; }
#else
int rand(void) {
  next = next * 1103515245 + 12345;
  return (next >> 16) & 0x7FFF;
}

void srand(unsigned int seed) { next = seed; }
#endif
