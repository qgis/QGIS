/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemhtml.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItemHtml: QgsLayoutMultiFrame
{
%Docstring
A layout multiframe subclass for HTML content.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemhtml.h"
%End
  public:

    enum ContentMode
    {
      Url,
      ManualHtml
    };

    QgsLayoutItemHtml( QgsLayout *layout /TransferThis/ );
%Docstring
Constructor for QgsLayoutItemHtml, with the specified parent ``layout``.

Ownership is transferred to the layout.
%End

    ~QgsLayoutItemHtml();

    virtual int type() const;

    virtual QIcon icon() const;


    static QgsLayoutItemHtml *create( QgsLayout *layout ) /Factory/;
%Docstring
Returns a new QgsLayoutItemHtml for the specified parent ``layout``.
%End

    void setContentMode( ContentMode mode );
%Docstring
Sets the source ``mode`` for item's HTML content.

.. seealso:: :py:func:`contentMode`

.. seealso:: :py:func:`setUrl`

.. seealso:: :py:func:`setHtml`
%End

    ContentMode contentMode() const;
%Docstring
Returns the source mode for item's HTML content.

.. seealso:: :py:func:`setContentMode`

.. seealso:: :py:func:`url`

.. seealso:: :py:func:`html`
%End

    void setUrl( const QUrl &url );
%Docstring
Sets the ``url`` for content to display in the item when the item is using
the QgsLayoutItemHtml.Url mode. Content is automatically fetched and the
HTML item refreshed after calling this function.

.. seealso:: :py:func:`url`

.. seealso:: :py:func:`contentMode`
%End

    QUrl url() const;
%Docstring
Returns the URL of the content displayed in the item if the item is using
the QgsLayoutItemHtml.Url mode.

.. seealso:: :py:func:`setUrl`

.. seealso:: :py:func:`contentMode`
%End

    void setHtml( const QString &html );
%Docstring
Sets the ``html`` to display in the item when the item is using
the QgsLayoutItemHtml.ManualHtml mode. Setting the HTML using this function
does not automatically refresh the item's contents. Call loadHtml to trigger
a refresh of the item after setting the HTML content.

.. seealso:: :py:func:`html`

.. seealso:: :py:func:`contentMode`

.. seealso:: :py:func:`loadHtml`
%End

    QString html() const;
%Docstring
Returns the HTML source displayed in the item if the item is using
the QgsLayoutItemHtml.ManualHtml mode.

.. seealso:: :py:func:`setHtml`

.. seealso:: :py:func:`contentMode`
%End

    bool evaluateExpressions() const;
%Docstring
Returns whether html item will evaluate QGIS expressions prior to rendering
the HTML content. If set, any content inside [% %] tags will be
treated as a QGIS expression and evaluated against the current atlas
feature.

.. seealso:: :py:func:`setEvaluateExpressions`
%End

    void setEvaluateExpressions( bool evaluateExpressions );
%Docstring
Sets whether the html item will evaluate QGIS expressions prior to rendering
the HTML content. If set, any content inside [% %] tags will be
treated as a QGIS expression and evaluated against the current atlas
feature.

.. seealso:: :py:func:`evaluateExpressions`
%End

    bool useSmartBreaks() const;
%Docstring
Returns whether html item is using smart breaks. Smart breaks prevent
the html frame contents from breaking mid-way though a line of text.

.. seealso:: :py:func:`setUseSmartBreaks`
%End

    void setUseSmartBreaks( bool useSmartBreaks );
%Docstring
Sets whether the html item should use smart breaks. Smart breaks prevent
the html frame contents from breaking mid-way though a line of text.

.. seealso:: :py:func:`useSmartBreaks`
%End

    void setMaxBreakDistance( double distance );
%Docstring
Sets the maximum ``distance`` allowed when calculating where to place page breaks
in the html. This distance is the maximum amount of empty space allowed
at the bottom of a frame after calculating the optimum break location. Setting
a larger value will result in better choice of page break location, but more
wasted space at the bottom of frames. This setting is only effective if
useSmartBreaks is true.

.. seealso:: :py:func:`maxBreakDistance`

.. seealso:: :py:func:`setUseSmartBreaks`
%End

    double maxBreakDistance() const;
%Docstring
Returns the maximum distance allowed when calculating where to place page breaks
in the html. This distance is the maximum amount of empty space allowed
at the bottom of a frame after calculating the optimum break location. This setting
is only effective if useSmartBreaks is true.

.. seealso:: :py:func:`setMaxBreakDistance`

.. seealso:: :py:func:`useSmartBreaks`
%End

    void setUserStylesheet( const QString &stylesheet );
%Docstring
Sets the user ``stylesheet`` CSS rules to use while rendering the HTML content. These
allow for overriding the styles specified within the HTML source. Setting the stylesheet
using this function does not automatically refresh the item's contents. Call loadHtml
to trigger a refresh of the item after setting the stylesheet rules.

.. seealso:: :py:func:`userStylesheet`

.. seealso:: :py:func:`setUserStylesheetEnabled`

.. seealso:: :py:func:`loadHtml`
%End

    QString userStylesheet() const;
%Docstring
Returns the user stylesheet CSS rules used while rendering the HTML content. These
overriding the styles specified within the HTML source.

.. seealso:: :py:func:`setUserStylesheet`

.. seealso:: :py:func:`userStylesheetEnabled`
%End

    void setUserStylesheetEnabled( const bool enabled );
%Docstring
Sets whether user stylesheets are ``enabled`` for the HTML content.

.. seealso:: :py:func:`userStylesheetEnabled`

.. seealso:: :py:func:`setUserStylesheet`
%End

    bool userStylesheetEnabled() const;
%Docstring
Returns whether user stylesheets are enabled for the HTML content.

.. seealso:: :py:func:`setUserStylesheetEnabled`

.. seealso:: :py:func:`userStylesheet`
%End

    virtual QString displayName() const;

    virtual QSizeF totalSize() const;

     virtual void render( QgsRenderContext &context, const QRectF &renderExtent, int frameIndex,
                 const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual double findNearbyPageBreak( double yPos );


  public slots:

    void loadHtml( const bool useCache = false, const QgsExpressionContext *context = 0 );
%Docstring
Reloads the html source from the url and redraws the item.

:param useCache: set to true to use a cached copy of remote html
content
:param context: expression context for evaluating data defined urls and expressions in html

.. seealso:: :py:func:`setUrl`

.. seealso:: :py:func:`url`
%End

    virtual void recalculateFrameSizes();

%Docstring
Recalculates the frame sizes for the current viewport dimensions
%End

    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );


  protected:

    virtual bool writePropertiesToElement( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemhtml.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
