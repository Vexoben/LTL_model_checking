#ifndef UTILS_HPP
#define UTILS_HPP

#define failwith(...)                                                   \
  do {                                                                  \
    fprintf(stderr, "\e[1;31mfatal error:\e[0m " __VA_ARGS__);          \
  } while(0)

#endif 