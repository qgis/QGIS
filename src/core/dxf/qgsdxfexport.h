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

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsgeometry.h"
#include "qgssymbol.h" // for OutputUnit enum
#include "qgsmapsettings.h"

#include <QColor>
#include <QList>
#include <QTextStream>

class QgsMapLayer;
class QgsPointXY;
class QgsSymbolLayer;
class QIODevice;
class QgsPalLayerSettings;

#define DXF_HANDSEED 100
#define DXF_HANDMAX 9999999
#define DXF_HANDPLOTSTYLE 0xf

namespace pal SIP_SKIP
{
  class LabelPosition;
}

/**
 * \ingroup core
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

    /**
     * Constructor for QgsDxfExport.
     */
    QgsDxfExport() = default;
    QgsDxfExport( const QgsDxfExport &dxfExport ) SIP_SKIP;
    QgsDxfExport &operator=( const QgsDxfExport &dxfExport );

    /**
     * Set map settings and assign layer name attributes
     * \param settings map settings to apply
     */
    void setMapSettings( const QgsMapSettings &settings );

    /**
     * Add layers to export
     * \param layers list of layers and corresponding attribute indexes that determine the layer name (-1 for original layer name or title)
     * \see setLayerTitleAsName
     */
    void addLayers( const QList< QPair<QgsVectorLayer *, int > > &layers );

    /**
     * Export to a dxf file in the given encoding
     * \param d device
     * \param codec encoding
     * \returns 0 on success, 1 on invalid device, 2 when devices is not writable
     */
    int writeToFile( QIODevice *d, const QString &codec );  //maybe add progress dialog? other parameters (e.g. scale, dpi)?

    /**
     * Set reference \a scale for output.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \since QGIS 3.0
     * \see symbologyScale()
     */
    void setSymbologyScale( double scale ) { mSymbologyScale = scale; }

    /**
     * Returns the reference scale for output.
     * The  scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \since QGIS 3.0
     * \see setSymbologyScale()
     */
    double symbologyScale() const { return mSymbologyScale; }

    /**
     * Retrieve map units
     * \returns unit
     */
    QgsUnitTypes::DistanceUnit mapUnits() const;

    /**
     * Set destination CRS
     * \see destinationCrs()
     * \since QGIS 3.0
     */
    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the destination CRS, or an invalid CRS if no reprojection will be done.
     * \see setDestinationCrs()
     * \since QGIS 3.0
     */
    QgsCoordinateReferenceSystem destinationCrs() const;

    /**
     * Set symbology export mode
     * \param e the mode
     */
    void setSymbologyExport( QgsDxfExport::SymbologyExport e ) { mSymbologyExport = e; }

    /**
     * Get symbology export mode
     * \returns mode
     * \see setSymbologyExport
     */
    QgsDxfExport::SymbologyExport symbologyExport() const { return mSymbologyExport; }

    /**
     * Set extent of area to export
     * \param r area to export
     */
    void setExtent( const QgsRectangle &r ) { mExtent = r; }

    /**
     * Get extent of area to export
     * \returns area to export
     * \see setExtent
     */
    QgsRectangle extent() const { return mExtent; }

    /**
     * Enable use of title (where set) instead of layer name,
     * when attribute index of corresponding layer index is -1
     * \param layerTitleAsName flag
     * \see addLayers
     */
    void setLayerTitleAsName( bool layerTitleAsName ) { mLayerTitleAsName = layerTitleAsName; }

    /**
     * Retrieve whether layer title (where set) instead of name shall be use
     * \returns flag
     * \see setLayerTitleAsName
     */
    bool layerTitleAsName() { return mLayerTitleAsName; }

    /**
     * Force 2d output (eg. to support linewidth in polylines)
     * \param force2d flag
     * \see force2d
     */
    void setForce2d( bool force2d ) { mForce2d = force2d; }

    /**
     * Retrieve whether the output should be forced to 2d
     * \returns flag
     * \see setForce2d
     */
    bool force2d() { return mForce2d; }

    /**
     * Get DXF palette index of nearest entry for given color
     * \param color
     */
    static int closestColorMatch( QRgb color );

    /**
     * Get layer name for feature
     * \param id layer id of layer
     * \param f feature of layer
     * \returns layer name for feature
     */
    QString layerName( const QString &id, const QgsFeature &f ) const;

    /**
     * Get name for layer respecting the use layer title as layer name mode
     * \param vl the vector layer
     * \returns name of layer
     * \see setLayerTitleAsName
     */
    QString layerName( QgsVectorLayer *vl ) const;

    /**
     * Write a tuple of group code and integer value
     * \param code group code
     * \param i integer value
     * \note available in Python bindings as writeGroupInt
     */
    void writeGroup( int code, int i ) SIP_PYNAME( writeGroupInt );

    /**
     * Write a group code with a floating point value
     * \param code group code
     * \param d floating point value
     * \note available in Python bindings as writeGroupDouble
     */
    void writeGroup( int code, double d ) SIP_PYNAME( writeGroupDouble );

    /**
     * Write a group code with a string value
     * \param code group code
     * \param s string value
     */
    void writeGroup( int code, const QString &s );

    /**
     * Write a group code with a point
     * \param code group code
     * \param p point value
     * \note available in Python bindings as writeGroupPointV2
     * \since QGIS 2.15
     */
    void writeGroup( int code, const QgsPoint &p ) SIP_PYNAME( writeGroupPointV2 );

    /**
     * Write a group code with color value
     * \param color color
     * \param exactMatch group code to use if the color has an exact match in the dxf palette
     * \param rgbCode group code to use if the color doesn't have an exact match or has a transparency component
     * \param transparencyCode group code to use for transparency component
     * \note available in Python bindings as writeGroupPoint
     */
    void writeGroup( const QColor &color, int exactMatch = 62, int rgbCode = 420, int transparencyCode = 440 );

    /**
     * Write a group code
     * \param code group code value
     */
    void writeGroupCode( int code );

    /**
     * Write an integer value
     * \param i integer value
     */
    void writeInt( int i );

    /**
     * Write a floating point value
     * \param d floating point value
     */
    void writeDouble( double d );

    /**
     * Write a string value
     * \param s string value
     */
    void writeString( const QString &s );

    /**
     * Write a tuple of group code and a handle
     * \param code group code to use
     * \param handle handle to use (0 generates a new handle)
     * \returns the used handle
     */
    int writeHandle( int code = 5, int handle = 0 );

    /**
     * Draw dxf primitives (LWPOLYLINE)
     * \param line polyline
     * \param layer layer name to use
     * \param lineStyleName line type to use
     * \param color color to use
     * \param width line width to use
     * \note not available in Python bindings
     * \since QGIS 2.15
     */
    void writePolyline( const QgsPointSequence &line, const QString &layer, const QString &lineStyleName, const QColor &color, double width = -1 ) SIP_SKIP;

    /**
     * Draw dxf filled polygon (HATCH)
     * \param polygon polygon
     * \param layer layer name to use
     * \param hatchPattern hatchPattern to use
     * \param color color to use
     * \note not available in Python bindings
     * \since QGIS 2.15
     */
    void writePolygon( const QgsRingSequence &polygon, const QString &layer, const QString &hatchPattern, const QColor &color ) SIP_SKIP;

    /**
     * Write line (as a polyline)
     * \since QGIS 2.15
     */
    void writeLine( const QgsPoint &pt1, const QgsPoint &pt2, const QString &layer, const QString &lineStyleName, const QColor &color, double width = -1 );

    /**
     * Write point
     * \note available in Python bindings as writePointV2
     * \since QGIS 2.15
     */
    void writePoint( const QString &layer, const QColor &color, const QgsPoint &pt ) SIP_PYNAME( writePointV2 );

    /**
     * Write filled circle (as hatch)
     * \note available in Python bindings as writePointV2
     * \since QGIS 2.15
     */
    void writeFilledCircle( const QString &layer, const QColor &color, const QgsPoint &pt, double radius ) SIP_PYNAME( writeFillCircleV2 );

    /**
     * Write circle (as polyline)
     * \note available in Python bindings as writeCircleV2
     * \since QGIS 2.15
     */
    void writeCircle( const QString &layer, const QColor &color, const QgsPoint &pt, double radius, const QString &lineStyleName, double width ) SIP_PYNAME( writeCircleV2 );

    /**
     * Write text (TEXT)
     * \note available in Python bindings as writeTextV2
     * \since QGIS 2.15
     */
    void writeText( const QString &layer, const QString &text, const QgsPoint &pt, double size, double angle, const QColor &color ) SIP_PYNAME( writeTextV2 );

    /**
     * Write mtext (MTEXT)
     * \note available in Python bindings as writeMTextV2
     * \since QGIS 2.15
     */
    void writeMText( const QString &layer, const QString &text, const QgsPoint &pt, double width, double angle, const QColor &color );

    /**
     * Calculates a scaling factor to convert from map units to a specified symbol unit.
     * The \a scale parameter indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     */
    static double mapUnitScaleFactor( double scale, QgsUnitTypes::RenderUnit symbolUnits, QgsUnitTypes::DistanceUnit mapUnits );

    //! Return cleaned layer name for use in DXF
    static QString dxfLayerName( const QString &name );

    //! return DXF encoding for Qt encoding
    static QString dxfEncoding( const QString &name );

    //! return list of available DXF encodings
    static QStringList encodings();

    /**
     * Output the label
     * \param layerId id of the layer
     * \param context render context
     * \param label position of label
     * \param settings label settings
     * \note not available in Python bindings
     */
    void drawLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings ) SIP_SKIP;

    /**
     * Register name of layer for feature
     * \param layerId id of layer
     * \param fid id of feature
     * \param layer dxf layer of feature
     */
    void registerDxfLayer( const QString &layerId, QgsFeatureId fid, const QString &layer );

  private:
    //! Extent for export, only intersecting features are exported. If the extent is an empty rectangle, all features are exported
    QgsRectangle mExtent;
    //! Scale for symbology export (used if symbols units are mm)
    double mSymbologyScale = 1.0;
    SymbologyExport mSymbologyExport = NoSymbology;
    QgsUnitTypes::DistanceUnit mMapUnits = QgsUnitTypes::DistanceMeters;
    bool mLayerTitleAsName = false;

    QTextStream mTextStream;

    static int sDxfColors[][3];
    static const char *DXF_ENCODINGS[][2];

    int mSymbolLayerCounter = 0; //internal counter
    int mNextHandleId = DXF_HANDSEED;
    int mBlockCounter = 0;

    QHash< const QgsSymbolLayer *, QString > mLineStyles; //symbol layer name types
    QHash< const QgsSymbolLayer *, QString > mPointSymbolBlocks; //reference to point symbol blocks

    //AC1009
    void writeHeader( const QString &codepage );
    void writeTables();
    void writeBlocks();
    void writeEntities();
    void writeEntitiesSymbolLevels( QgsVectorLayer *layer );
    void writeEndFile();

    void startSection();
    void endSection();

    void writePoint( const QgsPoint &pt, const QString &layer, const QColor &color, QgsSymbolRenderContext &ctx, const QgsSymbolLayer *symbolLayer, const QgsSymbol *symbol, double angle );
    void writeDefaultLinetypes();
    void writeSymbolLayerLinetype( const QgsSymbolLayer *symbolLayer );
    void writeLinetype( const QString &styleName, const QVector<qreal> &pattern, QgsUnitTypes::RenderUnit u );

    void addFeature( QgsSymbolRenderContext &ctx, const QgsCoordinateTransform &ct, const QString &layer, const QgsSymbolLayer *symbolLayer, const QgsSymbol *symbol );

    //returns dxf palette index from symbol layer color
    static QColor colorFromSymbolLayer( const QgsSymbolLayer *symbolLayer, QgsSymbolRenderContext &ctx );
    QString lineStyleFromSymbolLayer( const QgsSymbolLayer *symbolLayer );

    //functions for dxf palette
    static int color_distance( QRgb p1, int index );
    static QRgb createRgbEntry( qreal r, qreal g, qreal b );

    //helper functions for symbology export
    QgsRenderContext renderContext() const;

    QList< QPair< QgsSymbolLayer *, QgsSymbol * > > symbolLayers( QgsRenderContext &context );
    static int nLineTypes( const QList< QPair< QgsSymbolLayer *, QgsSymbol *> > &symbolLayers );
    static bool hasDataDefinedProperties( const QgsSymbolLayer *sl, const QgsSymbol *symbol );
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
    QgsCoordinateReferenceSystem mCrs;
    QgsMapSettings mMapSettings;
    QHash<QString, int> mLayerNameAttribute;
    double mFactor = 1.0;
    bool mForce2d = false;
};

#endif // QGSDXFEXPORT_H
