#ifdef DEBUG
#define LOG(...)                                                               \
  do {                                                                         \
    printf(__VA_ARGS__);                                                       \
  } while (false)
#else
#define LOG(...)                                                               \
  do {                                                                         \
  } while (false)
#endif

#define L(a, b)                                                                \
  do {                                                                         \
    assert(a);                                                                 \
    (a)->left = (b);                                                           \
    if (b) {                                                                   \
      (b)->parent = (a);                                                       \
    }                                                                          \
  } while (false)

#define M(a, b)                                                                \
  do {                                                                         \
    assert(a);                                                                 \
    (a)->middle = (b);                                                         \
    if (b) {                                                                   \
      (b)->parent = (a);                                                       \
    }                                                                          \
  } while (false)

#define R(a, b)                                                                \
  do {                                                                         \
    assert(a);                                                                 \
    (a)->right = (b);                                                          \
    if (b) {                                                                   \
      (b)->parent = (a);                                                       \
    }                                                                          \
  } while (false)