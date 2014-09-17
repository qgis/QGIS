/**Frame item for a composer multiframe item*/
class QgsComposerFrame: QgsComposerItem
{
%TypeHeaderCode
#include "qgscomposerframe.h"
%End

 public:
    QgsComposerFrame( QgsComposition* c, QgsComposerMultiFrame* mf, qreal x, qreal y, qreal width, qreal height );
    ~QgsComposerFrame();

    /**Sets the visible part of the multiframe's content which is visible within
     * this frame (relative to the total multiframe extent in mm).
     * @param section visible portion of content
     * @see extent
    */
    void setContentSection( const QRectF& section );
    
    /**Returns the parent multiframe for the frame.
     * @returns parent multiframe
     */
    QgsComposerMultiFrame* multiFrame() const;
    
    //Overriden to allow multiframe to set display name
    virtual QString displayName() const;
    
    //Overriden to handle fixed frame sizes set by multi frame
    void setSceneRect( const QRectF& rectangle );   
    
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );
    void beginItemCommand( const QString& text );
    void endItemCommand();
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );
    int type() const;
    
    /**Returns the visible portion of the multi frame's content which
     * is shown in this frame.
     * @returns extent of visible portion
     * @note added in QGIS 2.5
     * @see setContentSection
     */
    QRectF extent() const;

};
