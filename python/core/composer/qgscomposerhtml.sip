class QgsComposerHtml: QgsComposerMultiFrame
{
%TypeHeaderCode
#include "qgscomposerhtml.h"
%End

 public:

    /** Source modes for the HTML content to render in the item
    */
    enum ContentMode
    {
      Url, /*< Using this mode item fetches its content via a url*/
      ManualHtml /*< HTML content is manually set for the item*/
    };

    QgsComposerHtml( QgsComposition* c /TransferThis/, bool createUndoCommands );
    QgsComposerHtml();
    ~QgsComposerHtml();

    /**Sets the source mode for item's HTML content.
     * @param mode ContentMode for the item's source
     * @see contentMode
     * @see setUrl
     * @see setHtml
     * @note added in 2.5
     */
    void setContentMode( ContentMode mode );

    /**Returns the source mode for item's HTML content.
     * @returns ContentMode for the item's source
     * @see setContentMode
     * @see url
     * @see html
     * @note added in 2.5
     */
    ContentMode contentMode() const;

    /**Sets the URL for content to display in the item when the item is using
     * the QgsComposerHtml::Url mode. Content is automatically fetched and the
     * HTML item refreshed after calling this function.
     * @param url URL of content to display in the item
     * @see url
     * @see contentMode
     */
    void setUrl( const QUrl& url );

    /**Returns the URL of the content displayed in the item if the item is using
     * the QgsComposerHtml::Url mode.
     * @returns url for content displayed in item
     * @see setUrl
     * @see contentMode
     */
    const QUrl& url() const;

    /**Sets the HTML to display in the item when the item is using
     * the QgsComposerHtml::ManualHtml mode. Setting the HTML using this function
     * does not automatically refresh the item's contents. Call loadHtml to trigger
     * a refresh of the item after setting the HTML content.
     * @param html HTML to display in item
     * @see html
     * @see contentMode
     * @see loadHtml
     * @note added in 2.5
     */
    void setHtml( const QString html );

    /**Returns the HTML source displayed in the item if the item is using
     * the QgsComposerHtml::ManualHtml mode.
     * @returns HTML displayed in item
     * @see setHtml
     * @see contentMode
     * @note added in 2.5
     */
    QString html() const;

    /**Returns whether html item will evaluate QGIS expressions prior to rendering
     * the HTML content. If set, any content inside [% %] tags will be
     * treated as a QGIS expression and evaluated against the current atlas
     * feature.
     * @returns true if html item will evaluate expressions in the content
     * @see setEvaluateExpressions
     * @note added in QGIS 2.5
     */
    bool evaluateExpressions() const;

    /**Sets whether the html item will evaluate QGIS expressions prior to rendering
     * the HTML content. If set, any content inside [% %] tags will be
     * treated as a QGIS expression and evaluated against the current atlas
     * feature.
     * @param evaluateExpressions set to true to evaluate expressions in the HTML content
     * @see evaluateExpressions
     * @note added in QGIS 2.5
     */
    void setEvaluateExpressions( bool evaluateExpressions );

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

    /**Sets the user stylesheet CSS rules to use while rendering the HTML content. These
     * allow for overriding the styles specified within the HTML source. Setting the stylesheet
     * using this function does not automatically refresh the item's contents. Call loadHtml
     * to trigger a refresh of the item after setting the stylesheet rules.
     * @param stylesheet CSS rules for user stylesheet
     * @see userStylesheet
     * @see setUserStylesheetEnabled
     * @see loadHtml
     * @note added in 2.5
     */
    void setUserStylesheet( const QString stylesheet );

    /**Returns the user stylesheet CSS rules used while rendering the HTML content. These
     * overriding the styles specified within the HTML source.
     * @returns CSS rules for user stylesheet
     * @see setUserStylesheet
     * @see userStylesheetEnabled
     * @note added in 2.5
     */
    QString userStylesheet() const;

    /**Sets whether user stylesheets are enabled for the HTML content.
     * @param stylesheetEnabled set to true to enable user stylesheets
     * @see userStylesheetEnabled
     * @see setUserStylesheet
     * @note added in 2.5
     */
    void setUserStylesheetEnabled( const bool stylesheetEnabled );

    /**Returns whether user stylesheets are enabled for the HTML content.
     * @returns true if user stylesheets are enabled
     * @see setUserStylesheetEnabled
     * @see userStylesheet
     * @note added in 2.5
     */
    bool userStylesheetEnabled() const;

    virtual QString displayName() const;
    QSizeF totalSize() const;
    void render( QPainter* p, const QRectF& renderExtent, const int frameIndex );
    bool writeXML( QDomElement& elem, QDomDocument & doc, bool ignoreFrames = false ) const;
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc, bool ignoreFrames = false );
    void addFrame( QgsComposerFrame* frame /Transfer/, bool recalcFrameSizes = true );
    //overriden to break frames without dividing lines of text
    double findNearbyPageBreak( double yPos );

  public slots:

    /**Reloads the html source from the url and redraws the item.
     * @param useCache set to true to use a cached copy of remote html
     * content
     * @see setUrl
     * @see url
     */
    void loadHtml( const bool useCache = false );

    /**Recalculates the frame sizes for the current viewport dimensions*/
    void recalculateFrameSizes();

    void refreshExpressionContext();

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties );

};
