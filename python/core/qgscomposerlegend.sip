/** \ingroup MapComposer
 * A legend that can be placed onto a map composition
 */
class QgsComposerLegend: QgsComposerItem
{
%TypeHeaderCode
#include <qgscomposerlegend.h>
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

    QFont titleFont() const;
    void setTitleFont( const QFont& f );

    QFont layerFont() const;
    void setLayerFont( const QFont& f );

    QFont itemFont() const;
    void setItemFont( const QFont& f );

    double boxSpace() const;
    void setBoxSpace( double s );

    double layerSpace() const;
    void setLayerSpace( double s );

    double symbolSpace() const;
    void setSymbolSpace( double s );

    double iconLabelSpace() const;
    void setIconLabelSpace( double s );

    double symbolWidth() const;
    void setSymbolWidth( double w );

    double symbolHeight() const;
    void setSymbolHeight( double h );

    /**Updates the model and all legend entries*/
    void updateLegend();

    /** stores state in Dom node
       * @param elem is Dom element corresponding to 'Composer' tag
       * @param temp write template file
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

  public slots:
    /**Data changed*/
    void synchronizeWithModel();
};
