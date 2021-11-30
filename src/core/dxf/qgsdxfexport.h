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
#include "qgsmapsettings.h"
#include "qgslabelsink.h"
#include "qgsrendercontext.h"

#include <QColor>
#include <QList>
#include <QTextStream>

class QgsMapLayer;
class QgsPointXY;
class QgsSymbolLayer;
class QIODevice;
class QgsPalLayerSettings;
class QgsCurve;
class QgsCurvePolygon;
class QgsCircularString;
class QgsCompoundCurve;
struct DxfLayerJob;
class QgsSymbolRenderContext;

#define DXF_HANDSEED 100
#define DXF_HANDMAX 9999999
#define DXF_HANDPLOTSTYLE 0xf

namespace pal // SIP_SKIP
{
  class LabelPosition;
}


/**
 * \ingroup core
 * \class QgsDxfExport
 * \brief Exports QGIS layers to the DXF format.
 */
#ifdef SIP_RUN
class CORE_EXPORT QgsDxfExport
{
#else
class CORE_EXPORT QgsDxfExport : public QgsLabelSink
{
#endif
  public:

    /**
     * Layers and optional attribute index to split
     * into multiple layers using attribute value as layer name.
     */
    struct CORE_EXPORT DxfLayer
    {
        DxfLayer( QgsVectorLayer *vl, int layerOutputAttributeIndex = -1 )
          : mLayer( vl )
          , mLayerOutputAttributeIndex( layerOutputAttributeIndex )
        {}

        //! Returns the layer
        QgsVectorLayer *layer() const {return mLayer;}

        /**
         * Returns the attribute index used to split into multiple layers.
         * The attribute value is used for layer names.
         * \see splitLayerAttribute
         */
        int layerOutputAttributeIndex() const {return mLayerOutputAttributeIndex;}

        /**
         * If the split layer attribute is set, the vector layer
         * will be split into several dxf layers, one per each
         * unique value.
         * \since QGIS 3.12
         */
        QString splitLayerAttribute() const;

      private:
        QgsVectorLayer *mLayer = nullptr;
        int mLayerOutputAttributeIndex = -1;
    };

    enum SymbologyExport
    {
      NoSymbology = 0, //!< Export only data
      FeatureSymbology, //!< Keeps the number of features and export symbology per feature (using the first symbol level)
      SymbolLayerSymbology //!< Exports one feature per symbol layer (considering symbol levels)
    };

    //! Export flags
    enum Flag
    {
      FlagNoMText = 1 << 1, //!< Export text as TEXT elements. If not set, text will be exported as MTEXT elements.
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * The result of an export as dxf operation
     *
     * \since QGIS 3.10.1
     */
    enum class ExportResult
    {
      Success = 0, //!< Successful export
      InvalidDeviceError, //!< Invalid device error
      DeviceNotWritableError, //!< Device not writable error
      EmptyExtentError //!< Empty extent, no extent given and no extent could be derived from layers
    };

    /**
     * Vertical alignments.
     */
    enum class VAlign : int
    {
      VBaseLine = 0,    //!< Top (0)
      VBottom = 1,      //!< Bottom (1)
      VMiddle = 2,      //!< Middle (2)
      VTop = 3,         //!< Top (3)
      Undefined = 9999  //!< Undefined
    };

    //! Horizontal alignments.
    enum class HAlign : int
    {
      HLeft = 0,       //!< Left (0)
      HCenter = 1,     //!< Centered (1)
      HRight = 2,      //!< Right (2)
      HAligned = 3,    //!< Aligned = (3) (if VAlign==0)
      HMiddle = 4,     //!< Middle = (4) (if VAlign==0)
      HFit = 5,        //!< Fit into point = (5) (if VAlign==0)
      Undefined = 9999 //!< Undefined
    };

    /**
     * Flags for polylines
     *
     * \since QGIS 3.12
     */
    enum DxfPolylineFlag
    {
      Closed = 1, //!< This is a closed polyline (or a polygon mesh closed in the M direction)
      Curve = 2, //!< Curve-fit vertices have been added
      Spline = 4, //! < Spline-fit vertices have been added
      Is3DPolyline = 8, //!< This is a 3D polyline
      Is3DPolygonMesh = 16, //!< This is a 3D polygon mesh
      PolygonMesh = 32, //!< The polygon mesh is closed in the N direction
      PolyfaceMesh = 64, //!< The polyline is a polyface mesh
      ContinuousPattern = 128, //!< The linetype pattern is generated continuously around the vertices of this polyline
    };

    Q_DECLARE_FLAGS( DxfPolylineFlags, DxfPolylineFlag )

    /**
     * Constructor for QgsDxfExport.
     */
    QgsDxfExport();

    ~QgsDxfExport() override;

    /**
     * Set map settings and assign layer name attributes
     * \param settings map settings to apply
     */
    void setMapSettings( const QgsMapSettings &settings );

    /**
     * Sets the export flags.
     * \see flags()
     * \since QGIS 3.0
     */
    void setFlags( QgsDxfExport::Flags flags );

    /**
     * Returns the export flags.
     * \see setFlags()
     * \since QGIS 3.0
     */
    QgsDxfExport::Flags flags() const;

    /**
     * Add layers to export
     * \param layers list of layers and corresponding attribute indexes that determine the layer name (-1 for original layer name or title)
     * \see setLayerTitleAsName
     */
    void addLayers( const QList< QgsDxfExport::DxfLayer > &layers );

    /**
     * Export to a dxf file in the given encoding
     * \param d device
     * \param codec encoding
     * \returns an ExportResult
     */
    ExportResult writeToFile( QIODevice *d, const QString &codec );  //maybe add progress dialog? other parameters (e.g. scale, dpi)?

    /**
     * Set reference \a scale for output.
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see symbologyScale()
     * \since QGIS 3.0
     */
    void setSymbologyScale( double scale ) { mSymbologyScale = scale; }

    /**
     * Returns the reference scale for output.
     * The  scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see setSymbologyScale()
     * \since QGIS 3.0
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
     * Gets symbology export mode
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
     * Gets extent of area to export
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
     * Gets DXF palette index of nearest entry for given color
     * \param color
     */
    static int closestColorMatch( QRgb color );

    /**
     * Gets layer name for feature
     * \param id layer id of layer
     * \param f feature of layer
     * \returns layer name for feature
     */
    QString layerName( const QString &id, const QgsFeature &f ) const;

    /**
     * Gets name for layer respecting the use layer title as layer name mode
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
     * Write a tuple of group code and long value
     * \param code group code
     * \param i integer value
     * \note available in Python bindings as writeGroupLong
     */
    void writeGroup( int code, long long i ) SIP_PYNAME( writeGroupLong );

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
     * Draw dxf primitives (LWPOLYLINE)
     * \param curve polyline (including curved)
     * \param layer layer name to use
     * \param lineStyleName line type to use
     * \param color color to use
     * \param width line width to use
     * \note not available in Python bindings
     * \since QGIS 3.8
     */
    void writePolyline( const QgsCurve &curve, const QString &layer, const QString &lineStyleName, const QColor &color, double width = -1 ) SIP_SKIP;

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
     * Draw dxf curved filled polygon (HATCH)
     * \param polygon polygon (including curves)
     * \param layer layer name to use
     * \param hatchPattern hatchPattern to use
     * \param color color to use
     * \note not available in Python bindings
     * \since QGIS 3.8
     */
    void writePolygon( const QgsCurvePolygon &polygon, const QString &layer, const QString &hatchPattern, const QColor &color ) SIP_SKIP;

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
    void writeText( const QString &layer, const QString &text, const QgsPoint &pt, double size, double angle, const QColor &color, QgsDxfExport::HAlign hali = QgsDxfExport::HAlign::Undefined, QgsDxfExport::VAlign vali = QgsDxfExport::VAlign::Undefined ) SIP_PYNAME( writeTextV2 );

    /**
     * Write mtext (MTEXT)
     * \note available in Python bindings as writeMTextV2
     * \since QGIS 2.15
     */
    void writeMText( const QString &layer, const QString &text, const QgsPoint &pt, double width, double angle, const QColor &color );

    /**
     * Returns scale factor for conversion to map units
     * \param scale the map scale denominator
     * \param symbolUnits the symbol output units
     * \param mapUnits the map units
     * \param mapUnitsPerPixel Map units per pixel
    */
    static double mapUnitScaleFactor( double scale, QgsUnitTypes::RenderUnit symbolUnits, QgsUnitTypes::DistanceUnit mapUnits, double mapUnitsPerPixel = 1.0 );

    /**
     * Clips value to scale minimum/maximum
     * \param value the value to clip
     * \param scale the scale dependent minimum/maximum values
     * \param pixelToMMFactor pixels per mm
    */
    void clipValueToMapUnitScale( double &value, const QgsMapUnitScale &scale, double pixelToMMFactor ) const;

    //! Returns cleaned layer name for use in DXF
    static QString dxfLayerName( const QString &name );

    //! Returns DXF encoding for Qt encoding
    static QString dxfEncoding( const QString &name );

    //! Returns list of available DXF encodings
    static QStringList encodings();

    /**
     * Add a label to the dxf output.
     *
     * \note not available in Python bindings
     */
    void drawLabel( const QString &layerId, QgsRenderContext &context, pal::LabelPosition *label, const QgsPalLayerSettings &settings ) SIP_SKIP override;

    /**
     * Register name of layer for feature
     * \param layerId id of layer
     * \param fid id of feature
     * \param layer dxf layer of feature
     *
     * \deprecated Will be made private in QGIS 4
     */
    Q_DECL_DEPRECATED void registerDxfLayer( const QString &layerId, QgsFeatureId fid, const QString &layer );

  private:

#ifdef SIP_RUN
    QgsDxfExport( const QgsDxfExport &other );
    QgsDxfExport &operator=( const QgsDxfExport & );
#endif
    //! Extent for export, only intersecting features are exported. If the extent is an empty rectangle, all features are exported
    QgsRectangle mExtent;
    //! Scale for symbology export (used if symbols units are mm)
    double mSymbologyScale = 1.0;
    SymbologyExport mSymbologyExport = NoSymbology;
    QgsUnitTypes::DistanceUnit mMapUnits = QgsUnitTypes::DistanceMeters;
    bool mLayerTitleAsName = false;

    QTextStream mTextStream;

    int mSymbolLayerCounter = 0; //internal counter
    int mNextHandleId = DXF_HANDSEED;
    int mBlockCounter = 0;

    QHash< const QgsSymbolLayer *, QString > mLineStyles; //symbol layer name types
    QHash< const QgsSymbolLayer *, QString > mPointSymbolBlocks; //reference to point symbol blocks

    //AC1009
    void writeHeader( const QString &codepage );
    void prepareRenderers();
    void writeTables();
    void writeBlocks();
    void writeEntities();
    void writeEntitiesSymbolLevels( DxfLayerJob *job );
    void stopRenderers();
    void writeEndFile();

    void startSection();
    void endSection();

    void writePoint( const QgsPoint &pt, const QString &layer, const QColor &color, QgsSymbolRenderContext &ctx, const QgsSymbolLayer *symbolLayer, const QgsSymbol *symbol, double angle );
    void writeDefaultLinetypes();
    void writeSymbolLayerLinetype( const QgsSymbolLayer *symbolLayer );
    void writeLinetype( const QString &styleName, const QVector<qreal> &pattern, QgsUnitTypes::RenderUnit u );

    /**
     * Helper method to calculate text properties from (PAL) label
     */
    void writeText( const QString &layer, const QString &text, pal::LabelPosition *label, const QgsPalLayerSettings &layerSettings, const QgsExpressionContext &expressionContext );

    /**
     * Writes geometry generator symbol layer.
     *
     * \param context the symbol render context, with the reference to the feature set
     * \param ct coordinate transform from CRS of feature being added to destination DXF CRS
     * \param layer the layer name
     * \param symbolLayer the symbol layer to write to the dxf file
     * \param allSymbolLayers if TRUE, all symbol layers of the subsymbol are written. If FALSE, only the first one is written
    */
    void addGeometryGeneratorSymbolLayer( QgsSymbolRenderContext &context, const QgsCoordinateTransform &ct, const QString &layer, QgsSymbolLayer *symbolLayer, bool allSymbolLayers );

    /**
     * Writes a feature to the DXF file.
     *
     * \param context the symbol render context, with the reference to the feature set
     * \param ct coordinate transform from CRS of feature being added to destination DXF CRS
     * \param layer the layer name
     * \param symbolLayer the symbol layer to write to the dxf file
     * \param symbol symbol associated with feature being added
     */
    void addFeature( QgsSymbolRenderContext &context, const QgsCoordinateTransform &ct, const QString &layer, const QgsSymbolLayer *symbolLayer, const QgsSymbol *symbol );

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

    QgsDxfExport::Flags mFlags = QgsDxfExport::Flags();

    void appendCurve( const QgsCurve &c, QVector<QgsPoint> &points, QVector<double> &bulges );
    void appendLineString( const QgsLineString &ls, QVector<QgsPoint> &points, QVector<double> &bulges );
    void appendCircularString( const QgsCircularString &cs, QVector<QgsPoint> &points, QVector<double> &bulges );
    void appendCompoundCurve( const QgsCompoundCurve &cc, QVector<QgsPoint> &points, QVector<double> &bulges );

    QgsRenderContext mRenderContext;
    // Internal cache for layer related information required during rendering
    QList<DxfLayerJob *> mJobs;
    std::unique_ptr<QgsLabelingEngine> mLabelingEngine;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsDxfExport::Flags )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsDxfExport::DxfPolylineFlags )

#endif // QGSDXFEXPORT_H
