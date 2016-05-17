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
    void setSymbologyScaleDenominator( double d );

    /**
     * Retrieve reference scale for output
     * @returns reference scale
     * @see setSymbologyScaleDenominator
     */
    double symbologyScaleDenominator() const;

    /**
     * Set map units
     * @param u unit
     */
    void setMapUnits( QGis::UnitType u );

    /**
     * Retrieve map units
     * @returns unit
     * @see setMapUnits
     */
    QGis::UnitType mapUnits() const;

    /**
     * Set symbology export mode
     * @param e the mode
     */
    void setSymbologyExport( QgsDxfExport::SymbologyExport e );

    /**
     * Get symbology export mode
     * @returns mode
     * @see setSymbologyExport
     */
    QgsDxfExport::SymbologyExport symbologyExport() const;

    /**
     * Set extent of area to export
     * @param r area to export
     */
    void setExtent( const QgsRectangle &r );

    /**
     * Get extent of area to export
     * @returns area to export
     * @see setExtent
     */
    QgsRectangle extent() const;

    /**
     * Enable use of title (where set) instead of layer name,
     * when attribute index of corresponding layer index is -1
     * @param layerTitleAsName flag
     * @see addLayers
     */
    void setLayerTitleAsName( bool layerTitleAsName );

    /**
     * Retrieve wether layer title (where set) instead of name shall be use
     * @returns flag
     * @see setLayerTitleAsName
     */
    bool layerTitleAsName();

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
    void writeGroup( int code, int i ) /PyName=writeGroupInt/;

    /**
     * Write a group code with a floating point value
     * @param code group code
     * @param d floating point value
     * @note available in python bindings as writeGroupDouble
     */
    void writeGroup( int code, double d ) /PyName=writeGroupDouble/;

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
    void writeGroup( int code, const QgsPoint &p, double z = 0.0, bool skipz = false ) /PyName=writeGroupPoint/;

    /**
     * Write a group code with a point
     * @param code group code
     * @param p point value
     * @note available in python bindings as writeGroupPointV2
     * @note added in 2.15
     */
    void writeGroup( int code, const QgsPointV2 &p ) /PyName=writeGroupPointV2/;

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
     */
    void writePolyline( const QgsPolyline &line, const QString &layer, const QString &lineStyleName, const QColor& color, double width = -1 );

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
    // void writePolyline( const QgsPointSequenceV2 &line, const QString &layer, const QString &lineStyleName, const QColor& color, double width = -1 );

    /**
     * Draw dxf filled polygon (HATCH)
     * @param polygon polygon
     * @param layer layer name to use
     * @param hatchPattern hatchPattern to use
     * @param color color to use
     * @deprecated use version with QgsRingSequenceV2
     */
    void writePolygon( const QgsPolygon &polygon, const QString &layer, const QString &hatchPattern, const QColor& color ) /Deprecated/;

    /**
     * Draw dxf filled polygon (HATCH)
     * @param polygon polygon
     * @param layer layer name to use
     * @param hatchPattern hatchPattern to use
     * @param color color to use
     * @note not available in Python bindings
     * @note added in 2.15
     */
    // void writePolygon( const QgsRingSequenceV2 &polygon, const QString &layer, const QString &hatchPattern, const QColor& color );

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
    void writeSolid( const QString &layer, const QColor& color, const QgsPoint &pt1, const QgsPoint &pt2, const QgsPoint &pt3, const QgsPoint &pt4 ) /Deprecated/;

    //! Write line (as a polyline)
    void writeLine( const QgsPoint &pt1, const QgsPoint &pt2, const QString &layer, const QString &lineStyleName, const QColor& color, double width = -1 );

    //! Write line (as a polyline)
    //! @note added in 2.15
    void writeLine( const QgsPointV2 &pt1, const QgsPointV2 &pt2, const QString &layer, const QString &lineStyleName, const QColor& color, double width = -1 );

    //! Write point
    //! @deprecated use QgsPointV2 version
    void writePoint( const QString &layer, const QColor& color, const QgsPoint &pt ) /Deprecated/;

    //! Write point
    //! @note available in Python bindings as writePointV2
    //! @note added in 2.15
    void writePoint( const QString &layer, const QColor& color, const QgsPointV2 &pt ) /PyName=writePointV2/;

    //! Write filled circle (as hatch)
    //! @deprecated use QgsPointV2 version
    void writeFilledCircle( const QString &layer, const QColor& color, const QgsPoint &pt, double radius ) /Deprecated/;

    //! Write filled circle (as hatch)
    //! @note available in Python bindings as writePointV2
    //! @note added in 2.15
    void writeFilledCircle( const QString &layer, const QColor& color, const QgsPointV2 &pt, double radius ) /PyName=writeFillCircleV2/;

    //! Write circle (as polyline)
    //! @deprecated use QgsPointV2 version
    void writeCircle( const QString &layer, const QColor& color, const QgsPoint &pt, double radius, const QString &lineStyleName, double width ) /Deprecated/;

    //! Write circle (as polyline)
    //! @note available in Python bindings as writeCircleV2
    //! @note added in 2.15
    void writeCircle( const QString &layer, const QColor& color, const QgsPointV2 &pt, double radius, const QString &lineStyleName, double width ) /PyName=writeCircleV2/;

    //! Write text (TEXT)
    //! @deprecated use QgsPointV2 version
    void writeText( const QString &layer, const QString &text, const QgsPoint &pt, double size, double angle, const QColor& color ) /Deprecated/;

    //! Write text (TEXT)
    //! @note available in Python bindings as writeTextV2
    //! @note added in 2.15
    void writeText( const QString &layer, const QString &text, const QgsPointV2 &pt, double size, double angle, const QColor& color ) /PyName=writeTextV2/;

    //! Write mtext (MTEXT)
    //! @deprecated use QgsPointV2 version
    void writeMText( const QString &layer, const QString &text, const QgsPoint &pt, double width, double angle, const QColor& color ) /Deprecated/;

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
    // void drawLabel( QString layerId, QgsRenderContext& context, pal::LabelPosition* label, const QgsPalLayerSettings &settings );

    /** Register name of layer for feature
     * @param layerId id of layer
     * @param fid id of feature
     * @param layer dxf layer of feature
     */
    void registerDxfLayer( QString layerId, QgsFeatureId fid, QString layer );

};
