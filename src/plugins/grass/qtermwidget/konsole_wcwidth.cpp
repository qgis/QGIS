/* $XFree86: xc/programs/xterm/wcwidth.character,v 1.3 2001/07/29 22:08:16 tsi Exp $ */
/*
 * This is an implementation of wcwidth() and wcswidth() as defined in
 * "The Single UNIX Specification, Version 2, The Open Group, 1997"
 * <http://www.UNIX-systems.org/online.html>
 *
 * Markus Kuhn -- 2001-01-12 -- public domain
 */

#include <QString>

#ifdef HAVE_UTF8PROC
#include <utf8proc.h>
#else
#include <cwchar>
#endif

#include "konsole_wcwidth.h"

int konsole_wcwidth(wchar_t ucs)
{
#ifdef HAVE_UTF8PROC
    utf8proc_category_t cat = utf8proc_category( ucs );
    if (cat == UTF8PROC_CATEGORY_CO) {
        // Co: Private use area. libutf8proc makes them zero width, while tmux
        // assumes them to be width 1, and glibc's default width is also 1
        return 1;
    }
    return utf8proc_charwidth( ucs );
#else
    return wcwidth( ucs );
#endif
}

// single byte char: +1, multi byte char: +2
int string_width( const std::wstring & wstr )
{
    int w = 0;
    for ( size_t i = 0; i < wstr.length(); ++i ) {
        w += konsole_wcwidth( wstr[ i ] );
    }
    return w;
}
