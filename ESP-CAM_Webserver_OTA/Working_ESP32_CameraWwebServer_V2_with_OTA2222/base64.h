#ifndef BASE64_H
#define BASE64_H

#include <Arduino.h>

namespace base64 {
  static const char PROGMEM b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  String encode(const uint8_t *data, size_t len) {
    String result;
    int i = 0;
    while (i < len) {
      uint32_t a = i < len ? data[i++] : 0;
      uint32_t b = i < len ? data[i++] : 0;
      uint32_t c = i < len ? data[i++] : 0;

      uint32_t triple = (a << 16) + (b << 8) + c;

      result += b64_alphabet[(triple >> 18) & 0x3F];
      result += b64_alphabet[(triple >> 12) & 0x3F];
      result += i > len + 1 ? '=' : b64_alphabet[(triple >> 6) & 0x3F];
      result += i > len ? '=' : b64_alphabet[triple & 0x3F];
    }
    return result;
  }
}

#endif
