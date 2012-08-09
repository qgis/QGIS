
class QgsComposerFrame: QgsComposerItem
{
%TypeHeaderCode
#include "qgscomposerframe.h"
%End

 public:
    QgsComposerFrame( QgsComposition* c, QgsComposerMultiFrame* mf, qreal x, qreal y, qreal width, qreal height );
    ~QgsComposerFrame();

    /**Sets the part of this frame (relative to the total multiframe extent in mm)*/
    void setContentSection( const QRectF& section );

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );

    void beginItemCommand( const QString& text );
    void endItemCommand();

    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    int type() const;

    QgsComposerMultiFrame* multiFrame();
};