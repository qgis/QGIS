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
class QgsPalLayerSettings;

namespace pal
{
  class LabelPosition;
};

/** \ingroup core
 * \class QgsDxfExport
 */
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

    /**
     * Add layers to export
     * @param layers list of layers and corresponding attribute indexes that determine the layer name (-1 for original layer name or title)
     * @see setLayerTitleAsName
     */
    void addLayers( const QList< QPair<QgsVectorLayer *, int > > &layers );

    /**
     * Export to a dxf file in the given encoding
     * @param d device
     * @param codec encoding
     * @returns 0 on success, 1 on invalid device, 2 when devices is not writable
     */
    int writeToFile( QIODevice *d, const QString& codec );  //maybe add progress dialog? other parameters (e.g. scale, dpi)?

    /**
     * Set reference scale for output
     * @param d scale denominator
     */
    void setSymbologyScaleDenominator( double d ) { mSymbologyScaleDenominator = d; }

    /**
     * Retrieve reference scale for output
     * @returns reference scale
     * @see setSymbologyScaleDenominator
     */
    double symbologyScaleDenominator() const { return mSymbologyScaleDenominator; }

    /**
     * Set map units
     * @param u unit
     */
    void setMapUnits( QGis::UnitType u ) { mMapUnits = u; }

    /**
     * Retrieve map units
     * @returns unit
     * @see setMapUnits
     */
    QGis::UnitType mapUnits() const { return mMapUnits; }

    /**
     * Set symbology export mode
     * @param e the mode
     */
    void setSymbologyExport( SymbologyExport e ) { mSymbologyExport = e; }

    /**
     * Get symbology export mode
     * @returns mode
     * @see setSymbologyExport
     */
    SymbologyExport symbologyExport() const { return mSymbologyExport; }

    /**
     * Set extent of area to export
     * @param r area to export
     */
    void setExtent( const QgsRectangle &r ) { mExtent = r; }

    /**
     * Get extent of area to export
     * @returns area to export
     * @see setExtent
     */
    QgsRectangle extent() const { return mExtent; }

    /**
     * Enable use of title (where set) instead of layer name,
     * when attribute index of corresponding layer index is -1
     * @param layerTitleAsName flag
     * @see addLayers
     */
    void setLayerTitleAsName( bool layerTitleAsName ) { mLayerTitleAsName = layerTitleAsName; }

    /**
     * Retrieve wether layer title (where set) instead of name shall be use
     * @returns flag
     * @see setLayerTitleAsName
     */
    bool layerTitleAsName() { return mLayerTitleAsName; }

    /**
     * Get DXF palette index of nearest entry for given color
     * @param color
     */
    static int closestColorMatch( QRgb color );

    /**
     * Get layer name for feature
     * @param id layer id of layer
     * @param f feature of layer
     * @returns layer name for feature
     */
    QString layerName( const QString &id, const QgsFeature &f ) const;

    /**
     * Get name for layer respecting the use layer title as layer name mode
     * @param vl the vector layer
     * @returns name of layer
     * @see setLayerTitleAsName
     */
    QString layerName( QgsVectorLayer *vl ) const;

    /**
     * Write a tuple of group code and integer value
     * @param code group code
     * @param i integer value
     * @note available in python bindings as writeGroupInt
     */
    void writeGroup( int code, int i );

    /**
     * Write a group code with a floating point value
     * @param code group code
     * @param d floating point value
     * @note available in python bindings as writeGroupDouble
     */
    void writeGroup( int code, double d );

    /**
     * Write a group code with a string value
     * @param code group code
     * @param s string value
     */
    void writeGroup( int code, const QString &s );

    /**
     * Write a group code with a point
     * @param code group code
     * @param p point value
     * @param z z value of the point (defaults to 0.0)
     * @param skipz write point in 2d (defaults to false)
     * @note available in python bindings as writeGroupPoint
     * @deprecated use QgsPointV2 version instead
     */
    Q_DECL_DEPRECATED void writeGroup( int code, const QgsPoint &p, double z = 0.0, bool skipz = false );

    /**
     * Write a group code with a point
     * @param code group code
     * @param p point value
     * @note available in python bindings as writeGroupPointV2
     * @note added in 2.15
     */
    void writeGroup( int code, const QgsPointV2 &p );

    /**
     * Write a group code with color value
     * @param color color
     * @param exactMatch group code to use if the color has an exact match in the dxf palette
     * @param rgbCode group code to use if the color doesn't have an exact match or has a transparency component
     * @param transparencyCode group code to use for transparency component
     * @note available in python bindings as writeGroupPoint
     */
    void writeGroup( const QColor& color, int exactMatch = 62, int rgbCode = 420, int transparencyCode = 440 );

    /**
     * Write a group code
     * @param code group code value
     */
    void writeGroupCode( int code );

    /**
     * Write an integer value
     * @param i integer value
     */
    void writeInt( int i );

    /**
     * Write a floating point value
     * @param d floating point value
     */
    void writeDouble( double d );

    /**
     * Write a string value
     * @param s string value
     */
    void writeString( const QString &s );

    /**
     * Write a tuple of group code and a handle
     * @param code group code to use
     * @param handle handle to use (0 generates a new handle)
     * @returns the used handle
     */
    int writeHandle( int code = 5, int handle = 0 );

    /**
     * Draw dxf primitives (LWPOLYLINE)
     * @param line polyline
     * @param layer layer name to use
     * @param lineStyleName line type to use
     * @param color color to use
     * @param width line width to use
     * @deprecated use QgsPointSequenceV2 variant
     */
    Q_DECL_DEPRECATED void writePolyline( const QgsPolyline &line, const QString &layer, const QString &lineStyleName, const QColor& color, double width = -1 );

    /**
     * Draw dxf primitives (LWPOLYLINE)
     * @param line polyline
     * @param layer layer name to use
     * @param lineStyleName line type to use
     * @param color color to use
     * @param width line width to use
     * @note not available in Python bindings
     * @note added in 2.15
     */
    void writePolyline( const QgsPointSequenceV2 &line, const QString &layer, const QString &lineStyleName, const QColor& color, double width = -1 );

    /**
     * Draw dxf filled polygon (HATCH)
     * @param polygon polygon
     * @param layer layer name to use
     * @param hatchPattern hatchPattern to use
     * @param color color to use
     * @deprecated use version with QgsRingSequenceV2
     */
    Q_DECL_DEPRECATED void writePolygon( const QgsPolygon &polygon, const QString &layer, const QString &hatchPattern, const QColor& color );

    /**
     * Draw dxf filled polygon (HATCH)
     * @param polygon polygon
     * @param layer layer name to use
     * @param hatchPattern hatchPattern to use
     * @param color color to use
     * @note not available in Python bindings
     * @note added in 2.15
     */
    void writePolygon( const QgsRingSequenceV2 &polygon, const QString &layer, const QString &hatchPattern, const QColor& color );

    /**
     * Draw dxf filled polygon (SOLID)
     * @param layer layer name to use
     * @param color color to use
     * @param pt1 1. point of solid
     * @param pt2 2. point of solid
     * @param pt3 3. point of solid
     * @param pt4 4. point of solid
     * @deprecated see writePolygon
     */
    Q_DECL_DEPRECATED void writeSolid( const QString &layer, const QColor& color, const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, const QgsPoint &pt4 );

    //! Write line (as a polyline)
    //! @deprecated use QgsPointV2 version
    Q_DECL_DEPRECATED void writeLine( const QgsPoint &pt1, const QgsPoint &pt2, const QString &layer, const QString &lineStyleName, const QColor& color, double width = -1 );

    //! Write line (as a polyline)
    //! @note added in 2.15
    void writeLine( const QgsPointV2 &pt1, const QgsPointV2 &pt2, const QString &layer, const QString &lineStyleName, const QColor& color, double width = -1 );

    //! Write point
    //! @deprecated use QgsPointV2 version
    Q_DECL_DEPRECATED void writePoint( const QString &layer, const QColor& color, const QgsPoint &pt );

    //! Write point
    //! @note available in Python bindings as writePointV2
    //! @note added in 2.15
    void writePoint( const QString &layer, const QColor& color, const QgsPointV2 &pt );

    //! Write filled circle (as hatch)
    //! @deprecated use QgsPointV2 version
    Q_DECL_DEPRECATED void writeFilledCircle( const QString &layer, const QColor& color, const QgsPoint &pt, double radius );

    //! Write filled circle (as hatch)
    //! @note available in Python bindings as writePointV2
    //! @note added in 2.15
    void writeFilledCircle( const QString &layer, const QColor& color, const QgsPointV2 &pt, double radius );

    //! Write circle (as polyline)
    //! @deprecated use QgsPointV2 version
    Q_DECL_DEPRECATED void writeCircle( const QString &layer, const QColor& color, const QgsPoint &pt, double radius, const QString &lineStyleName, double width );

    //! Write circle (as polyline)
    //! @note available in Python bindings as writeCircleV2
    //! @note added in 2.15
    void writeCircle( const QString &layer, const QColor& color, const QgsPointV2 &pt, double radius, const QString &lineStyleName, double width );

    //! Write text (TEXT)
    //! @deprecated use QgsPointV2 version
    Q_DECL_DEPRECATED void writeText( const QString &layer, const QString &text, const QgsPoint &pt, double size, double angle, const QColor& color );

    //! Write text (TEXT)
    //! @note available in Python bindings as writeTextV2
    //! @note added in 2.15
    void writeText( const QString &layer, const QString &text, const QgsPointV2 &pt, double size, double angle, const QColor& color );

    //! Write mtext (MTEXT)
    //! @deprecated use QgsPointV2 version
    Q_DECL_DEPRECATED void writeMText( const QString &layer, const QString &text, const QgsPoint &pt, double width, double angle, const QColor& color );

    //! Write mtext (MTEXT)
    //! @note available in Python bindings as writeMTextV2
    //! @note added in 2.15
    void writeMText( const QString &layer, const QString &text, const QgsPointV2 &pt, double width, double angle, const QColor& color );

    static double mapUnitScaleFactor( double scaleDenominator, QgsSymbolV2::OutputUnit symbolUnits, QGis::UnitType mapUnits );

    //! Return cleaned layer name for use in DXF
    static QString dxfLayerName( const QString &name );

    //! return DXF encoding for Qt encoding
    static QString dxfEncoding( const QString &name );

    //! return list of available DXF encodings
    static QStringList encodings();

    /** Output the label
     * @param layerId id of the layer
     * @param context render context
     * @param label position of label
     * @param settings label settings
     * @note not available in Python bindings
     */
    void drawLabel( QString layerId, QgsRenderContext& context, pal::LabelPosition* label, const QgsPalLayerSettings &settings );

    /** Register name of layer for feature
     * @param layerId id of layer
     * @param fid id of feature
     * @param layer dxf layer of feature
     */
    void registerDxfLayer( QString layerId, QgsFeatureId fid, QString layer );

  private:
    QList< QPair<QgsVectorLayer*, int> > mLayers;

    /** Extent for export, only intersecting features are exported. If the extent is an empty rectangle, all features are exported*/
    QgsRectangle mExtent;
    /** Scale for symbology export (used if symbols units are mm)*/
    double mSymbologyScaleDenominator;
    SymbologyExport mSymbologyExport;
    QGis::UnitType mMapUnits;
    bool mLayerTitleAsName;

    QTextStream mTextStream;

    static int mDxfColors[][3];
    static const char *mDxfEncodings[][2];

    int mSymbolLayerCounter; //internal counter
    int mNextHandleId;
    int mBlockCounter;

    QHash< const QgsSymbolLayerV2*, QString > mLineStyles; //symbol layer name types
    QHash< const QgsSymbolLayerV2*, QString > mPointSymbolBlocks; //reference to point symbol blocks

    //AC1009
    void writeHeader( const QString& codepage );
    void writeTables();
    void writeBlocks();
    void writeEntities();
    void writeEntitiesSymbolLevels( QgsVectorLayer *layer );
    void writeEndFile();

    void startSection();
    void endSection();

    void writePoint( const QgsPointV2 &pt, const QString &layer, const QColor& color, QgsSymbolV2RenderContext &ctx, const QgsSymbolLayerV2 *symbolLayer, const QgsSymbolV2 *symbol, double angle );
    void writeDefaultLinetypes();
    void writeSymbolLayerLinetype( const QgsSymbolLayerV2 *symbolLayer );
    void writeLinetype( const QString &styleName, const QVector<qreal> &pattern, QgsSymbolV2::OutputUnit u );

    QgsRectangle dxfExtent() const;

    void addFeature( QgsSymbolV2RenderContext &ctx, const QString &layer, const QgsSymbolLayerV2 *symbolLayer, const QgsSymbolV2 *symbol );

    //returns dxf palette index from symbol layer color
    static QColor colorFromSymbolLayer( const QgsSymbolLayerV2 *symbolLayer, QgsSymbolV2RenderContext &ctx );
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

    //! DXF layer name for each label feature
    QMap< QString, QMap<QgsFeatureId, QString> > mDxfLayerNames;
};

#endif // QGSDXFEXPORT_H
