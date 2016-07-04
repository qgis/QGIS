
/** \ingroup core
 * Item model implementation based on layer tree model for composer legend.
 * Overrides some functionality of QgsLayerTreeModel to better fit the needs of composer legend.
 *
 * @note added in 2.6
 */
class QgsLegendModelV2 : QgsLayerTreeModel
{
%TypeHeaderCode
#include <qgscomposerlegend.h>
%End

  public:
    QgsLegendModelV2( QgsLayerTreeGroup* rootNode, QObject *parent /TransferThis/ = 0 );

    QVariant data( const QModelIndex& index,  int role ) const;

    Qt::ItemFlags flags( const QModelIndex &index ) const;
};


/** \ingroup core
 * A legend that can be placed onto a map composition
 */
class QgsComposerLegend : QgsComposerItem
{
%TypeHeaderCode
#include <qgscomposerlegend.h>
#include <qgscomposerlegendstyle.h>
%End

  public:
    QgsComposerLegend( QgsComposition* composition /TransferThis/);
    ~QgsComposerLegend();

    /** Return correct graphics item type. */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** Paints the legend and calculates its size. If painter is 0, only size is calculated*/
    QSizeF paintAndDetermineSize( QPainter* painter );

    /** Sets item box to the whole content*/
    void adjustBoxSize();

    /** Returns pointer to the legend model*/
    //! @deprecated in 2.6 - use modelV2()
    QgsLegendModel* model() /Deprecated/;

    //! @note added in 2.6
    QgsLegendModelV2* modelV2();

    //! @note added in 2.6
    void setAutoUpdateModel( bool autoUpdate );
    //! @note added in 2.6
    bool autoUpdateModel() const;

    //! Set whether legend items should be filtered to show just the ones visible in the associated map
    //! @note added in 2.6
    void setLegendFilterByMapEnabled( bool enabled );
    //! Find out whether legend items are filtered to show just the ones visible in the associated map
    //! @note added in 2.6
    bool legendFilterByMapEnabled() const;

    //! Update() overloading. Use it rather than update()
    //! @note added in 2.12
    virtual void updateItem();

    //! When set to true, during an atlas rendering, it will filter out legend elements
    //! where features are outside the current atlas feature.
    //! @note added in 2.14
    void setLegendFilterOutAtlas( bool doFilter );

    //! Whether to filter out legend elements outside of the current atlas feature
    //! @see setLegendFilterOutAtlas()
    //! @note added in 2.14
    bool legendFilterOutAtlas() const;

    //setters and getters
    void setTitle( const QString& t );
    QString title() const;

    /** Returns the alignment of the legend title
     * @returns Qt::AlignmentFlag for the legend title
     * @note added in 2.3
     * @see setTitleAlignment
     */
    Qt::AlignmentFlag titleAlignment() const;
    /** Sets the alignment of the legend title
     * @param alignment Text alignment for drawing the legend title
     * @note added in 2.3
     * @see titleAlignment
     */
    void setTitleAlignment( Qt::AlignmentFlag alignment );

    /** Returns reference to modifiable style */
    QgsComposerLegendStyle & rstyle( QgsComposerLegendStyle::Style s );
    /** Returns style */
    QgsComposerLegendStyle style( QgsComposerLegendStyle::Style s ) const;
    void setStyle( QgsComposerLegendStyle::Style s, const QgsComposerLegendStyle& style );

    QFont styleFont( QgsComposerLegendStyle::Style s ) const;
    /** Set style font */
    void setStyleFont( QgsComposerLegendStyle::Style s, const QFont& f );

    /** Set style margin*/
    void setStyleMargin( QgsComposerLegendStyle::Style s, double margin );
    void setStyleMargin( QgsComposerLegendStyle::Style s, QgsComposerLegendStyle::Side side, double margin );

    double boxSpace() const;
    void setBoxSpace( double s );

    double columnSpace() const;
    void setColumnSpace( double s );

    QColor fontColor() const;
    void setFontColor( const QColor& c );

    double symbolWidth() const;
    void setSymbolWidth( double w );

    double symbolHeight() const;
    void setSymbolHeight( double h );

    double wmsLegendWidth() const;
    void setWmsLegendWidth( double w );

    double wmsLegendHeight() const;
    void setWmsLegendHeight( double h );

    void setWrapChar( const QString& t );
    QString wrapChar() const;

    int columnCount() const;
    void setColumnCount( int c );

    int splitLayer() const;
    void setSplitLayer( bool s );

    int equalColumnWidth() const;
    void setEqualColumnWidth( bool s );

    /** Returns whether a border will be drawn around raster symbol items.
     * @see setDrawRasterBorder()
     * @see rasterBorderColor()
     * @see rasterBorderWidth()
     * @note added in QGIS 2.12
     */
    bool drawRasterBorder() const;

    /** Sets whether a border will be drawn around raster symbol items.
     * @param enabled set to true to draw borders
     * @see drawRasterBorder()
     * @see setRasterBorderColor()
     * @see setRasterBorderWidth()
     * @note added in QGIS 2.12
     */
    void setDrawRasterBorder( bool enabled );

    /** Returns the border color for the border drawn around raster symbol items. The border is
     * only drawn if drawRasterBorder() is true.
     * @see setRasterBorderColor()
     * @see drawRasterBorder()
     * @see rasterBorderWidth()
     * @note added in QGIS 2.12
     */
    QColor rasterBorderColor() const;

    /** Sets the border color for the border drawn around raster symbol items. The border is
     * only drawn if drawRasterBorder() is true.
     * @param color border color
     * @see rasterBorderColor()
     * @see setDrawRasterBorder()
     * @see setRasterBorderWidth()
     * @note added in QGIS 2.12
     */
    void setRasterBorderColor( const QColor& color );

    /** Returns the border width (in millimeters) for the border drawn around raster symbol items. The border is
     * only drawn if drawRasterBorder() is true.
     * @see setRasterBorderWidth()
     * @see drawRasterBorder()
     * @see rasterBorderColor()
     * @note added in QGIS 2.12
     */
    double rasterBorderWidth() const;

    /** Sets the border width for the border drawn around raster symbol items. The border is
     * only drawn if drawRasterBorder() is true.
     * @param width border width in millimeters
     * @see rasterBorderWidth()
     * @see setDrawRasterBorder()
     * @see setRasterBorderColor()
     * @note added in QGIS 2.12
     */
    void setRasterBorderWidth( double width );

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap() const;

    /** Updates the model and all legend entries*/
    void updateLegend();

    /** Stores state in Dom node
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** Sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    //Overridden to show legend title
    virtual QString displayName() const;

  public slots:
    /** Data changed*/
    void synchronizeWithModel();
    /** Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();
};
