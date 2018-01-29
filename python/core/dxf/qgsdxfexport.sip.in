/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/dxf/qgsdxfexport.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsDxfExport
{

%TypeHeaderCode
#include "qgsdxfexport.h"
%End
  public:
    enum SymbologyExport
    {
      NoSymbology,
      FeatureSymbology,
      SymbolLayerSymbology
    };

    enum Flag
    {
      FlagNoMText,
    };
    typedef QFlags<QgsDxfExport::Flag> Flags;


    QgsDxfExport();
%Docstring
Constructor for QgsDxfExport.
%End

    void setMapSettings( const QgsMapSettings &settings );
%Docstring
Set map settings and assign layer name attributes

:param settings: map settings to apply
%End

    void setFlags( QgsDxfExport::Flags flags );
%Docstring
Sets the export flags.

.. versionadded:: 3.0

.. seealso:: :py:func:`flags`
%End

    QgsDxfExport::Flags flags() const;
%Docstring
Returns the export flags.

.. versionadded:: 3.0

.. seealso:: :py:func:`setFlags`
%End

    void addLayers( const QList< QPair<QgsVectorLayer *, int > > &layers );
%Docstring
Add layers to export

:param layers: list of layers and corresponding attribute indexes that determine the layer name (-1 for original layer name or title)

.. seealso:: :py:func:`setLayerTitleAsName`
%End

    int writeToFile( QIODevice *d, const QString &codec );  //maybe add progress dialog? other parameters (e.g. scale, dpi)?

    void setSymbologyScale( double scale );
%Docstring
Set reference ``scale`` for output.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. versionadded:: 3.0

.. seealso:: :py:func:`symbologyScale`
%End

    double symbologyScale() const;
%Docstring
Returns the reference scale for output.
The  scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. versionadded:: 3.0

.. seealso:: :py:func:`setSymbologyScale`
%End

    QgsUnitTypes::DistanceUnit mapUnits() const;
%Docstring
Retrieve map units

:return: unit
%End

    void setDestinationCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Set destination CRS

.. seealso:: :py:func:`destinationCrs`

.. versionadded:: 3.0
%End

    QgsCoordinateReferenceSystem destinationCrs() const;
%Docstring
Returns the destination CRS, or an invalid CRS if no reprojection will be done.

.. seealso:: :py:func:`setDestinationCrs`

.. versionadded:: 3.0
%End

    void setSymbologyExport( QgsDxfExport::SymbologyExport e );
%Docstring
Set symbology export mode

:param e: the mode
%End

    QgsDxfExport::SymbologyExport symbologyExport() const;
%Docstring
Get symbology export mode

:return: mode

.. seealso:: :py:func:`setSymbologyExport`
%End

    void setExtent( const QgsRectangle &r );
%Docstring
Set extent of area to export

:param r: area to export
%End

    QgsRectangle extent() const;
%Docstring
Get extent of area to export

:return: area to export

.. seealso:: :py:func:`setExtent`
%End

    void setLayerTitleAsName( bool layerTitleAsName );
%Docstring
Enable use of title (where set) instead of layer name,
when attribute index of corresponding layer index is -1

:param layerTitleAsName: flag

.. seealso:: :py:func:`addLayers`
%End

    bool layerTitleAsName();
%Docstring
Retrieve whether layer title (where set) instead of name shall be use

:return: flag

.. seealso:: :py:func:`setLayerTitleAsName`
%End

    void setForce2d( bool force2d );
%Docstring
Force 2d output (eg. to support linewidth in polylines)

:param force2d: flag

.. seealso:: :py:func:`force2d`
%End

    bool force2d();
%Docstring
Retrieve whether the output should be forced to 2d

:return: flag

.. seealso:: :py:func:`setForce2d`
%End

    static int closestColorMatch( QRgb color );
%Docstring
Get DXF palette index of nearest entry for given color

:param color:
%End

    QString layerName( const QString &id, const QgsFeature &f ) const;
%Docstring
Get layer name for feature

:param id: layer id of layer
:param f: feature of layer

:return: layer name for feature
%End

    QString layerName( QgsVectorLayer *vl ) const;
%Docstring
Get name for layer respecting the use layer title as layer name mode

:param vl: the vector layer

:return: name of layer

.. seealso:: :py:func:`setLayerTitleAsName`
%End

    void writeGroup( int code, int i ) /PyName=writeGroupInt/;
%Docstring
Write a tuple of group code and integer value

:param code: group code
:param i: integer value

.. note::

   available in Python bindings as writeGroupInt
%End

    void writeGroup( int code, double d ) /PyName=writeGroupDouble/;
%Docstring
Write a group code with a floating point value

:param code: group code
:param d: floating point value

.. note::

   available in Python bindings as writeGroupDouble
%End

    void writeGroup( int code, const QString &s );
%Docstring
Write a group code with a string value

:param code: group code
:param s: string value
%End

    void writeGroup( int code, const QgsPoint &p ) /PyName=writeGroupPointV2/;
%Docstring
Write a group code with a point

:param code: group code
:param p: point value

.. note::

   available in Python bindings as writeGroupPointV2

.. versionadded:: 2.15
%End

    void writeGroup( const QColor &color, int exactMatch = 62, int rgbCode = 420, int transparencyCode = 440 );
%Docstring
Write a group code with color value

:param color: color
:param exactMatch: group code to use if the color has an exact match in the dxf palette
:param rgbCode: group code to use if the color doesn't have an exact match or has a transparency component
:param transparencyCode: group code to use for transparency component

.. note::

   available in Python bindings as writeGroupPoint
%End

    void writeGroupCode( int code );
%Docstring
Write a group code

:param code: group code value
%End

    void writeInt( int i );
%Docstring
Write an integer value

:param i: integer value
%End

    void writeDouble( double d );
%Docstring
Write a floating point value

:param d: floating point value
%End

    void writeString( const QString &s );
%Docstring
Write a string value

:param s: string value
%End

    int writeHandle( int code = 5, int handle = 0 );
%Docstring
Write a tuple of group code and a handle

:param code: group code to use
:param handle: handle to use (0 generates a new handle)

:return: the used handle
%End



    void writeLine( const QgsPoint &pt1, const QgsPoint &pt2, const QString &layer, const QString &lineStyleName, const QColor &color, double width = -1 );
%Docstring
Write line (as a polyline)

.. versionadded:: 2.15
%End

    void writePoint( const QString &layer, const QColor &color, const QgsPoint &pt ) /PyName=writePointV2/;
%Docstring
Write point

.. note::

   available in Python bindings as writePointV2

.. versionadded:: 2.15
%End

    void writeFilledCircle( const QString &layer, const QColor &color, const QgsPoint &pt, double radius ) /PyName=writeFillCircleV2/;
%Docstring
Write filled circle (as hatch)

.. note::

   available in Python bindings as writePointV2

.. versionadded:: 2.15
%End

    void writeCircle( const QString &layer, const QColor &color, const QgsPoint &pt, double radius, const QString &lineStyleName, double width ) /PyName=writeCircleV2/;
%Docstring
Write circle (as polyline)

.. note::

   available in Python bindings as writeCircleV2

.. versionadded:: 2.15
%End

    void writeText( const QString &layer, const QString &text, const QgsPoint &pt, double size, double angle, const QColor &color ) /PyName=writeTextV2/;
%Docstring
Write text (TEXT)

.. note::

   available in Python bindings as writeTextV2

.. versionadded:: 2.15
%End

    void writeMText( const QString &layer, const QString &text, const QgsPoint &pt, double width, double angle, const QColor &color );
%Docstring
Write mtext (MTEXT)

.. note::

   available in Python bindings as writeMTextV2

.. versionadded:: 2.15
%End

    static double mapUnitScaleFactor( double scale, QgsUnitTypes::RenderUnit symbolUnits, QgsUnitTypes::DistanceUnit mapUnits );
%Docstring
Calculates a scaling factor to convert from map units to a specified symbol unit.
The ``scale`` parameter indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
%End

    static QString dxfLayerName( const QString &name );
%Docstring
Return cleaned layer name for use in DXF
%End

    static QString dxfEncoding( const QString &name );
%Docstring
return DXF encoding for Qt encoding
%End

    static QStringList encodings();
%Docstring
return list of available DXF encodings
%End


    void registerDxfLayer( const QString &layerId, QgsFeatureId fid, const QString &layer );
%Docstring
Register name of layer for feature

:param layerId: id of layer
:param fid: id of feature
:param layer: dxf layer of feature
%End

};

QFlags<QgsDxfExport::Flag> operator|(QgsDxfExport::Flag f1, QFlags<QgsDxfExport::Flag> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/dxf/qgsdxfexport.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
