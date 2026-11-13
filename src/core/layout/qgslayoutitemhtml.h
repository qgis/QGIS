/***************************************************************************
                              qgslayoutitemhtml.h
    ------------------------------------------------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEMHTML_H
#define QGSLAYOUTITEMHTML_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsdistancearea.h"
#include "qgsfeature.h"
#include "qgsfeedback.h"
#include "qgslayoutmultiframe.h"

#include <QFutureWatcher>
#include <QPicture>
#include <QUrl>

class QgsWebPage;
class QImage;
class QgsLayoutItemLabel;
class QgsVectorLayer;
class QgsNetworkContentFetcher;
class QgsPictureRasterizer;

///@cond PRIVATE
#ifndef SIP_RUN
class QgsLayoutItemHtmlRenderJob : public QObject
{
    Q_OBJECT

  public:
    QgsLayoutItemHtmlRenderJob( const QString &html, const QUrl &baseUrl, const QString &userStylesheet, const QVariantMap &featureJson, double frameWidthLayoutUnits, double unitsPerCssPx );

    void start();

    void cancelWithoutBlocking();

    void cancelAndWait();

    bool isCanceled() const;
    bool success() const { return mSuccess; }
    QString errorMessage() const { return mErrorMessage; }
    double frameWidthLayoutUnits() const { return mFrameWidthLayoutUnits; }
    QSizeF size() const { return mSize; }
    QPicture picture() const { return mPicture; }
    QImage renderedPage() const { return mRenderedPage; }

  signals:
    void finished();

  private:
    void run();

    QString mHtml;
    QUrl mBaseUrl;
    QString mUserStylesheet;
    QVariantMap mFeatureJson;
    double mFrameWidthLayoutUnits = 0.0;
    double mUnitsPerCssPx = 1.0;

    QgsFeedback mFeedback;
    QFutureWatcher< void > mWatcher;

    bool mSuccess = false;
    QString mErrorMessage;
    QSizeF mSize;
    QPicture mPicture;
    QImage mRenderedPage;
};
#endif
///@endcond PRIVATE

/**
 * \ingroup core
 * \brief A layout multiframe subclass for HTML content.
 */
class CORE_EXPORT QgsLayoutItemHtml : public QgsLayoutMultiFrame
{
    Q_OBJECT

  public:
    //! Source modes for the HTML content to render in the item
    enum ContentMode
    {
      Url,       //!< Using this mode item fetches its content via a url
      ManualHtml //!< HTML content is manually set for the item
    };

    /**
     * Constructor for QgsLayoutItemHtml, with the specified parent \a layout.
     *
     * Ownership is transferred to the layout.
     */
    QgsLayoutItemHtml( QgsLayout *layout SIP_TRANSFERTHIS );

    ~QgsLayoutItemHtml() override;

    int type() const override;
    QIcon icon() const override;

    /**
     * Returns a new QgsLayoutItemHtml for the specified parent \a layout.
     */
    static QgsLayoutItemHtml *create( QgsLayout *layout ) SIP_FACTORY;

    /**
     * Returns a new QgsLayoutItemHtml matching the content and rendering of a given \a label.
     *
     * \since QGIS 3.32
     */
    static QgsLayoutItemHtml *createFromLabel( QgsLayoutItemLabel *label ) SIP_FACTORY;

    /**
     * Sets the source \a mode for item's HTML content.
     * \see contentMode()
     * \see setUrl()
     * \see setHtml()
     */
    void setContentMode( ContentMode mode ) { mContentMode = mode; }

    /**
     * Returns the source mode for item's HTML content.
     * \see setContentMode()
     * \see url()
     * \see html()
     */
    ContentMode contentMode() const { return mContentMode; }

    /**
     * Sets the \a url for content to display in the item when the item is using
     * the QgsLayoutItemHtml::Url mode. Content is automatically fetched and the
     * HTML item refreshed after calling this function.
     * \see url()
     * \see contentMode()
     */
    void setUrl( const QUrl &url );

    /**
     * Returns the URL of the content displayed in the item if the item is using
     * the QgsLayoutItemHtml::Url mode.
     * \see setUrl()
     * \see contentMode()
     */
    QUrl url() const { return mUrl; }

    /**
     * Sets the \a html to display in the item when the item is using
     * the QgsLayoutItemHtml::ManualHtml mode. Setting the HTML using this function
     * does not automatically refresh the item's contents. Call loadHtml to trigger
     * a refresh of the item after setting the HTML content.
     * \see html()
     * \see contentMode()
     * \see loadHtml()
     */
    void setHtml( const QString &html );

    /**
     * Returns the HTML source displayed in the item if the item is using
     * the QgsLayoutItemHtml::ManualHtml mode.
     * \see setHtml()
     * \see contentMode()
     */
    QString html() const { return mHtml; }

    /**
     * Returns whether html item will evaluate QGIS expressions prior to rendering
     * the HTML content. If set, any content inside [% %] tags will be
     * treated as a QGIS expression and evaluated against the current atlas
     * feature.
     * \see setEvaluateExpressions()
     */
    bool evaluateExpressions() const { return mEvaluateExpressions; }

    /**
     * Sets whether the html item will evaluate QGIS expressions prior to rendering
     * the HTML content. If set, any content inside [% %] tags will be
     * treated as a QGIS expression and evaluated against the current atlas
     * feature.
     * \see evaluateExpressions()
     */
    void setEvaluateExpressions( bool evaluateExpressions );

    /**
     * Returns whether html item is using smart breaks. Smart breaks prevent
     * the html frame contents from breaking mid-way though a line of text.
     * \see setUseSmartBreaks()
     */
    bool useSmartBreaks() const { return mUseSmartBreaks; }

    /**
     * Sets whether the html item should use smart breaks. Smart breaks prevent
     * the html frame contents from breaking mid-way though a line of text.
     * \see useSmartBreaks()
     */
    void setUseSmartBreaks( bool useSmartBreaks );

    /**
     * Sets the maximum \a distance allowed when calculating where to place page breaks
     * in the html. This distance is the maximum amount of empty space allowed
     * at the bottom of a frame after calculating the optimum break location. Setting
     * a larger value will result in better choice of page break location, but more
     * wasted space at the bottom of frames. This setting is only effective if
     * useSmartBreaks is TRUE.
     * \see maxBreakDistance()
     * \see setUseSmartBreaks()
     */
    void setMaxBreakDistance( double distance );

    /**
     * Returns the maximum distance allowed when calculating where to place page breaks
     * in the html. This distance is the maximum amount of empty space allowed
     * at the bottom of a frame after calculating the optimum break location. This setting
     * is only effective if useSmartBreaks is TRUE.
     * \see setMaxBreakDistance()
     * \see useSmartBreaks()
     */
    double maxBreakDistance() const { return mMaxBreakDistance; }

    /**
     * Sets the user \a stylesheet CSS rules to use while rendering the HTML content. These
     * allow for overriding the styles specified within the HTML source. Setting the stylesheet
     * using this function does not automatically refresh the item's contents. Call loadHtml
     * to trigger a refresh of the item after setting the stylesheet rules.
     * \see userStylesheet()
     * \see setUserStylesheetEnabled()
     * \see loadHtml()
     */
    void setUserStylesheet( const QString &stylesheet );

    /**
     * Returns the user stylesheet CSS rules used while rendering the HTML content. These
     * overriding the styles specified within the HTML source.
     * \see setUserStylesheet()
     * \see userStylesheetEnabled()
     */
    QString userStylesheet() const { return mUserStylesheet; }

    /**
     * Sets whether user stylesheets are \a enabled for the HTML content.
     * \see userStylesheetEnabled()
     * \see setUserStylesheet()
     */
    void setUserStylesheetEnabled( bool enabled );

    /**
     * Returns whether user stylesheets are enabled for the HTML content.
     * \see setUserStylesheetEnabled()
     * \see userStylesheet()
     */
    bool userStylesheetEnabled() const { return mEnableUserStylesheet; }

    QString displayName() const override;
    QSizeF totalSize() const override;
    void render( QgsLayoutItemRenderContext &context, const QRectF &renderExtent, int frameIndex ) override;

    //overridden to break frames without dividing lines of text
    double findNearbyPageBreak( double yPos ) override;

  public slots:

    /**
     * Reloads the html source from the url and redraws the item.
     * \param useCache set to TRUE to use a cached copy of remote html
     * content
     * \param context expression context for evaluating data defined urls and expressions in html
     * \see setUrl
     * \see url
     */
    void loadHtml( bool useCache = false, const QgsExpressionContext *context = nullptr );

    //! Recalculates the frame sizes for the current viewport dimensions
    void recalculateFrameSizes() override;

    void refreshDataDefinedProperty( QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::DataDefinedProperty::AllProperties ) override;

    /**
     * Invalidates cached renders.
     *
     * \since QGIS 4.4
     */
    void invalidateCache();

  signals:

    /**
     * Emitted whenever the item's preview has been refreshed.
     *
     * \since QGIS 4.4
     */
    void previewRefreshed();

  protected:
    bool writePropertiesToElement( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool renderInProgress() const override;

  private slots:
    void onHtmlFetched();
    void recreateCachedImageInBackground();
    void htmlRenderFinished();

  private:
    ContentMode mContentMode = QgsLayoutItemHtml::Url;
    QUrl mUrl;
    QString mHtml;
    QString mFetchedHtml;
    bool mHasFetchedHtml = false;
    QString mLastFetchedUrl;
    QString mActualFetchedUrl; //may be different if page was redirected
    QSizeF mSize;              //total size in layout units

    QPicture mRenderedPicture;
    QImage mRenderedPage;

    bool mEvaluateExpressions = true;
    bool mUseSmartBreaks = true;
    double mMaxBreakDistance = 10.0;

    QgsFeature mExpressionFeature;
    QgsVectorLayer *mExpressionLayer = nullptr;
    QgsDistanceArea mDistanceArea;

    QString mUserStylesheet;
    bool mEnableUserStylesheet = false;

    //! JSON string representation of current atlas feature
    QVariantMap mAtlasFeatureJSON;

    QgsNetworkContentFetcher *mFetcher = nullptr;
    bool mFetching = false;
    QString mPendingFetchUrl;
    std::optional< QgsExpressionContext > mPendingExpressionContext;

    QString mEvaluatedHtml;
    QUrl mResolvedBaseUrl;
    double mLastRenderedWidth = -1.0;
    bool mCacheInvalidated = true;
    QTimer *mBackgroundUpdateTimer = nullptr;
    std::unique_ptr< QgsLayoutItemHtmlRenderJob > mRenderJob;
    QgsPictureRasterizer *mPictureRasterizer = nullptr;

    double layoutUnitsPerCssPixel() const; //calculate scale factor

    void startFetchHtml( const QUrl &url );
    void cancelPendingFetch();
    void waitForPendingFetch();
    void finalizeLoadedHtml( const QString &rawHtml, const QUrl &baseUrl, const QgsExpressionContext *context );

    void renderHtmlSynchronously();
    void cancelRunningRenderJob( bool block );

    void updateFrames();

    //! Sets the current feature, the current layer and a list of local variable substitutions for evaluating expressions
    void setExpressionContext( const QgsFeature &feature, QgsVectorLayer *layer );

    //! Calculates the max width of frames in the html multiframe
    double maxFrameWidth() const;

    void refreshExpressionContext();

    QgsLayoutItemHtml( const QgsLayoutItemHtml & ) = delete;
    QgsLayoutItemHtml &operator=( const QgsLayoutItemHtml & ) = delete;
};


#endif // QGSLAYOUTITEMHTML_H
