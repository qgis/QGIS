/***************************************************************************
    qgssymbolconvertermapboxgl.h
    --------------------
    begin                : February 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSYMBOLCONVERTERMAPBOXGL_H
#define QGSSYMBOLCONVERTERMAPBOXGL_H

#include "qgssymbolconverter.h"

#define SIP_NO_FILE

/**
 * \class QgsSymbolConverterMapBoxGl
 * \ingroup core
 * \brief A symbol converter for converting MapBox GL JSON values to QgsSymbol objects.
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsSymbolConverterMapBoxGl : public QgsAbstractSymbolConverter
{
  public:
    Qgis::SymbolConverterCapabilities capabilities() const override;
    QString name() const override;
    QString formatName() const override;
    QVariant toVariant( const QgsSymbol *symbol, QgsSymbolConverterContext &context ) const override;
    std::unique_ptr< QgsSymbol > createSymbol( const QVariant &variant, QgsSymbolConverterContext &context ) const override;
};

#endif // QGSSYMBOLCONVERTERMAPBOXGL_H
