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
      FeatureSymbology, //Keeps the number of features and export symbology per feature
      SymbolLayerSymbology //Exports one feature per symbol layer (considering symbol levels)
    };

    QgsDxfExport();
    ~QgsDxfExport();

    void addLayers( QList< QgsMapLayer* >& layers ) { mLayers = layers; }
    int writeToFile( QIODevice* d, SymbologyExport s = SymbolLayerSymbology );  //maybe add progress dialog? //other parameters (e.g. scale, dpi)?

    void setSymbologyScaleDenominator( double d ) { mSymbologyScaleDenominator = d; }
    double symbologyScaleDenominator() const { return mSymbologyScaleDenominator; }

    void setSymbologyExport( SymbologyExport e ) { mSymbologyExport = e; }
    SymbologyExport symbologyExport() const { return mSymbologyExport; }

  private:

    QList< QgsMapLayer* > mLayers;
    /**Scale for symbology export (used if symbols units are mm)*/
    double mSymbologyScaleDenominator;
    SymbologyExport mSymbologyExport;

    void writeHeader( QTextStream& stream );
    void writeTables( QTextStream& stream );
    void writeEntities( QTextStream& stream );
    void writeEntitiesSymbolLevels( QTextStream& stream );
    void writeEndFile( QTextStream& stream );

    void startSection( QTextStream& stream );
    void endSection( QTextStream& stream );

    void writePolyline( QTextStream& stream, const QgsPolyline& line, const QString& layer, int color,
                        double width = -1, bool closed = false );
    void writeVertex( QTextStream& stream, const QgsPoint& pt, const QString& layer );

    QgsRectangle dxfExtent() const;

    void addFeature( const QgsFeature& fet, QTextStream& stream, const QString& layer, const QgsSymbolLayerV2* symbolLayer );
    double scaleToMapUnits( double value, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits ) const;

    //returns dxf palette index from symbol layer color
    int colorFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer );
    double widthFromSymbolLayer( const QgsSymbolLayerV2* symbolLayer );

    //functions for dxf palette
    static int closestMatch( QRgb pixel, const QVector<QRgb>& palette );
    static int pixel_distance( QRgb p1, QRgb p2 );

};

#endif // QGSDXFEXPORT_H
