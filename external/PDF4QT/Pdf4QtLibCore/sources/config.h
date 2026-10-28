#define PDF4QT_PLUGINS_RELATIVE_PATH ""
#define PDF4QT_TRANSLATIONS_RELATIVE_PATH ""


#if ( __GNUC__ > 4 || ( __GNUC__ == 4 && __GNUC_MINOR__ >= 6 ) ) || defined( __clang__ )

#define Q_NOWARN_DEPRECATED_PUSH _Pragma( "GCC diagnostic push" ) _Pragma( "GCC diagnostic ignored \"-Wdeprecated-declarations\"" );
#define Q_NOWARN_DEPRECATED_POP _Pragma( "GCC diagnostic pop" );
#define Q_NOWARN_UNREACHABLE_PUSH
#define Q_NOWARN_UNREACHABLE_POP

#elif defined( _MSC_VER )

#define Q_NOWARN_DEPRECATED_PUSH __pragma( warning( push ) ) __pragma( warning( disable : 4996 ) )
#define Q_NOWARN_DEPRECATED_POP __pragma( warning( pop ) )
#define Q_NOWARN_UNREACHABLE_PUSH __pragma( warning( push ) ) __pragma( warning( disable : 4702 ) )
#define Q_NOWARN_UNREACHABLE_POP __pragma( warning( pop ) )
#else

#define Q_NOWARN_DEPRECATED_PUSH
#define Q_NOWARN_DEPRECATED_POP
#define Q_NOWARN_UNREACHABLE_PUSH
#define Q_NOWARN_UNREACHABLE_POP

#endif
