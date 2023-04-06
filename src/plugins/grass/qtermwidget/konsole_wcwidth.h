/* $XFree86: xc/programs/xterm/wcwidth.h,v 1.2 2001/06/18 19:09:27 dickey Exp $ */

/* Markus Kuhn -- 2001-01-12 -- public domain */
/* Adaptations for KDE by Waldo Bastian <bastian@kde.org> */
/*
    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>
*/


#ifndef _KONSOLE_WCWIDTH_H_
#define _KONSOLE_WCWIDTH_H_

// Standard
#include <string>

int konsole_wcwidth(wchar_t ucs);

int string_width( const std::wstring & wstr );

#endif
