/***************************************************************************
                         qgsdxfexport.h
                         --------------
    begin                : September 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDXFEXPORT_H
#define QGSDXFEXPORT_H

#include "qgsgeometry.h"
#include "qgssymbolv2.h"
#include <QColor>
#include <QList>

class QgsMapLayer;
class QgsPoint;
class QgsSymbolLayerV2;
class QIODevice;
class QTextStream;

class QgsDxfExport
{
  public:
    enum SymbologyExport
    {
      NoSymbology = 0, //export only data
      FeatureSymbology, //Keeps the number of features and export symbology per feature (using the first symbol level)
      SymbolLayerSymbology //Exports one feature per symbol layer (considering symbol levels)
    };

    QgsDxfExport();
    ~QgsDxfExport();

    void addLayers( QList< QgsMapLayer* >& layers ) { mLayers = layers; }
    int writeToFile( QIODevice* d );  //maybe add progress dialog? //other parameters (e.g. scale, dpi)?

    void setSymbologyScaleDenominator( double d ) { mSymbologyScaleDenominator = d; }
    double symbologyScaleDenominator() const { return mSymbologyScaleDenominator; }

    void setMapUnits( QGis::UnitType u ) { mMapUnits = u; }
    QGis::UnitType mapUnits() const { return mMapUnits; }

    void setSymbologyExport( SymbologyExport e ) { mSymbologyExport = e; }
    SymbologyExport symbologyExport() const { return mSymbologyExport; }

  private:

    QList< QgsMapLayer* > mLayers;
    /**Scale for symbology export (used if symbols units are mm)*/
    double mSymbologyScaleDenominator;
    SymbologyExport mSymbologyExport;
    QGis::UnitType mMapUnits;

    QVector<QRgb> mDxfColorPalette;

    static double mDxfColors[][3];

    void writeHeader( QTextStream& stream );
    void writeTables( QTextStream& stream );
    void writeEntities( QTextStream& stream );
    void writeEntitiesSymbolLevels( QTextStream& stream, QgsVectorLayer* layer );
    void writeEndFile( QTextStream& stream );

    void startSection( QTextStream& stream );
    void endSection( QTextStream& stream );

    void writePolyline( QTextStream& stream, const QgsPolyline& line, const QString& layer, int color,
                        double width = -1, bool polygon = false );
    void writeVertex( QTextStream& stream, const QgsPoint& pt, const QString& layer );

    QgsRectangle dxfExtent() const;

    void addFeature( const QgsFeature& fet, QTextStream& stream, const QString& layer, const QgsSymbolLayerV2* symbolLayer );
    double scaleToMapUnits( double value, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits ) const;

    //returns dxf palette index from symbol layer color
    int colorFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer );
    double widthFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer );

    //functions for dxf palette
    static int closestColorMatch( QRgb pixel );
    static int color_distance( QRgb p1, int index );
    static QRgb createRgbEntry( qreal r, qreal g, qreal b );

    //helper functions for symbology export
    QgsRenderContext renderContext() const;
    void startRender( QgsVectorLayer* vl ) const;
    void stopRender( QgsVectorLayer* vl ) const;
    static double mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits );
};

#endif // QGSDXFEXPORT_H
