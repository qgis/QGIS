class QgsComposerHtml: QgsComposerMultiFrame
{
%TypeHeaderCode
#include "qgscomposerhtml.h"
%End

 public:
    QgsComposerHtml( QgsComposition* c, bool createUndoCommands );
    QgsComposerHtml();
    ~QgsComposerHtml();

    void setUrl( const QUrl& url );
    const QUrl& url() const;  

    QSizeF totalSize() const;
    void render( QPainter* p, const QRectF& renderExtent );

    bool writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames = false ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false );

    void addFrame( QgsComposerFrame* frame, bool recalcFrameSizes = true );
    
    /**Returns whether html item is using smart breaks. Smart breaks prevent
     * the html frame contents from breaking mid-way though a line of text.
     * @returns true if html item is using smart breaks
     * @see setUseSmartBreaks
     */
    bool useSmartBreaks() const;

    /**Sets whether the html item should use smart breaks. Smart breaks prevent
     * the html frame contents from breaking mid-way though a line of text.
     * @param useSmartBreaks set to true to prevent content from breaking
     * mid-way through a line of text
     * @see useSmartBreaks
     */
    void setUseSmartBreaks( bool useSmartBreaks );
    
    /**Sets the maximum distance allowed when calculating where to place page breaks
     * in the html. This distance is the maximum amount of empty space allowed
     * at the bottom of a frame after calculating the optimum break location. Setting
     * a larger value will result in better choice of page break location, but more
     * wasted space at the bottom of frames. This setting is only effective if
     * useSmartBreaks is true.
     * @param maxBreakDistance maximum amount of empty space to leave when calculating
     * page break locations
     * @note added in 2.3
     * @see maxBreakDistance
     * @see setUseSmartBreaks
     */
    void setMaxBreakDistance( double maxBreakDistance );

    /**Returns the maximum distance allowed when calculating where to place page breaks
     * in the html. This distance is the maximum amount of empty space allowed
     * at the bottom of a frame after calculating the optimum break location. This setting
     * is only effective if useSmartBreaks is true.
     * @returns maximum amount of empty space to leave when calculating page break locations
     * @note added in 2.3
     * @see setMaxBreakDistance
     * @see useSmartBreaks
     */
    double maxBreakDistance() const;
    
 public slots:
    
    /**Reloads the html source from the url and redraws the item.
     * @see setUrl
     * @see url
     */
    void loadHtml();  
};
