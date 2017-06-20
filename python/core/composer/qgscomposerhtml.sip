/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerhtml.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsComposerHtml: QgsComposerMultiFrame
{

%TypeHeaderCode
#include "qgscomposerhtml.h"
%End
  public:

    enum ContentMode
    {
      Url,
      ManualHtml
    };

    QgsComposerHtml( QgsComposition *c /TransferThis/, bool createUndoCommands );

    ~QgsComposerHtml();

    void setContentMode( ContentMode mode );
%Docstring
 Sets the source mode for item's HTML content.
 \param mode ContentMode for the item's source
.. seealso:: contentMode
.. seealso:: setUrl
.. seealso:: setHtml
.. versionadded:: 2.5
%End

    ContentMode contentMode() const;
%Docstring
 Returns the source mode for item's HTML content.
 :return: ContentMode for the item's source
.. seealso:: setContentMode
.. seealso:: url
.. seealso:: html
.. versionadded:: 2.5
 :rtype: ContentMode
%End

    void setUrl( const QUrl &url );
%Docstring
 Sets the URL for content to display in the item when the item is using
 the QgsComposerHtml.Url mode. Content is automatically fetched and the
 HTML item refreshed after calling this function.
 \param url URL of content to display in the item
.. seealso:: url
.. seealso:: contentMode
%End

    QUrl url() const;
%Docstring
 Returns the URL of the content displayed in the item if the item is using
 the QgsComposerHtml.Url mode.
 :return: url for content displayed in item
.. seealso:: setUrl
.. seealso:: contentMode
 :rtype: QUrl
%End

    void setHtml( const QString &html );
%Docstring
 Sets the HTML to display in the item when the item is using
 the QgsComposerHtml.ManualHtml mode. Setting the HTML using this function
 does not automatically refresh the item's contents. Call loadHtml to trigger
 a refresh of the item after setting the HTML content.
 \param html HTML to display in item
.. seealso:: html
.. seealso:: contentMode
.. seealso:: loadHtml
.. versionadded:: 2.5
%End

    QString html() const;
%Docstring
 Returns the HTML source displayed in the item if the item is using
 the QgsComposerHtml.ManualHtml mode.
 :return: HTML displayed in item
.. seealso:: setHtml
.. seealso:: contentMode
.. versionadded:: 2.5
 :rtype: str
%End

    bool evaluateExpressions() const;
%Docstring
 Returns whether html item will evaluate QGIS expressions prior to rendering
 the HTML content. If set, any content inside [% %] tags will be
 treated as a QGIS expression and evaluated against the current atlas
 feature.
 :return: true if html item will evaluate expressions in the content
.. seealso:: setEvaluateExpressions
.. versionadded:: 2.5
 :rtype: bool
%End

    void setEvaluateExpressions( bool evaluateExpressions );
%Docstring
 Sets whether the html item will evaluate QGIS expressions prior to rendering
 the HTML content. If set, any content inside [% %] tags will be
 treated as a QGIS expression and evaluated against the current atlas
 feature.
 \param evaluateExpressions set to true to evaluate expressions in the HTML content
.. seealso:: evaluateExpressions
.. versionadded:: 2.5
%End

    bool useSmartBreaks() const;
%Docstring
 Returns whether html item is using smart breaks. Smart breaks prevent
 the html frame contents from breaking mid-way though a line of text.
 :return: true if html item is using smart breaks
.. seealso:: setUseSmartBreaks
 :rtype: bool
%End

    void setUseSmartBreaks( bool useSmartBreaks );
%Docstring
 Sets whether the html item should use smart breaks. Smart breaks prevent
 the html frame contents from breaking mid-way though a line of text.
 \param useSmartBreaks set to true to prevent content from breaking
 mid-way through a line of text
.. seealso:: useSmartBreaks
%End

    void setMaxBreakDistance( double maxBreakDistance );
%Docstring
 Sets the maximum distance allowed when calculating where to place page breaks
 in the html. This distance is the maximum amount of empty space allowed
 at the bottom of a frame after calculating the optimum break location. Setting
 a larger value will result in better choice of page break location, but more
 wasted space at the bottom of frames. This setting is only effective if
 useSmartBreaks is true.
 \param maxBreakDistance maximum amount of empty space to leave when calculating
 page break locations
.. versionadded:: 2.3
.. seealso:: maxBreakDistance
.. seealso:: setUseSmartBreaks
%End

    double maxBreakDistance() const;
%Docstring
 Returns the maximum distance allowed when calculating where to place page breaks
 in the html. This distance is the maximum amount of empty space allowed
 at the bottom of a frame after calculating the optimum break location. This setting
 is only effective if useSmartBreaks is true.
 :return: maximum amount of empty space to leave when calculating page break locations
.. versionadded:: 2.3
.. seealso:: setMaxBreakDistance
.. seealso:: useSmartBreaks
 :rtype: float
%End

    void setUserStylesheet( const QString &stylesheet );
%Docstring
 Sets the user stylesheet CSS rules to use while rendering the HTML content. These
 allow for overriding the styles specified within the HTML source. Setting the stylesheet
 using this function does not automatically refresh the item's contents. Call loadHtml
 to trigger a refresh of the item after setting the stylesheet rules.
 \param stylesheet CSS rules for user stylesheet
.. seealso:: userStylesheet
.. seealso:: setUserStylesheetEnabled
.. seealso:: loadHtml
.. versionadded:: 2.5
%End

    QString userStylesheet() const;
%Docstring
 Returns the user stylesheet CSS rules used while rendering the HTML content. These
 overriding the styles specified within the HTML source.
 :return: CSS rules for user stylesheet
.. seealso:: setUserStylesheet
.. seealso:: userStylesheetEnabled
.. versionadded:: 2.5
 :rtype: str
%End

    void setUserStylesheetEnabled( const bool stylesheetEnabled );
%Docstring
 Sets whether user stylesheets are enabled for the HTML content.
 \param stylesheetEnabled set to true to enable user stylesheets
.. seealso:: userStylesheetEnabled
.. seealso:: setUserStylesheet
.. versionadded:: 2.5
%End

    bool userStylesheetEnabled() const;
%Docstring
 Returns whether user stylesheets are enabled for the HTML content.
 :return: true if user stylesheets are enabled
.. seealso:: setUserStylesheetEnabled
.. seealso:: userStylesheet
.. versionadded:: 2.5
 :rtype: bool
%End

    virtual QString displayName() const;
    virtual QSizeF totalSize() const;

    virtual void render( QPainter *p, const QRectF &renderExtent, const int frameIndex );

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc, bool ignoreFrames = false ) const;

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc, bool ignoreFrames = false );

    virtual void addFrame( QgsComposerFrame *frame /Transfer/, bool recalcFrameSizes = true );

    virtual double findNearbyPageBreak( double yPos );


  public slots:

    void loadHtml( const bool useCache = false, const QgsExpressionContext *context = 0 );
%Docstring
 Reloads the html source from the url and redraws the item.
 \param useCache set to true to use a cached copy of remote html
 content
 \param context expression context for evaluating data defined urls and expressions in html
.. seealso:: setUrl
.. seealso:: url
%End

    virtual void recalculateFrameSizes();

%Docstring
Recalculates the frame sizes for the current viewport dimensions
%End
    void refreshExpressionContext();

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = 0 );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposerhtml.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
