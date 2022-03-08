/***************************************************************************
                         qgsimagecache.h
                         ---------------
    begin                : December 2018
    copyright            : (C) 2018 by Nyall Dawson
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

#ifndef QGSIMAGECACHE_H
#define QGSIMAGECACHE_H

#include "qgsabstractcontentcache.h"
#include "qgis_sip.h"
#include "qgis_core.h"

#include <QElapsedTimer>
#include <QSize>
#include <QImage>

#ifndef SIP_RUN

///@cond PRIVATE

/**
 * \ingroup core
 * \class QgsImageCacheEntry
 * \brief An entry for a QgsImageCache, representing a single raster rendered at a specific width and height.
 * \since QGIS 3.6
 */
class CORE_EXPORT QgsImageCacheEntry : public QgsAbstractContentCacheEntry
{
  public:

    /**
     * Constructor for QgsImageCacheEntry, corresponding to the specified image \a path , \a size and \a opacity.
     *
     * If \a keepAspectRatio is TRUE then the original raster aspect ratio will always be preserved
     * when resizing.
     *
     * The \a targetDpi argument is ignored if \a size is a valid size.
     *
     * The \a frameNumber argument specifies a frame number for image formats which support animations. This should be
     * set to -1 if not required.
     */
    QgsImageCacheEntry( const QString &path, QSize size, bool keepAspectRatio, double opacity, double targetDpi, int frameNumber ) ;

    //! Rendered image size
    QSize size;

    //! True if original raster aspect ratio was kept during resizing
    bool keepAspectRatio = true;

    //! Rendered image opacity
    double opacity = 1.0;

    //! Rendered, resampled image.
    QImage image;

    /**
     * TRUE if the image represents a broken/missing path.
     *
     * \since QGIS 3.14
     */
    bool isMissingImage = false;

    /**
     * Target DPI
     *
     * \since QGIS 3.22
     */
    double targetDpi = 96;

    /**
     * Frame number
     *
     * \since QGIS 3.26
     */
    int frameNumber = -1;

    /**
     * Total frame count in source image
     *
     * \since QGIS 3.26
     */
    int totalFrameCount = -1;

    int dataSize() const override;
    void dump() const override;
    bool isEqual( const QgsAbstractContentCacheEntry *other ) const override;

};

///@endcond
#endif

/**
 * \class QgsImageCache
 * \ingroup core
 * \brief A cache for images derived from raster files.
 *
 * QgsImageCache stores pre-rendered resampled versions of raster image files, allowing efficient
 * reuse without incurring the cost of resampling on every render.
 *
 * QgsImageCache is not usually directly created, but rather accessed through
 * QgsApplication::imageCache().
 *
 * \since QGIS 3.6
*/
#ifdef SIP_RUN
class CORE_EXPORT QgsImageCache : public QgsAbstractContentCacheBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsImageCache : public QgsAbstractContentCache< QgsImageCacheEntry >
{
#endif
    Q_OBJECT

  public:

    /**
     * Constructor for QgsImageCache, with the specified \a parent object.
     */
    QgsImageCache( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the specified \a path rendered as an image. If possible, a pre-existing cached
     * version of the image will be used. If not, the image is fetched and resampled to the desired
     * size, and then the result cached for subsequent lookups.
     *
     * \param path may be a local file, remote (HTTP) url, or a base 64 encoded string (with a "base64:" prefix).
     * \param size dictates the target size of the image. An invalid size indicates the
     * original raster image size (with no resampling). A size in which the width or height is
     * set to zero will have the zeroed value automatically computed when keepAspectRatio is TRUE.
     * \param keepAspectRatio if TRUE then the original raster aspect ratio will be maintained during
     * any resampling operations.
     * \param opacity dictates the opacity of the image (between 0 and 1).
     * \param fitsInCache will be set to TRUE if the resultant raster was of a sufficiently small size to store in the cache
     * \param blocking if TRUE, forces to wait for loading before returning image. The content is loaded
     * in the same thread to ensure provided the image. WARNING: the \a blocking parameter must NEVER
     * be TRUE from GUI based applications (like the main QGIS application) or crashes will result. Only for
     * use in external scripts or QGIS server.
     * \param targetDpi (since QGIS 3.22) can be used to specify an explicit DPI to render the image
     * at. This is used for some image formats (e.g. PDF) to ensure that content is rendered at the desired
     * DPI. This argument is only used when an invalid \a size argument is specified. If a valid \a size is
     * specified then the image will always be rendered at this size, regardless of the \a targetDpi.
     * \param frameNumber (since QGIS 3.26) specifies a frame number for image formats which support
     * animations. This should be set to -1 if not required.
     * \param isMissing will be set to TRUE if returned image is the "broken" image placeholder
     *
     * \returns rendered image
     */
    QImage pathAsImage( const QString &path, const QSize size, const bool keepAspectRatio, const double opacity, bool &fitsInCache SIP_OUT, bool blocking = false, double targetDpi = 96, int frameNumber = -1, bool *isMissing SIP_PYARGREMOVE = nullptr );

    /**
     * Returns the original size (in pixels) of the image at the specified \a path.
     *
     * \a path may be a local file, remote (HTTP) url, or a base 64 encoded string (with a "base64:" prefix).
     *
     * If \a path is a remote file, then an invalid size may be returned while the image is in the process
     * of being fetched.
     *
     * The \a blocking boolean forces to wait for loading before returning the original size. The content is loaded
     * in the same thread to ensure provided the original size. WARNING: the \a blocking parameter must NEVER
     * be TRUE from GUI based applications (like the main QGIS application) or crashes will result. Only for
     * use in external scripts or QGIS server.
     *
     * If the image could not be read then an invalid QSize is returned.
     */
    QSize originalSize( const QString &path, bool blocking = false ) const;

    /**
     * Returns the total frame count of the image at the specified \a path.
     *
     * \a path may be a local file, remote (HTTP) url, or a base 64 encoded string (with a "base64:" prefix).
     *
     * If \a path is a remote file, then -1 may be returned while the image is in the process
     * of being fetched.
     *
     * The \a blocking boolean forces to wait for loading before returning the frame count. The content is loaded
     * in the same thread to ensure provided the original size. WARNING: the \a blocking parameter must NEVER
     * be TRUE from GUI based applications (like the main QGIS application) or crashes will result. Only for
     * use in external scripts or QGIS server.
     *
     * If the image could not be read or is not an animated format then -1 is returned
     *
     * \since QGIS 3.26
     */
    int totalFrameCount( const QString &path, bool blocking = false );

  signals:

    /**
     * Emitted when the cache has finished retrieving an image file from a remote \a url.
     */
    void remoteImageFetched( const QString &url );

  private:

    QImage pathAsImagePrivate( const QString &path, const QSize size, const bool keepAspectRatio, const double opacity, bool &fitsInCache, bool blocking, double targetDpi, int frameNumber, bool *isMissing, int &totalFrameCount );

    QImage renderImage( const QString &path, QSize size, const bool keepAspectRatio, const double opacity, double targetDpi, int frameNumber, bool &isBroken, int &totalFrameCount, bool blocking = false ) const;

    static QImage getFrameFromReader( QImageReader &reader, int frameNumber );

    //! SVG content to be rendered if SVG file was not found.
    QByteArray mMissingSvg;

    QByteArray mFetchingSvg;

    friend class TestQgsImageCache;
};

#endif // QGSIMAGECACHE_H
