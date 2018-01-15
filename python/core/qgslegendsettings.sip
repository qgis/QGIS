/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslegendsettings.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsLegendSettings
{
%Docstring
 The QgsLegendSettings class stores the appearance and layout settings
for legend drawing with QgsLegendRenderer. The content of the legend is given
in QgsLegendModel class.

.. versionadded:: 2.6
%End

%TypeHeaderCode
#include "qgslegendsettings.h"
%End
  public:
    QgsLegendSettings();

    void setTitle( const QString &t );
    QString title() const;

    Qt::AlignmentFlag titleAlignment() const;
%Docstring
Returns the alignment of the legend title

:return: Qt.AlignmentFlag for the legend title

.. seealso:: :py:func:`setTitleAlignment`
%End

    void setTitleAlignment( Qt::AlignmentFlag alignment );
%Docstring
Sets the alignment of the legend title

:param alignment: Text alignment for drawing the legend title

.. seealso:: :py:func:`titleAlignment`
%End

    QgsLegendStyle style( QgsLegendStyle::Style s ) const;
%Docstring
Returns style
%End
    void setStyle( QgsLegendStyle::Style s, const QgsLegendStyle &style );

    double boxSpace() const;
    void setBoxSpace( double s );

    void setWrapChar( const QString &t );
    QString wrapChar() const;

    double columnSpace() const;
    void setColumnSpace( double s );

    int columnCount() const;
    void setColumnCount( int c );

    bool splitLayer() const;
    void setSplitLayer( bool s );

    bool equalColumnWidth() const;
    void setEqualColumnWidth( bool s );

    QColor fontColor() const;
    void setFontColor( const QColor &c );

    QSizeF symbolSize() const;
    void setSymbolSize( QSizeF s );

    bool drawRasterStroke() const;
%Docstring
Returns whether a stroke will be drawn around raster symbol items.

.. seealso:: :py:func:`setDrawRasterStroke`

.. seealso:: :py:func:`rasterStrokeColor`

.. seealso:: :py:func:`rasterStrokeWidth`

.. versionadded:: 2.12
%End

    void setDrawRasterStroke( bool enabled );
%Docstring
Sets whether a stroke will be drawn around raster symbol items.

:param enabled: set to true to draw borders

.. seealso:: :py:func:`drawRasterStroke`

.. seealso:: :py:func:`setRasterStrokeColor`

.. seealso:: :py:func:`setRasterStrokeWidth`

.. versionadded:: 2.12
%End

    QColor rasterStrokeColor() const;
%Docstring
Returns the stroke color for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

.. seealso:: :py:func:`setRasterStrokeColor`

.. seealso:: :py:func:`drawRasterStroke`

.. seealso:: :py:func:`rasterStrokeWidth`

.. versionadded:: 2.12
%End

    void setRasterStrokeColor( const QColor &color );
%Docstring
Sets the stroke color for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

:param color: stroke color

.. seealso:: :py:func:`rasterStrokeColor`

.. seealso:: :py:func:`setDrawRasterStroke`

.. seealso:: :py:func:`setRasterStrokeWidth`

.. versionadded:: 2.12
%End

    double rasterStrokeWidth() const;
%Docstring
Returns the stroke width (in millimeters) for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

.. seealso:: :py:func:`setRasterStrokeWidth`

.. seealso:: :py:func:`drawRasterStroke`

.. seealso:: :py:func:`rasterStrokeColor`

.. versionadded:: 2.12
%End

    void setRasterStrokeWidth( double width );
%Docstring
Sets the stroke width for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

:param width: stroke width in millimeters

.. seealso:: :py:func:`rasterStrokeWidth`

.. seealso:: :py:func:`setDrawRasterStroke`

.. seealso:: :py:func:`setRasterStrokeColor`

.. versionadded:: 2.12
%End

    QSizeF wmsLegendSize() const;
    void setWmsLegendSize( QSizeF s );

    double lineSpacing() const;
    void setLineSpacing( double s );

    double mmPerMapUnit() const;
    void setMmPerMapUnit( double mmPerMapUnit );

    bool useAdvancedEffects() const;
    void setUseAdvancedEffects( bool use );

    double mapScale() const;
%Docstring
Returns the legend map scale.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`setMapScale`
%End

    void setMapScale( double scale );
%Docstring
Sets the legend map ``scale``.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`mapScale`
%End

    int dpi() const;
    void setDpi( int dpi );


    QStringList splitStringForWrapping( const QString &stringToSplt ) const;
%Docstring
Splits a string using the wrap char taking into account handling empty
wrap char which means no wrapping
%End

    void drawText( QPainter *p, double x, double y, const QString &text, const QFont &font ) const;
%Docstring
Draws Text. Takes care about all the composer specific issues (calculation to
pixel, scaling of font and painter to work around the Qt font bug)
%End

    void drawText( QPainter *p, const QRectF &rect, const QString &text, const QFont &font, Qt::AlignmentFlag halignment = Qt::AlignLeft, Qt::AlignmentFlag valignment = Qt::AlignTop, int flags = Qt::TextWordWrap ) const;
%Docstring
Like the above, but with a rectangle for multiline text

:param p: painter to use
:param rect: rectangle to draw into
:param text: text to draw
:param font: font to use
:param halignment: optional horizontal alignment
:param valignment: optional vertical alignment
:param flags: allows for passing Qt.TextFlags to control appearance of rendered text
%End

    QFont scaledFontPixelSize( const QFont &font ) const;
%Docstring
Returns a font where size is in pixel and font size is upscaled with FONT_WORKAROUND_SCALE
%End

    double pixelFontSize( double pointSize ) const;
%Docstring
Calculates font to from point size to pixel size
%End

    double textWidthMillimeters( const QFont &font, const QString &text ) const;
%Docstring
Returns the font width in millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE
%End

    double fontHeightCharacterMM( const QFont &font, QChar c ) const;
%Docstring
Returns the font height of a character in millimeters
%End

    double fontAscentMillimeters( const QFont &font ) const;
%Docstring
Returns the font ascent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE
%End

    double fontDescentMillimeters( const QFont &font ) const;
%Docstring
Returns the font descent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE
%End

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgslegendsettings.h                                         *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
