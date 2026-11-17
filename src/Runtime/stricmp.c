#include <ctype.h>
#include <string.h>

int stricmp(const char* s1, const char* s2) {
  signed char left, right;

  do {
    left = tolower((signed char)*s1++);
    right = tolower((signed char)*s2++);
    if (left < right) {
      return -1;
    }
    if (left > right) {
      return 1;
    }
  } while (left != '\0');

  return 0;
}
