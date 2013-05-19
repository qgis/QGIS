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

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    /**Paints the legend and calculates its size. If painter is 0, only size is calculated*/
    QSizeF paintAndDetermineSize( QPainter* painter );

    /**Sets item box to the whole content*/
    void adjustBoxSize();

    /**Returns pointer to the legend model*/
    QgsLegendModel* model();

    //setters and getters
    void setTitle( const QString& t );
    QString title() const;

    /*
    QgsComposerLegendStyle style(QgsComposerLegendStyle::Style s) const;
    void setStyle(QgsComposerLegendStyle::Style s, const QgsComposerLegendStyle style);
    QFont styleFont( QgsComposerLegendStyle::Style s ) const;
    void setStyleFont( QgsComposerLegendStyle::Style s, const QFont& f );
    void setStyleMargin( QgsComposerLegendStyle::Style s, double margin );
    void setStyleMargin( QgsComposerLegendStyle::Style s, QgsComposerLegendStyle::Side side, double margin );
    */

    double boxSpace() const;
    void setBoxSpace( double s );

    double symbolWidth() const;
    void setSymbolWidth( double w );

    double symbolHeight() const;
    void setSymbolHeight( double h );

    void setWrapChar( const QString& t );
    QString wrapChar() const;

    void setComposerMap( const QgsComposerMap* map );
    const QgsComposerMap* composerMap() const;

    /**Updates the model and all legend entries*/
    void updateLegend();

    /** stores state in Dom node
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  public slots:
    /**Data changed*/
    void synchronizeWithModel();
    /**Sets mCompositionMap to 0 if the map is deleted*/
    void invalidateCurrentMap();
};
