/** Frame item for a composer multiframe item*/
class QgsComposerFrame: QgsComposerItem
{
%TypeHeaderCode
#include "qgscomposerframe.h"
%End

  public:
    QgsComposerFrame( QgsComposition* c /TransferThis/, QgsComposerMultiFrame* mf, qreal x, qreal y, qreal width, qreal height );

    ~QgsComposerFrame();

    /** Sets the visible part of the multiframe's content which is visible within
     * this frame (relative to the total multiframe extent in mm).
     * @param section visible portion of content
     * @see extent
     */
    void setContentSection( const QRectF& section );

    /** Returns the parent multiframe for the frame.
     * @returns parent multiframe
     */
    QgsComposerMultiFrame* multiFrame() const;

    //Overridden to allow multiframe to set display name
    virtual QString displayName() const;

    //Overridden to handle fixed frame sizes set by multi frame
    void setSceneRect( const QRectF& rectangle );

    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget );
    void beginItemCommand( const QString& text );
    void endItemCommand();
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );
    int type() const;

    /** Returns the visible portion of the multi frame's content which
     * is shown in this frame.
     * @returns extent of visible portion
     * @note added in QGIS 2.5
     * @see setContentSection
     */
    QRectF extent() const;

    /** Returns whether the page should be hidden (ie, not included in composer exports) if this frame is empty
     * @returns true if page should be hidden if frame is empty
     * @note added in QGIS 2.5
     * @see setHidePageIfEmpty
     */
    bool hidePageIfEmpty() const;

    /** Sets whether the page should be hidden (ie, not included in composer exports) if this frame is empty
     * @param hidePageIfEmpty set to true if page should be hidden if frame is empty
     * @note added in QGIS 2.5
     * @see hidePageIfEmpty
     */
    void setHidePageIfEmpty( const bool hidePageIfEmpty );

    /** Returns whether the background and frame border should be hidden if this frame is empty
     * @returns true if background and border should be hidden if frame is empty
     * @note added in QGIS 2.5
     * @see setHideBackgroundIfEmpty
     */
    bool hideBackgroundIfEmpty() const;

    /** Sets whether the background and frame border should be hidden if this frame is empty
     * @param hideBackgroundIfEmpty set to true if background and border should be hidden if frame is empty
     * @note added in QGIS 2.5
     * @see hideBackgroundIfEmpty
     */
    void setHideBackgroundIfEmpty( const bool hideBackgroundIfEmpty );

    /** Returns whether the frame is empty
     * @returns true if frame is empty
     * @note added in QGIS 2.5
     * @see hidePageIfEmpty
     */
    bool isEmpty() const;

    virtual QgsExpressionContext* createExpressionContext() const;
};
