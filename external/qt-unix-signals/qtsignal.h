
#ifndef QTSIGNAL_EXPORT_H
#define QTSIGNAL_EXPORT_H

#ifdef QTSIGNAL_STATIC_DEFINE
#  define QTSIGNAL_EXPORT
#  define QTSIGNAL_NO_EXPORT
#else
#  ifndef QTSIGNAL_EXPORT
#    ifdef QTSignal_EXPORTS
        /* We are building this library */
#      define QTSIGNAL_EXPORT __attribute__((visibility("default")))
#    else
        /* We are using this library */
#      define QTSIGNAL_EXPORT __attribute__((visibility("default")))
#    endif
#  endif

#  ifndef QTSIGNAL_NO_EXPORT
#    define QTSIGNAL_NO_EXPORT __attribute__((visibility("hidden")))
#  endif
#endif

#ifndef QTSIGNAL_DEPRECATED
#  define QTSIGNAL_DEPRECATED __attribute__ ((__deprecated__))
#endif

#ifndef QTSIGNAL_DEPRECATED_EXPORT
#  define QTSIGNAL_DEPRECATED_EXPORT QTSIGNAL_EXPORT QTSIGNAL_DEPRECATED
#endif

#ifndef QTSIGNAL_DEPRECATED_NO_EXPORT
#  define QTSIGNAL_DEPRECATED_NO_EXPORT QTSIGNAL_NO_EXPORT QTSIGNAL_DEPRECATED
#endif

#if 0 /* DEFINE_NO_DEPRECATED */
#  ifndef QTSIGNAL_NO_DEPRECATED
#    define QTSIGNAL_NO_DEPRECATED
#  endif
#endif

#endif /* QTSIGNAL_EXPORT_H */
