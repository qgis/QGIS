/***************************************************************************
                         qgsdxfexport.sip
                         ----------------
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

class QgsDxfExport
{
%TypeHeaderCode
#include <qgsdxfexport.h>
%End
  public:
    enum SymbologyExport
    {
      NoSymbology, //export only data
      FeatureSymbology, //Keeps the number of features and export symbology per feature (using the first symbol level)
      SymbolLayerSymbology //Exports one feature per symbol layer (considering symbol levels)
    };

    QgsDxfExport();
    ~QgsDxfExport();

    void addLayers( QList< QgsMapLayer* >& layers );
    int writeToFile( QIODevice* d );

    void setSymbologyScaleDenominator( double d );
    double symbologyScaleDenominator() const;

    void setMapUnits( QGis::UnitType u );
    QGis::UnitType mapUnits() const;

    void setSymbologyExport( QgsDxfExport::SymbologyExport e );
    QgsDxfExport::SymbologyExport symbologyExport() const;

    void setExtent( const QgsRectangle& r );
    QgsRectangle extent() const;

    //get closest entry in dxf palette
    static int closestColorMatch( QRgb pixel );

    //! @note available in python bindings as writeGroupInt
    void writeGroup( int code, int i ) /PyName=writeGroupInt/;
    //! @note available in python bindings as writeGroupDouble
    void writeGroup( int code, double d ) /PyName=writeGroupDouble/;
    void writeGroup( int code, const QString& s );
    void writeGroupCode( int code );
    void writeInt( int i );
    void writeDouble( double d );
    void writeString( const QString& s );

    //draw dxf primitives
    void writePolyline( const QgsPolyline& line, const QString& layer, const QString& lineStyleName, int color,
                        double width = -1, bool polygon = false );

    void writeSolid( const QString& layer, int color, const QgsPoint& pt1, const QgsPoint& pt2, const QgsPoint& pt3, const QgsPoint& pt4 );

    //write line (as a polyline)
    void writeLine( const QgsPoint& pt1, const QgsPoint& pt2, const QString& layer, const QString& lineStyleName, int color, double width = -1 );

    void writePoint( const QString& layer, int color, const QgsPoint& pt );

    void writeCircle( const QString& layer, int color, const QgsPoint& pt, double radius );

    void writeText( const QString& layer, const QString& text, const QgsPoint& pt, double size, double angle, int color );

    static double mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits );

    static QString dxfLayerName( const QString& name );

};
