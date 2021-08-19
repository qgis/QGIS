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

#include "fromencodedcomponenthelper.h"

#include <QString>

static bool qt_is_ascii( const char *&ptr, const char *end ) noexcept
{
  while ( ptr + 4 <= end )
  {
    quint32 data = qFromUnaligned<quint32>( ptr );
    if ( data &= 0x80808080U )
    {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
      uint idx = qCountLeadingZeroBits( data );
#else
      const uint idx = qCountTrailingZeroBits( data );
#endif
      ptr += idx / 8;
      return false;
    }
    ptr += 4;
  }
  while ( ptr != end )
  {
    if ( quint8( *ptr ) & 0x80 )
      return false;
    ++ptr;
  }
  return true;
}

/*!
    \a ba contains an 8-bit form of the component and it might be
    percent-encoded already. We can't use QString::fromUtf8 because it might
    contain non-UTF8 sequences. We can't use QByteArray::toPercentEncoding
    because it might already contain percent-encoded sequences. We can't use
    qt_urlRecode because it needs UTF-16 input.

    This method is named qt_urlRecodeByteArray in Qt's internals
*/
QString fromEncodedComponent_helper( const QByteArray &ba )
{
  if ( ba.isNull() )
    return QString();
  // scan ba for anything above or equal to 0x80
  // control points below 0x20 are fine in QString
  const char *in = ba.constData();
  const char *const end = ba.constEnd();
  if ( qt_is_ascii( in, end ) )
  {
    // no non-ASCII found, we're safe to convert to QString
    return QString::fromLatin1( ba, ba.size() );
  }
  // we found something that we need to encode
  QByteArray intermediate = ba;
  intermediate.resize( ba.size() * 3 - ( in - ba.constData() ) );
  uchar *out = reinterpret_cast<uchar *>( intermediate.data() + ( in - ba.constData() ) );
  for ( ; in < end; ++in )
  {
    if ( *in & 0x80 )
    {
      // encode
      *out++ = '%';
      *out++ = encodeNibble( uchar( *in ) >> 4 );
      *out++ = encodeNibble( uchar( *in ) & 0xf );
    }
    else
    {
      // keep
      *out++ = uchar( *in );
    }
  }
  // now it's safe to call fromLatin1
  return QString::fromLatin1( intermediate, out - reinterpret_cast<uchar *>( intermediate.data() ) );
}




