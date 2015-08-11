
/** \ingroup MapComposer
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


/** \ingroup MapComposer
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

    /** Return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /** Paints the legend and calculates its size. If painter is 0, only size is calculated*/
    QSizeF paintAndDetermineSize( QPainter* painter );

    /** Sets item box to the whole content*/
    void adjustBoxSize();

    /** Returns pointer to the legend model*/
    //! @note deprecated in 2.6 - use modelV2()
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
    void setStyle( QgsComposerLegendStyle::Style s, const QgsComposerLegendStyle style );

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

  public slots:
    /** Data changed*/
    void synchronizeWithModel();
    /** Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();
};
