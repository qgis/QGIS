
/** \ingroup core
 * Item model implementation based on layer tree model for composer legend.
 * Overrides some functionality of QgsLayerTreeModel to better fit the needs of composer legend.
 *
 * @note added in 2.6
 */
class QgsLegendModel : QgsLayerTreeModel
{
%TypeHeaderCode
#include <qgscomposerlegend.h>
%End

  public:
    //! Construct the model based on the given layer tree
    QgsLegendModel( QgsLayerTree* rootNode, QObject *parent /TransferThis/ = 0 );

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
#include <qgslegendstyle.h>
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

    /** Sets whether the legend should automatically resize to fit its contents.
     * @param enabled set to false to disable automatic resizing. The legend frame will not
     * be expanded to fit legend items, and items may be cropped from display.
     * @see resizeToContents()
     * @note added in QGIS 3.0
     */
    void setResizeToContents( bool enabled );

    /** Returns whether the legend should automatically resize to fit its contents.
     * @see setResizeToContents()
     * @note added in QGIS 3.0
     */
    bool resizeToContents() const;

    //! @note added in 2.6
    QgsLegendModel* model();

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
    QgsLegendStyle & rstyle( QgsLegendStyle::Style s );
    /** Returns style */
    QgsLegendStyle style( QgsLegendStyle::Style s ) const;
    void setStyle( QgsLegendStyle::Style s, const QgsLegendStyle& style );

    QFont styleFont( QgsLegendStyle::Style s ) const;
    /** Set style font */
    void setStyleFont( QgsLegendStyle::Style s, const QFont& f );

    /** Set style margin*/
    void setStyleMargin( QgsLegendStyle::Style s, double margin );
    void setStyleMargin( QgsLegendStyle::Style s, QgsLegendStyle::Side side, double margin );

    /** Returns the spacing in-between lines in mm
     * @note added in 3.0
     * @see setLineSpacing
     */
    double lineSpacing() const;
    /** Sets the spacing in-between multiple lines
     * @param spacing Double value to use as spacing in between multiple lines
     * @note added in 3.0
     * @see lineSpacing
     */
    void setLineSpacing( double spacing );

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

    /** Returns whether a stroke will be drawn around raster symbol items.
     * @see setDrawRasterStroke()
     * @see rasterStrokeColor()
     * @see rasterStrokeWidth()
     * @note added in QGIS 2.12
     */
    bool drawRasterStroke() const;

    /** Sets whether a stroke will be drawn around raster symbol items.
     * @param enabled set to true to draw borders
     * @see drawRasterStroke()
     * @see setRasterStrokeColor()
     * @see setRasterStrokeWidth()
     * @note added in QGIS 2.12
     */
    void setDrawRasterStroke( bool enabled );

    /** Returns the stroke color for the stroke drawn around raster symbol items. The stroke is
     * only drawn if drawRasterStroke() is true.
     * @see setRasterStrokeColor()
     * @see drawRasterStroke()
     * @see rasterStrokeWidth()
     * @note added in QGIS 2.12
     */
    QColor rasterStrokeColor() const;

    /** Sets the stroke color for the stroke drawn around raster symbol items. The stroke is
     * only drawn if drawRasterStroke() is true.
     * @param color stroke color
     * @see rasterStrokeColor()
     * @see setDrawRasterStroke()
     * @see setRasterStrokeWidth()
     * @note added in QGIS 2.12
     */
    void setRasterStrokeColor( const QColor& color );

    /** Returns the stroke width (in millimeters) for the stroke drawn around raster symbol items. The stroke is
     * only drawn if drawRasterStroke() is true.
     * @see setRasterStrokeWidth()
     * @see drawRasterStroke()
     * @see rasterStrokeColor()
     * @note added in QGIS 2.12
     */
    double rasterStrokeWidth() const;

    /** Sets the stroke width for the stroke drawn around raster symbol items. The stroke is
     * only drawn if drawRasterStroke() is true.
     * @param width stroke width in millimeters
     * @see rasterStrokeWidth()
     * @see setDrawRasterStroke()
     * @see setRasterStrokeColor()
     * @note added in QGIS 2.12
     */
    void setRasterStrokeWidth( double width );

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap() const;

    /** Updates the model and all legend entries*/
    void updateLegend();

    /** Stores state in Dom node
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc Dom document
       */
    bool writeXml( QDomElement& elem, QDomDocument & doc ) const;

    /** Sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXml( const QDomElement& itemElem, const QDomDocument& doc );

    //Overridden to show legend title
    virtual QString displayName() const;

    const QgsLegendSettings& legendSettings() const;

  public slots:
    /** Data changed*/
    void synchronizeWithModel();
    /** Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();
    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext* context = nullptr );

};
