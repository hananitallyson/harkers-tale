/* lang.h - Text lookup. The application is English-only; the second
   argument of translate is ignored but kept so call sites still compile. */
#ifndef LANG_H
#define LANG_H

const char *translate(const char *english, const char *portuguese);

#endif