/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerlegend.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/







class QgsComposerLegend : QgsComposerItem
{
%Docstring
A legend that can be placed onto a map composition
%End

%TypeHeaderCode
#include "qgscomposerlegend.h"
%End
  public:
    QgsComposerLegend( QgsComposition *composition /TransferThis/ );
    ~QgsComposerLegend();

    virtual int type() const;
%Docstring
Return correct graphics item type.
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Reimplementation of QCanvasItem.paint
%End

    QSizeF paintAndDetermineSize( QPainter *painter );
%Docstring
Paints the legend and calculates its size. If painter is 0, only size is calculated
%End

    void adjustBoxSize();
%Docstring
Sets item box to the whole content
%End

    void setResizeToContents( bool enabled );
%Docstring
Sets whether the legend should automatically resize to fit its contents.

:param enabled: set to false to disable automatic resizing. The legend frame will not
be expanded to fit legend items, and items may be cropped from display.

.. seealso:: :py:func:`resizeToContents()`

.. versionadded:: 3.0
%End

    bool resizeToContents() const;
%Docstring
Returns whether the legend should automatically resize to fit its contents.

.. seealso:: :py:func:`setResizeToContents()`

.. versionadded:: 3.0
%End


    QgsLegendModel *model();
%Docstring
Returns the legend model
%End

    void setAutoUpdateModel( bool autoUpdate );
%Docstring

.. versionadded:: 2.6
%End
    bool autoUpdateModel() const;
%Docstring

.. versionadded:: 2.6
%End

    void setLegendFilterByMapEnabled( bool enabled );
%Docstring
Set whether legend items should be filtered to show just the ones visible in the associated map

.. versionadded:: 2.6
%End

    bool legendFilterByMapEnabled() const;
%Docstring
Find out whether legend items are filtered to show just the ones visible in the associated map

.. versionadded:: 2.6
%End

    virtual void updateItem();

%Docstring
Update() overloading. Use it rather than update()

.. versionadded:: 2.12
%End

    void setLegendFilterOutAtlas( bool doFilter );
%Docstring
When set to true, during an atlas rendering, it will filter out legend elements
where features are outside the current atlas feature.

.. versionadded:: 2.14
%End

    bool legendFilterOutAtlas() const;
%Docstring
Whether to filter out legend elements outside of the current atlas feature

.. seealso:: :py:func:`setLegendFilterOutAtlas()`

.. versionadded:: 2.14
%End

    void setTitle( const QString &t );
    QString title() const;

    Qt::AlignmentFlag titleAlignment() const;
%Docstring
Returns the alignment of the legend title

:return: Qt.AlignmentFlag for the legend title

.. versionadded:: 2.3

.. seealso:: :py:func:`setTitleAlignment`
%End

    void setTitleAlignment( Qt::AlignmentFlag alignment );
%Docstring
Sets the alignment of the legend title

:param alignment: Text alignment for drawing the legend title

.. versionadded:: 2.3

.. seealso:: :py:func:`titleAlignment`
%End

    QgsLegendStyle &rstyle( QgsLegendStyle::Style s );
%Docstring
Returns reference to modifiable style
%End
    QgsLegendStyle style( QgsLegendStyle::Style s ) const;
%Docstring
Returns style
%End
    void setStyle( QgsLegendStyle::Style s, const QgsLegendStyle &style );

    QFont styleFont( QgsLegendStyle::Style s ) const;
    void setStyleFont( QgsLegendStyle::Style s, const QFont &f );
%Docstring
Set style font
%End

    void setStyleMargin( QgsLegendStyle::Style s, double margin );
%Docstring
Set style margin
%End
    void setStyleMargin( QgsLegendStyle::Style s, QgsLegendStyle::Side side, double margin );

    double lineSpacing() const;
%Docstring
Returns the spacing in-between lines in mm

.. versionadded:: 3.0

.. seealso:: :py:func:`setLineSpacing`
%End

    void setLineSpacing( double spacing );
%Docstring
Sets the spacing in-between multiple lines

:param spacing: Double value to use as spacing in between multiple lines

.. versionadded:: 3.0

.. seealso:: :py:func:`lineSpacing`
%End

    double boxSpace() const;
    void setBoxSpace( double s );

    double columnSpace() const;
    void setColumnSpace( double s );

    QColor fontColor() const;
    void setFontColor( const QColor &c );

    double symbolWidth() const;
    void setSymbolWidth( double w );

    double symbolHeight() const;
    void setSymbolHeight( double h );

    double wmsLegendWidth() const;
    void setWmsLegendWidth( double w );

    double wmsLegendHeight() const;
    void setWmsLegendHeight( double h );

    void setWrapChar( const QString &t );
    QString wrapChar() const;

    int columnCount() const;
    void setColumnCount( int c );

    bool splitLayer() const;
    void setSplitLayer( bool s );

    bool equalColumnWidth() const;
    void setEqualColumnWidth( bool s );

    bool drawRasterStroke() const;
%Docstring
Returns whether a stroke will be drawn around raster symbol items.

.. seealso:: :py:func:`setDrawRasterStroke()`

.. seealso:: :py:func:`rasterStrokeColor()`

.. seealso:: :py:func:`rasterStrokeWidth()`

.. versionadded:: 2.12
%End

    void setDrawRasterStroke( bool enabled );
%Docstring
Sets whether a stroke will be drawn around raster symbol items.

:param enabled: set to true to draw borders

.. seealso:: :py:func:`drawRasterStroke()`

.. seealso:: :py:func:`setRasterStrokeColor()`

.. seealso:: :py:func:`setRasterStrokeWidth()`

.. versionadded:: 2.12
%End

    QColor rasterStrokeColor() const;
%Docstring
Returns the stroke color for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

.. seealso:: :py:func:`setRasterStrokeColor()`

.. seealso:: :py:func:`drawRasterStroke()`

.. seealso:: :py:func:`rasterStrokeWidth()`

.. versionadded:: 2.12
%End

    void setRasterStrokeColor( const QColor &color );
%Docstring
Sets the stroke color for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

:param color: stroke color

.. seealso:: :py:func:`rasterStrokeColor()`

.. seealso:: :py:func:`setDrawRasterStroke()`

.. seealso:: :py:func:`setRasterStrokeWidth()`

.. versionadded:: 2.12
%End

    double rasterStrokeWidth() const;
%Docstring
Returns the stroke width (in millimeters) for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

.. seealso:: :py:func:`setRasterStrokeWidth()`

.. seealso:: :py:func:`drawRasterStroke()`

.. seealso:: :py:func:`rasterStrokeColor()`

.. versionadded:: 2.12
%End

    void setRasterStrokeWidth( double width );
%Docstring
Sets the stroke width for the stroke drawn around raster symbol items. The stroke is
only drawn if drawRasterStroke() is true.

:param width: stroke width in millimeters

.. seealso:: :py:func:`rasterStrokeWidth()`

.. seealso:: :py:func:`setDrawRasterStroke()`

.. seealso:: :py:func:`setRasterStrokeColor()`

.. versionadded:: 2.12
%End

    void setComposerMap( const QgsComposerMap *map );
    const QgsComposerMap *composerMap() const;

    void updateLegend();
%Docstring
Updates the model and all legend entries
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
Stores state in Dom node

:param elem: is Dom element corresponding to 'Composer' tag
:param doc: Dom document
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
Sets state from Dom document

:param itemElem: is Dom node corresponding to item tag
:param doc: is Dom document
%End

    virtual QString displayName() const;


    const QgsLegendSettings &legendSettings() const;
%Docstring
Returns the legend's renderer settings object.

.. versionadded:: 3.0
%End

  public slots:
    void synchronizeWithModel();
%Docstring
Data changed
%End
    void invalidateCurrentMap();
%Docstring
Sets mCompositionMap to 0 if the map is deleted
%End

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = 0 );



};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerlegend.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
