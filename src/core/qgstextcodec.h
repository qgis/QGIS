/***************************************************************************
  qgstextcodec.h - QgsTextCodec

 ---------------------
 begin                : 12.1.2026
 copyright            : (C) 2026 by Jan Dalheimer
 email                : jan@dalheimer.de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#pragma once

#include "qgis_core.h"

#include <QString>

#define SIP_NO_FILE

class QTextCodec;

/**
 * \ingroup core
 * \brief Encapsulates a text codec/encoding.
 *
 * The public constructors and the fromName method only allow constructing valid instances of this class, so
 * any instance of this class can be used to encode or decode data.
 *
 * This class should primarily be used when storing and/or passing around a somehow determined encoding is necessary.
 * For simple encoding/decoding where the encoding is known you should prefer using QStringEncoder/QStringDecoder directly.
 */
class CORE_EXPORT QgsTextCodec
{
    explicit QgsTextCodec( const QString &encoding );

  public:
    /**
     * \brief Construct a QgsTextCodec from a legacy QTextCodec instance
     * \deprecated QGIS 4.0. Only included during transition away from QTextCodec usage, do not use for new code.
     */
    [[deprecated]] QgsTextCodec( QTextCodec *codec );
    /**
     * \brief Construct a QgsTextCodec from the list of always available encodings
     */
    QgsTextCodec( const QStringConverter::Encoding encoding = QStringConverter::Encoding::Utf8 );

    /**
     * \brief Construct a QgsTextCodec from a free-form string.
     *
     * The encoding given should be one listed in availableCodecs, otherwise this method
     * will return an empty optional.
     *
     * \return A QgsTextCodec, if the encoding was found
     */
    static std::optional<QgsTextCodec> fromName( const QString &encoding );

    /**
     * \brief Retrieve the name of the encoding that this class represents.
     */
    QString name() const;

    /**
     * \brief Decode the given data using the encoding that this class represents.
     *
     * Uses QStringDecoder.
     */
    QString decode( const QByteArrayView &a ) const;
    /**
     * \brief Encode the given string using the encoding that this class represents.
     *
     * Uses QStringEncoder.
     */
    QByteArray encode( const QStringView &s ) const;

    /**
     * \brief Returns the list of all codecs supported by this class.
     *
     * Any returned value can be passed to fromName to retrieve a valid instance.
     */
    static QStringList availableCodecs();

  private:
    std::variant<QString, QStringConverter::Encoding> mEncoding;
};
