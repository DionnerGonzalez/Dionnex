/* ============================================================================
 * Dionnex Monolithic Kernel - Standard String Utilities Implementation
 * ============================================================================
 */
#include <string.h>

void *memset(void *s, int c, size_t n) {
    unsigned char *p = (unsigned char *)s;
    while (n--) {
        *p++ = (unsigned char)c;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void *memmove(void *dest, const void *src, size_t n) {
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else if (d > s) {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

size_t strlen(const char *s) {
    size_t len = 0;
    while (s && s[len]) {
        len++;
    }
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *orig = dest;
    while ((*dest++ = *src++)) ;
    return orig;
}

char *strncpy(char *dest, const char *src, size_t n) {
    char *orig = dest;
    while (n > 0 && *src) {
        *dest++ = *src++;
        n--;
    }
    while (n > 0) {
        *dest++ = '\0';
        n--;
    }
    return orig;
}

int strcmp(const char *s1, const char *s2) {
    if (!s1 || !s2) return -1;
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    if (!s1 || !s2) return -1;
    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

char *strcat(char *dest, const char *src) {
    char *orig = dest;
    while (*dest) dest++;
    while ((*dest++ = *src++)) ;
    return orig;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char *)s;
        s++;
    }
    if ((char)c == '\0') return (char *)s;
    return NULL;
}

char *strrchr(const char *s, int c) {
    const char *last = NULL;
    do {
        if (*s == (char)c) last = s;
    } while (*s++);
    return (char *)last;
}

char *strstr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for (; *haystack; haystack++) {
        if (*haystack == *needle) {
            const char *h = haystack;
            const char *n = needle;
            while (*h && *n && (*h == *n)) {
                h++;
                n++;
            }
            if (!*n) return (char *)haystack;
        }
    }
    return NULL;
}

static char *strtok_saved = NULL;

char *strtok(char *str, const char *delim) {
    if (str) strtok_saved = str;
    if (!strtok_saved) return NULL;

    while (*strtok_saved && strchr(delim, *strtok_saved)) {
        strtok_saved++;
    }
    if (!*strtok_saved) return NULL;

    char *token_start = strtok_saved;
    while (*strtok_saved && !strchr(delim, *strtok_saved)) {
        strtok_saved++;
    }

    if (*strtok_saved) {
        *strtok_saved = '\0';
        strtok_saved++;
    } else {
        strtok_saved = NULL;
    }

    return token_start;
}

int atoi(const char *s) {
    int res = 0;
    int sign = 1;
    while (*s == ' ' || *s == '\t') s++;
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        res = res * 10 + (*s - '0');
        s++;
    }
    return res * sign;
}

uint32_t atox(const char *s) {
    uint32_t res = 0;
    while (*s == ' ' || *s == '\t') s++;
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) s += 2;

    while (*s) {
        char c = *s;
        uint32_t val = 0;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
        else break;

        res = (res << 4) | val;
        s++;
    }
    return res;
}

void itoa(int32_t val, char *buf, int base) {
    if (base < 2 || base > 36) {
        *buf = '\0';
        return;
    }
    char *ptr = buf;
    int sign = 0;
    if (val < 0 && base == 10) {
        sign = 1;
        val = -val;
    }
    uint32_t uval = (uint32_t)val;
    do {
        uint32_t rem = uval % base;
        *ptr++ = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
        uval /= base;
    } while (uval);

    if (sign) *ptr++ = '-';
    *ptr = '\0';

    // Reverse
    char *start = buf;
    char *end = ptr - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}

void utoa(uint32_t val, char *buf, int base) {
    if (base < 2 || base > 36) {
        *buf = '\0';
        return;
    }
    char *ptr = buf;
    do {
        uint32_t rem = val % base;
        *ptr++ = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
        val /= base;
    } while (val);

    *ptr = '\0';

    // Reverse
    char *start = buf;
    char *end = ptr - 1;
    while (start < end) {
        char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}
