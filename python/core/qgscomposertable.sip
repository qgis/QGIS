/**A class to display feature attributes in the print composer*/
class QgsComposerTable: QgsComposerItem
{
%TypeHeaderCode
#include <qgscomposertable.h>
%End

  public:
    QgsComposerTable( QgsComposition* composition /TransferThis/ );
    virtual ~QgsComposerTable();

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

    /**Adapts the size of the frame to match the content. This is normally done in the paint method, but sometimes \
    it needs to be done before the first render*/
    void adjustFrameToSize();

  protected:
    //virtual bool getFeatureAttributes( QList< QMap< int, QVariant> >& attributes );
    //virtual QMap<int, QString> getHeaderLabels() const;
};
