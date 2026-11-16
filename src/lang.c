/* lang.c - Text lookup. English-only. */
#include "lang.h"

/*
 * translate — Returns the English text. The Portuguese argument is
 * ignored; the signature is preserved for compatibility.
 */
const char *translate(const char *english, const char *portuguese)
{
    (void)portuguese;
    return english;
}