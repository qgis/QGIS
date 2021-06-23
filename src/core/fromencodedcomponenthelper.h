/***************************************************************************
                          fromencodedcomponenthelper.h
                             -------------------
    begin                : 22.06.2021
    copyright            : (C) 2021 by Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define SIP_NO_FILE

#include "qgis_core.h"

#include <QtEndian>


// Mega ewwww. all this is taken from Qt's QUrl::addEncodedQueryItem compatibility helper.
// (I can't see any way to port the below code to NOT require this without breaking
// existing projects.)

inline char toHexUpper( uint value ) noexcept
{
  return "0123456789ABCDEF"[value & 0xF];
}

inline ushort encodeNibble( ushort c )
{
  return ushort( toHexUpper( c ) );
}

/*!
    \a ba contains an 8-bit form of the component and it might be
    percent-encoded already. We can't use QString::fromUtf8 because it might
    contain non-UTF8 sequences. We can't use QByteArray::toPercentEncoding
    because it might already contain percent-encoded sequences. We can't use
    qt_urlRecode because it needs UTF-16 input.

    This method is named qt_urlRecodeByteArray in Qt's internals
*/
QString CORE_EXPORT fromEncodedComponent_helper( const QByteArray &ba );

