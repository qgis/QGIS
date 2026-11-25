/***************************************************************************
                             qgspicturerasterizer.h
                             -----------------
    begin                : November 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPICTURERASTERIZER_H
#define QGSPICTURERASTERIZER_H

#include "qgis.h"
#include "qgis_core.h"

#include <QFutureWatcher>
#include <QImage>
#include <QObject>
#include <QPicture>

class QgsFeedback;

#ifndef SIP_RUN
///@cond PRIVATE
class QgsPictureRasterizerJob : public QObject
{
    Q_OBJECT

  public:
    QgsPictureRasterizerJob( const QPicture &picture, const QSize &targetSize );

    void cancelWithoutBlocking();
    void cancelAndWait();
    bool isCanceled() const;
    QSize targetSize() const { return mTargetSize; }
    QImage result() const { return mWatcher.result(); }

  signals:
    void finished();

  private:
    QSize mTargetSize;
    std::unique_ptr< QgsFeedback > mFeedback;
    QFutureWatcher< QImage > mWatcher;
};
///@endcond PRIVATE
#endif

/**
 * \class QgsPictureRasterizer
 * \ingroup core
 * \brief Rasterizes and caches a QPicture as a QImage at a requested size.
 *
 * Allows for background asynchronous regeneration of the cached image at a different size.
 *
 * This class is useful for generating pre-rendered versions of QPictures for rendering
 * on the main GUI thread, where those pictures are too expensive to draw directly.
 *
 * \since QGIS 4.4
*/
class CORE_EXPORT QgsPictureRasterizer : public QObject
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsPictureRasterizer, with the specified \a parent object.
     */
    explicit QgsPictureRasterizer( QObject *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsPictureRasterizer() override;

    /**
     * Sets the source \a picture to cache.
     *
     * \see picture()
     */
    void setPicture( const QPicture &picture );

    /**
     * Returns the current source QPicture.
     *
     * \see setPicture()
     */
    QPicture picture() const { return mPicture; }

    /**
     * Clears the source picture and cached image, canceling any in-progress background render.
     */
    void clear();

    /**
     * Immediately invalidates the current cached image.
     */
    void invalidate();

    /**
     * Immediately returns the best available QImage for the given \a targetSize.
     *
     * This method never blocks. If no cached image exists for the specified \a targetSize
     * or the cache has been invalidated then the previously cached image is returned,
     * and a background render at the desired \a targetSize will be started.
     */
    QImage image( const QSize &targetSize );

    /**
     * Returns TRUE if a background job is currently running or scheduled.
     */
    bool isRendering() const;

    /**
     * Sets the maximum width or height (in pixels) of the cached QImage.
     *
     * \see maxCacheSize()
     */
    void setMaxCacheSize( int maxSize );

    /**
     * Returns the maximum width or height (in pixels) of the cached QImage.
     *
     * \see setMaxCacheSize()
     */
    int maxCacheSize() const { return mMaxCacheSize; }

  signals:

    /**
     * Emitted when a background rasterization finishes and a new cached image is available.
     */
    void rendered();

  private slots:
    void startRenderJob();
    void onRenderJobFinished();

  private:
#ifdef SIP_RUN
    QgsPictureRasterizer( const QgsPictureRasterizer & );
#endif

    void cancelRunningJob( bool block );

    QPicture mPicture;

    QImage mCachedImage;
    QSize mTargetSize;
    bool mInvalidated = true;

    int mMaxCacheSize = 5000;

    QTimer *mUpdateTimer = nullptr;
    std::unique_ptr< QgsPictureRasterizerJob > mJob;

    QgsPictureRasterizer( const QgsPictureRasterizer & ) = delete;
    QgsPictureRasterizer &operator=( const QgsPictureRasterizer & ) = delete;
};

#endif //QGSPICTURERASTERIZER_H
