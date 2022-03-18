
#ifndef EDLIB_EXPORT_H
#define EDLIB_EXPORT_H

#ifdef EDLIB_STATIC_DEFINE
#  define EDLIB_EXPORT
#  define EDLIB_NO_EXPORT
#else
#  ifndef EDLIB_EXPORT
#    ifdef edlib_EXPORTS
        /* We are building this library */
#      define EDLIB_EXPORT 
#    else
        /* We are using this library */
#      define EDLIB_EXPORT 
#    endif
#  endif

#  ifndef EDLIB_NO_EXPORT
#    define EDLIB_NO_EXPORT 
#  endif
#endif

#ifndef EDLIB_DEPRECATED
#  define EDLIB_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef EDLIB_DEPRECATED_EXPORT
#  define EDLIB_DEPRECATED_EXPORT EDLIB_EXPORT EDLIB_DEPRECATED
#endif

#ifndef EDLIB_DEPRECATED_NO_EXPORT
#  define EDLIB_DEPRECATED_NO_EXPORT EDLIB_NO_EXPORT EDLIB_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef EDLIB_NO_DEPRECATED
#    define EDLIB_NO_DEPRECATED
#  endif
#endif

#endif /* EDLIB_EXPORT_H */
