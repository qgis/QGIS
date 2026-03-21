/***************************************************************************
    qgssymbolconverteresrirest.h
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
#ifndef QGSSYMBOLCONVERTERESRIREST_H
#define QGSSYMBOLCONVERTERESRIREST_H

#include "qgssymbolconverter.h"

#define SIP_NO_FILE

class QgsLineSymbol;
class QgsFillSymbol;
class QgsMarkerSymbol;


/**
 * \ingroup core
 * \brief A symbol converter for converting ESRI REST JSON symbols.
 *
 * \since QGIS 4.2
 */
class CORE_EXPORT QgsSymbolConverterEsriRest : public QgsAbstractSymbolConverter
{
  public:
    Qgis::SymbolConverterCapabilities capabilities() const override;
    QString name() const override;
    QString formatName() const override;
    QVariant toVariant( const QgsSymbol *symbol, QgsSymbolConverterContext &context ) const override SIP_THROW( QgsNotSupportedException );
    std::unique_ptr< QgsSymbol > createSymbol( const QVariant &variant, QgsSymbolConverterContext &context ) const override;

    /**
     * Converts ESRI JSON color data to a QColor object.
     */
    static QColor convertColor( const QVariant &data );

    /**
     * Converts an ESRI line \a style to a Qt pen style.
     */
    static Qt::PenStyle convertLineStyle( const QString &style );

    /**
     * Converts an ESRI fill \a style to a Qt brush style.
     */
    static Qt::BrushStyle convertFillStyle( const QString &style );

  private:
    static std::unique_ptr< QgsLineSymbol > parseEsriLineSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsFillSymbol > parseEsriFillSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsFillSymbol > parseEsriPictureFillSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsMarkerSymbol > parseEsriMarkerSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsMarkerSymbol > parseEsriPictureMarkerSymbolJson( const QVariantMap &symbolData );
    static std::unique_ptr< QgsMarkerSymbol > parseEsriTextMarkerSymbolJson( const QVariantMap &symbolData );

    static Qgis::MarkerShape parseEsriMarkerShape( const QString &style );

    friend class TestQgsArcGisRestUtils;
};

#endif // QGSSYMBOLCONVERTERESRIREST_H
