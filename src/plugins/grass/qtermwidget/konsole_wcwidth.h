
/* Markus Kuhn -- 2001-01-12 -- public domain */
/* Adaptions for KDE by Waldo Bastian <bastian@kde.org> */
/*
    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>
*/


#ifndef _KONSOLE_WCWIDTH_H_
#define _KONSOLE_WCWIDTH_H_

// Qt
#include <QtCore/QBool>
#include <QtCore/QString>

int konsole_wcwidth( quint16 ucs );
#if 0
int konsole_wcwidth_cjk( Q_UINT16 ucs );
#endif

int string_width( const QString &txt );

#endif
