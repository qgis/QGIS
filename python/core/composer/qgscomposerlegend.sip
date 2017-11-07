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
 :rtype: int
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

%Docstring
 Reimplementation of QCanvasItem.paint
%End

    QSizeF paintAndDetermineSize( QPainter *painter );
%Docstring
Paints the legend and calculates its size. If painter is 0, only size is calculated
 :rtype: QSizeF
%End

    void adjustBoxSize();
%Docstring
Sets item box to the whole content
%End

    void setResizeToContents( bool enabled );
%Docstring
 Sets whether the legend should automatically resize to fit its contents.
 \param enabled set to false to disable automatic resizing. The legend frame will not
 be expanded to fit legend items, and items may be cropped from display.
.. seealso:: resizeToContents()
.. versionadded:: 3.0
%End

    bool resizeToContents() const;
%Docstring
 Returns whether the legend should automatically resize to fit its contents.
.. seealso:: setResizeToContents()
.. versionadded:: 3.0
 :rtype: bool
%End


    QgsLegendModel *model();
%Docstring
 Returns the legend model
 :rtype: QgsLegendModel
%End

    void setAutoUpdateModel( bool autoUpdate );
%Docstring
.. versionadded:: 2.6
%End
    bool autoUpdateModel() const;
%Docstring
.. versionadded:: 2.6
 :rtype: bool
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
 :rtype: bool
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
.. seealso:: setLegendFilterOutAtlas()
.. versionadded:: 2.14
 :rtype: bool
%End

    void setTitle( const QString &t );
    QString title() const;
%Docstring
 :rtype: str
%End

    Qt::AlignmentFlag titleAlignment() const;
%Docstring
 Returns the alignment of the legend title
 :return: Qt.AlignmentFlag for the legend title
.. versionadded:: 2.3
.. seealso:: setTitleAlignment
 :rtype: Qt.AlignmentFlag
%End

    void setTitleAlignment( Qt::AlignmentFlag alignment );
%Docstring
 Sets the alignment of the legend title
 \param alignment Text alignment for drawing the legend title
.. versionadded:: 2.3
.. seealso:: titleAlignment
%End

    QgsLegendStyle &rstyle( QgsLegendStyle::Style s );
%Docstring
Returns reference to modifiable style
 :rtype: QgsLegendStyle
%End
    QgsLegendStyle style( QgsLegendStyle::Style s ) const;
%Docstring
Returns style
 :rtype: QgsLegendStyle
%End
    void setStyle( QgsLegendStyle::Style s, const QgsLegendStyle &style );

    QFont styleFont( QgsLegendStyle::Style s ) const;
%Docstring
 :rtype: QFont
%End
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
.. seealso:: setLineSpacing
 :rtype: float
%End

    void setLineSpacing( double spacing );
%Docstring
 Sets the spacing in-between multiple lines
 \param spacing Double value to use as spacing in between multiple lines
.. versionadded:: 3.0
.. seealso:: lineSpacing
%End

    double boxSpace() const;
%Docstring
 :rtype: float
%End
    void setBoxSpace( double s );

    double columnSpace() const;
%Docstring
 :rtype: float
%End
    void setColumnSpace( double s );

    QColor fontColor() const;
%Docstring
 :rtype: QColor
%End
    void setFontColor( const QColor &c );

    double symbolWidth() const;
%Docstring
 :rtype: float
%End
    void setSymbolWidth( double w );

    double symbolHeight() const;
%Docstring
 :rtype: float
%End
    void setSymbolHeight( double h );

    double wmsLegendWidth() const;
%Docstring
 :rtype: float
%End
    void setWmsLegendWidth( double w );

    double wmsLegendHeight() const;
%Docstring
 :rtype: float
%End
    void setWmsLegendHeight( double h );

    void setWrapChar( const QString &t );
    QString wrapChar() const;
%Docstring
 :rtype: str
%End

    int columnCount() const;
%Docstring
 :rtype: int
%End
    void setColumnCount( int c );

    bool splitLayer() const;
%Docstring
 :rtype: bool
%End
    void setSplitLayer( bool s );

    bool equalColumnWidth() const;
%Docstring
 :rtype: bool
%End
    void setEqualColumnWidth( bool s );

    bool drawRasterStroke() const;
%Docstring
 Returns whether a stroke will be drawn around raster symbol items.
.. seealso:: setDrawRasterStroke()
.. seealso:: rasterStrokeColor()
.. seealso:: rasterStrokeWidth()
.. versionadded:: 2.12
 :rtype: bool
%End

    void setDrawRasterStroke( bool enabled );
%Docstring
 Sets whether a stroke will be drawn around raster symbol items.
 \param enabled set to true to draw borders
.. seealso:: drawRasterStroke()
.. seealso:: setRasterStrokeColor()
.. seealso:: setRasterStrokeWidth()
.. versionadded:: 2.12
%End

    QColor rasterStrokeColor() const;
%Docstring
 Returns the stroke color for the stroke drawn around raster symbol items. The stroke is
 only drawn if drawRasterStroke() is true.
.. seealso:: setRasterStrokeColor()
.. seealso:: drawRasterStroke()
.. seealso:: rasterStrokeWidth()
.. versionadded:: 2.12
 :rtype: QColor
%End

    void setRasterStrokeColor( const QColor &color );
%Docstring
 Sets the stroke color for the stroke drawn around raster symbol items. The stroke is
 only drawn if drawRasterStroke() is true.
 \param color stroke color
.. seealso:: rasterStrokeColor()
.. seealso:: setDrawRasterStroke()
.. seealso:: setRasterStrokeWidth()
.. versionadded:: 2.12
%End

    double rasterStrokeWidth() const;
%Docstring
 Returns the stroke width (in millimeters) for the stroke drawn around raster symbol items. The stroke is
 only drawn if drawRasterStroke() is true.
.. seealso:: setRasterStrokeWidth()
.. seealso:: drawRasterStroke()
.. seealso:: rasterStrokeColor()
.. versionadded:: 2.12
 :rtype: float
%End

    void setRasterStrokeWidth( double width );
%Docstring
 Sets the stroke width for the stroke drawn around raster symbol items. The stroke is
 only drawn if drawRasterStroke() is true.
 \param width stroke width in millimeters
.. seealso:: rasterStrokeWidth()
.. seealso:: setDrawRasterStroke()
.. seealso:: setRasterStrokeColor()
.. versionadded:: 2.12
%End

    void setComposerMap( const QgsComposerMap *map );
    const QgsComposerMap *composerMap() const;
%Docstring
 :rtype: QgsComposerMap
%End

    void updateLegend();
%Docstring
Updates the model and all legend entries
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
 Stores state in Dom node
 \param elem is Dom element corresponding to 'Composer' tag
 \param doc Dom document
 :rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
 Sets state from Dom document
 \param itemElem is Dom node corresponding to item tag
 \param doc is Dom document
 :rtype: bool
%End

    virtual QString displayName() const;

    const QgsLegendSettings &legendSettings() const;
%Docstring
 Returns the legend's renderer settings object.
.. versionadded:: 3.0
 :rtype: QgsLegendSettings
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
