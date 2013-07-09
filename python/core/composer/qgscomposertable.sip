/**A class to display feature attributes in the print composer*/
class QgsComposerTable: QgsComposerItem
{
%TypeHeaderCode
#include <qgscomposertable.h>
%End

  public:
    QgsComposerTable( QgsComposition* composition /TransferThis/ );
    virtual ~QgsComposerTable();

    /** return correct graphics item type. Added in v1.7 */
    virtual int type() const;

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    virtual bool writeXML( QDomElement& elem, QDomDocument & doc ) const = 0;
    virtual bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) = 0;

    void setLineTextDistance( double d );
    double lineTextDistance() const;

    void setHeaderFont( const QFont& f );
    QFont headerFont() const;

    void setContentFont( const QFont& f );
    QFont contentFont() const;

    void setShowGrid( bool show );
    bool showGrid() const;

    void setGridStrokeWidth( double w );
    double gridStrokeWidth() const;

    void setGridColor( const QColor& c );
    QColor gridColor() const;

    /**Adapts the size of the frame to match the content. This is normally done in the paint method, but sometimes
    it needs to be done before the first render*/
    void adjustFrameToSize();

  protected:
    /**Retrieves feature attributes*/
    // virtual bool getFeatureAttributes( QList<QgsAttributeMap>& attributeMaps );
    virtual QMap<int, QString> getHeaderLabels() const;
    /**Calculate the maximum width values of the vector attributes*/
    // virtual bool calculateMaxColumnWidths( QMap<int, double>& maxWidthMap, const QList< QMap<int, QVariant> >& attributeMaps ) const;
    /**Adapts the size of the item frame to match the content*/
    // void adaptItemFrame( const QMap<int, double>& maxWidthMap, const QList< QMap<int, QVariant> >& attributeMaps );
    void drawHorizontalGridLines( QPainter* p, int nAttributes );
    // void drawVerticalGridLines( QPainter* p, const QMap<int, double>& maxWidthMap );

    bool tableWriteXML( QDomElement& itemElem, QDomDocument& doc ) const;
    bool tableReadXML( const QDomElement& itemElem, const QDomDocument& doc );
};
