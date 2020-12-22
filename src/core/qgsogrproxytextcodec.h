/***************************************************************************
                             qgsogrproxytextcodec.h
                             -------------
    begin                : June 2020
    copyright            : (C) 2020 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRPROXYTEXTCODEC_H
#define QGSOGRPROXYTEXTCODEC_H

#define SIP_NO_FILE

#include "qgis_core.h"

#include <QTextCodec>

/**
 * \ingroup core
 * \class QgsOgrProxyTextCodec
 * \brief A QTextCodec implementation which relies on OGR to do the text conversion.
 * \note not available in Python bindings
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsOgrProxyTextCodec: public QTextCodec
{
  public:

    /**
     * Constructor for QgsOgrProxyTextCodec, for the specified encoding \a name.
     */
    QgsOgrProxyTextCodec( const QByteArray &name );
    ~QgsOgrProxyTextCodec() override = default;

    QString convertToUnicode( const char *in, int length, ConverterState *state ) const override;
    QByteArray convertFromUnicode( const QChar *in, int length, ConverterState *state ) const override;
    QByteArray name() const override;
    QList<QByteArray> aliases() const override;
    int mibEnum() const override;

    /**
     * Returns a list of supported text codecs.
     */
    static QStringList supportedCodecs();

  private:

    QByteArray mName;
};

#endif // QGSOGRPROXYTEXTCODEC_H
