/** \ingroup MapComposer
 * Attribute table item for map composer.
 */

class QgsComposerAttributeTable: QgsComposerTable
{
%TypeHeaderCode
#include <qgscomposerattributetable.h>
%End

  public:
    QgsComposerAttributeTable( QgsComposition* composition /TransferThis/ );
    ~QgsComposerAttributeTable();

    /** \brief Reimplementation of QCanvasItem::paint*/
    virtual void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    void setVectorLayer( QgsVectorLayer* vl );
    const QgsVectorLayer* vectorLayer() const;

    void setComposerMap( const QgsComposerMap* map /TransferThis/ );
    const QgsComposerMap* composerMap() const;

    void setMaximumNumberOfFeatures( int nr );
    int maximumNumberOfFeatures() const;

    void setDisplayOnlyVisibleFeatures( bool b );
    bool displayOnlyVisibleFeatures() const;

    QSet<int> displayAttributes() const;
    void setDisplayAttributes( const QSet<int>& attr );

    QMap<int, QString> fieldAliasMap() const;
    void setFieldAliasMap( const QMap<int, QString>& map );

    /**Adapts mMaximumNumberOfFeatures depending on the rectangle height*/
    void setSceneRect( const QRectF& rectangle );

    //void setSortAttributes( const QList<QPair<int, bool> > att );
    //QList<QPair<int, bool> > sortAttributes() const;
};
