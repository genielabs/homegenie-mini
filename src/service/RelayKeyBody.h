#pragma once
#include <stddef.h>
#include <string.h>

namespace PanelRelay {
namespace KeyBody {
constexpr size_t MaxLength = 512;

inline void whitespace(const char*& p) {
    while (*p && strchr(" \r\n\t", *p)) ++p;
}
inline bool take(const char*& p, char c) {
    whitespace(p);
    if (*p != c) return false;
    ++p;
    return true;
}
// Decode JSON strings restricted to printable ASCII, suitable for an HTTP header.
inline bool string(const char*& p, char* out, size_t capacity) {
    if (!take(p, '"')) return false;
    size_t used = 0;
    while (*p && *p != '"') {
        unsigned c = static_cast<unsigned char>(*p++);
        if (c == '\\') {
            c = *p++;
            if (c == 'u') {
                c = 0;
                for (int i = 0; i < 4; ++i) {
                    char digit = *p++;
                    if (digit >= '0' && digit <= '9') c = c * 16 + digit - '0';
                    else if (digit >= 'a' && digit <= 'f') c = c * 16 + digit - 'a' + 10;
                    else if (digit >= 'A' && digit <= 'F') c = c * 16 + digit - 'A' + 10;
                    else return false;
                }
            } else if (c != '"' && c != '\\' && c != '/') return false;
        }
        if (c < 32 || c > 126 || used + 1 >= capacity) return false;
        out[used++] = static_cast<char>(c);
    }
    if (*p != '"') return false;
    ++p;
    out[used] = 0;
    return true;
}

inline bool parse(const char* body, size_t length, char (&key)[MaxLength + 1]) {
    const char* p = body;
    char field[4];
    if (!take(p, '{') || !string(p, field, sizeof(field)) || strcmp(field, "key") ||
        !take(p, ':') || !string(p, key, sizeof(key)) || !take(p, '}')) return false;
    whitespace(p);
    return p == body + length && (!*key || (*key != ' ' && key[strlen(key)-1] != ' '));
}
}
}
