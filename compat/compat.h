#ifndef __COMPAT_H
#define __COMPAT_H

#ifndef arc4random
uint32_t arc4random(void);
#endif

#ifndef strlcat
size_t strlcat(char *, const char *, size_t);
#endif

#ifndef strlcpy
size_t strlcpy(char *, const char *, size_t);
#endif

#endif /* __COMPAT_H */
