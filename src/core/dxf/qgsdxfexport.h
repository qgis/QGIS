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
#include <QTextStream>

class QgsMapLayer;
class QgsPoint;
class QgsSymbolLayerV2;
class QIODevice;

class CORE_EXPORT QgsDxfExport
{
  public:
    enum SymbologyExport
    {
      NoSymbology = 0, //export only data
      FeatureSymbology, //Keeps the number of features and export symbology per feature (using the first symbol level)
      SymbolLayerSymbology //Exports one feature per symbol layer (considering symbol levels)
    };

    QgsDxfExport();
    QgsDxfExport( const QgsDxfExport &dxfExport );
    ~QgsDxfExport();
    QgsDxfExport &operator=( const QgsDxfExport &dxfExport );

    void addLayers( const QList< QPair<QgsVectorLayer *, int > > &layers );
    int writeToFile( QIODevice *d, QString codec );  //maybe add progress dialog? other parameters (e.g. scale, dpi)?

    void setSymbologyScaleDenominator( double d ) { mSymbologyScaleDenominator = d; }
    double symbologyScaleDenominator() const { return mSymbologyScaleDenominator; }

    void setMapUnits( QGis::UnitType u ) { mMapUnits = u; }
    QGis::UnitType mapUnits() const { return mMapUnits; }

    void setSymbologyExport( SymbologyExport e ) { mSymbologyExport = e; }
    SymbologyExport symbologyExport() const { return mSymbologyExport; }

    void setExtent( const QgsRectangle &r ) { mExtent = r; }
    QgsRectangle extent() const { return mExtent; }

    //get closest entry in dxf palette
    static int closestColorMatch( QRgb pixel );

    QString layerName( const QString &id, const QgsFeature &f ) const;

    //! @note available in python bindings as writeGroupInt
    void writeGroup( int code, int i );
    //! @note available in python bindings as writeGroupDouble
    void writeGroup( int code, double d );
    void writeGroup( int code, const QString &s );
    void writeGroupCode( int code );
    void writeInt( int i );
    void writeDouble( double d );
    void writeString( const QString &s );
    void writeGroup( int code, const QgsPoint &p, double z = 0.0, bool skipz = false );
    void writeGroup( QColor color, int exactMatch = 62, int rgbCode = 420, int transparencyCode = 440 );

    int writeHandle( int code = 5, int handle = 0 );

    //! Draw dxf primitives (LWPOLYLINE)
    void writePolyline( const QgsPolyline &line, const QString &layer, const QString &lineStyleName, QColor color, double width = -1 );

    //! Draw dxf polygon (HATCH)
    void writePolygon( const QgsPolygon &polygon, const QString &layer, const QString &hatchPattern, QColor color );

    /** Draw solid
     * @deprecated see writePolygon
     */
    Q_DECL_DEPRECATED void writeSolid( const QString &layer, QColor color, const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, const QgsPoint &pt4 );

    //! Write line (as a polyline)
    void writeLine( const QgsPoint &pt1, const QgsPoint &pt2, const QString &layer, const QString &lineStyleName, QColor color, double width = -1 );

    //! Write point
    void writePoint( const QString &layer, QColor color, const QgsPoint &pt );

    //! Write filled circle (as hatch)
    void writeFilledCircle( const QString &layer, QColor color, const QgsPoint &pt, double radius );

    //! Write circle (as polyline)
    void writeCircle( const QString &layer, QColor color, const QgsPoint &pt, double radius, const QString &lineStyleName, double width );

    //! Write text (TEXT)
    void writeText( const QString &layer, const QString &text, const QgsPoint &pt, double size, double angle, QColor color );

    //! Write mtext (MTEXT)
    void writeMText( const QString &layer, const QString &text, const QgsPoint &pt, double width, double angle, QColor color );

    static double mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits );

    //! Return cleaned layer name for use in DXF
    static QString dxfLayerName( const QString &name );

    //! return DXF encoding for Qt encoding
    static QString dxfEncoding( const QString &name );

    //! return list of available DXF encodings
    static QStringList encodings();

  private:
    QList< QPair<QgsVectorLayer*, int> > mLayers;

    /** Extent for export, only intersecting features are exported. If the extent is an empty rectangle, all features are exported*/
    QgsRectangle mExtent;
    /** Scale for symbology export (used if symbols units are mm)*/
    double mSymbologyScaleDenominator;
    SymbologyExport mSymbologyExport;
    QGis::UnitType mMapUnits;

    QTextStream mTextStream;

    static int mDxfColors[][3];
    static const char *mDxfEncodings[][2];

    int mSymbolLayerCounter; //internal counter
    int mNextHandleId;
    int mBlockCounter;

    QHash< const QgsSymbolLayerV2*, QString > mLineStyles; //symbol layer name types
    QHash< const QgsSymbolLayerV2*, QString > mPointSymbolBlocks; //reference to point symbol blocks

    //AC1009
    void writeHeader( QString codepage );
    void writeTables();
    void writeBlocks();
    void writeEntities();
    void writeEntitiesSymbolLevels( QgsVectorLayer *layer );
    void writeEndFile();

    void startSection();
    void endSection();

    void writePoint( const QgsPoint &pt, const QString &layer, QColor color, const QgsFeature *f, const QgsSymbolLayerV2 *symbolLayer, const QgsSymbolV2 *symbol );
    void writeVertex( const QgsPoint &pt, const QString &layer );
    void writeDefaultLinetypes();
    void writeSymbolLayerLinetype( const QgsSymbolLayerV2 *symbolLayer );
    void writeLinetype( const QString &styleName, const QVector<qreal> &pattern, QgsSymbolV2::OutputUnit u );

    QgsRectangle dxfExtent() const;

    void addFeature( const QgsSymbolV2RenderContext &ctx, const QString &layer, const QgsSymbolLayerV2 *symbolLayer, const QgsSymbolV2 *symbol );

    //returns dxf palette index from symbol layer color
    static QColor colorFromSymbolLayer( const QgsSymbolLayerV2 *symbolLayer, const QgsSymbolV2RenderContext &ctx );
    QString lineStyleFromSymbolLayer( const QgsSymbolLayerV2 *symbolLayer );

    //functions for dxf palette
    static int color_distance( QRgb p1, int index );
    static QRgb createRgbEntry( qreal r, qreal g, qreal b );

    //helper functions for symbology export
    QgsRenderContext renderContext() const;

    QList< QPair< QgsSymbolLayerV2 *, QgsSymbolV2 * > > symbolLayers( QgsRenderContext& context );
    static int nLineTypes( const QList< QPair< QgsSymbolLayerV2*, QgsSymbolV2*> > &symbolLayers );
    static bool hasDataDefinedProperties( const QgsSymbolLayerV2 *sl, const QgsSymbolV2 *symbol );
    double dashSize() const;
    double dotSize() const;
    double dashSeparatorSize() const;
    double sizeToMapUnits( double s ) const;
    static QString lineNameFromPenStyle( Qt::PenStyle style );
    bool layerIsScaleBasedVisible( const QgsMapLayer *layer ) const;

    QHash<QString, int> mBlockHandles;
    QString mBlockHandle;
};

#endif // QGSDXFEXPORT_H
