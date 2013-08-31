#ifndef CPP11_FEATURES_H
#define CPP11_FEATURES_H

#if defined(_MSC_VER) && (_MSC_FULL_VER >= 170050727)
// for (T t : collection) { }
#define CPP11_FOR_EACH
// enum class T {};
#define CPP11_ENUM_CLASS
#elif defined(__GNUC__) && (__GNUC__ >= 4) && (__GNUC_MINOR__ >= 6)
#define CPP11_FOR_EACH
#define CPP11_ENUM_CLASS
#endif

#endif // Include guard.