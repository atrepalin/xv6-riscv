#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#include <stdarg.h>

static char *
skip_spaces(char *s) {
  while (*s == ' ' || *s == '\t' || *s == '\n')
    s++;
  return s;
}

static int
strtoi(char *s, int base, int sign, uint64 *out) {
  int neg = 0;
  uint64 x = 0;

  if (sign && *s == '-') {
    neg = 1;
    s++;
  }

  while (*s) {
    int d;
    if (*s >= '0' && *s <= '9')
      d = *s - '0';
    else if (*s >= 'a' && *s <= 'f')
      d = *s - 'a' + 10;
    else if (*s >= 'A' && *s <= 'F')
      d = *s - 'A' + 10;
    else
      break;

    if (d >= base)
      break;

    x = x * base + d;
    s++;
  }

  if (sign && neg)
    *out = -(long)x;
  else
    *out = x;

  return 0;
}

int
vscanf(int fd, const char *fmt, va_list ap) {
  char buf[128];
  int n = read(fd, buf, sizeof(buf) - 1);
  if (n <= 0)
    return -1;
  buf[n] = '\0';

  char *s = buf;
  int assigned = 0;

  for (int i = 0; fmt[i]; i++) {
    if (fmt[i] != '%')
      continue;

    i++;
    if (fmt[i] == 'd') {
      s = skip_spaces(s);
      uint64 val;
      strtoi(s, 10, 1, &val);
      int *p = va_arg(ap, int *);
      *p = (int)val;
      assigned++;
      while (*s && *s != ' ' && *s != '\n' && *s != '\t')
        s++;
    } else if (fmt[i] == 'u') {
      s = skip_spaces(s);
      uint64 val;
      strtoi(s, 10, 0, &val);
      uint *p = va_arg(ap, uint *);
      *p = (uint)val;
      assigned++;
      while (*s && *s != ' ' && *s != '\n' && *s != '\t')
        s++;
    } else if (fmt[i] == 'x') {
      s = skip_spaces(s);
      uint64 val;
      strtoi(s, 16, 0, &val);
      uint *p = va_arg(ap, uint *);
      *p = (uint)val;
      assigned++;
      while (*s && *s != ' ' && *s != '\n' && *s != '\t')
        s++;
    } else if (fmt[i] == 'c') {
      char *p = va_arg(ap, char *);
      *p = *s;
      if (*s)
        s++;
      assigned++;
    } else if (fmt[i] == 's') {
      s = skip_spaces(s);
      char *p = va_arg(ap, char *);
      while (*s && *s != ' ' && *s != '\n' && *s != '\t') {
        *p++ = *s++;
      }
      *p = '\0';
      assigned++;
    }
  }

  return assigned;
}

int
fscanf(int fd, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  return vscanf(fd, fmt, ap);
}

int
scanf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  return vscanf(0, fmt, ap);
}
