/***************************************************************************
                              qgssvgcache.h
                            ------------------------------
  begin                :  2011
  copyright            : (C) 2011 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSVGCACHE_H
#define QGSSVGCACHE_H

#include "qgsabstractcontentcache.h"
#include "qgis.h"

#include <QPicture>

class QDomElement;

#ifndef SIP_RUN

///@cond PRIVATE

/**
 * \ingroup core
 * \class QgsSvgCacheEntry
 */
class CORE_EXPORT QgsSvgCacheEntry : public QgsAbstractContentCacheEntry
{
  public:

    /**
     * Constructor.
     * \param path Absolute path to SVG file (relative paths are not resolved).
     * \param size
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param fill color of fill
     * \param stroke color of stroke
     * \param fixedAspectRatio fixed aspect ratio (optional)
     * \param parameters an optional map of parameters to dynamically replace content in the SVG
     */
    QgsSvgCacheEntry( const QString &path, double size, double strokeWidth, double widthScaleFactor, const QColor &fill, const QColor &stroke,
                      double fixedAspectRatio = 0, const QMap<QString, QString> &parameters = QMap<QString, QString>() ) ;

    QgsSvgCacheEntry( const QgsSvgCacheEntry &rh ) = delete;
    QgsSvgCacheEntry &operator=( const QgsSvgCacheEntry &rh ) = delete;

    double size = 0.0; //size in pixels (cast to int for QImage)
    double strokeWidth = 0;
    double widthScaleFactor = 1.0;

    //! Fixed aspect ratio
    double fixedAspectRatio = 0;

    /**
     * SVG viewbox size.
     */
    QSizeF viewboxSize;

    QColor fill = Qt::black;
    QColor stroke = Qt::black;
    QMap<QString, QString> parameters;

    std::unique_ptr< QImage > image;
    std::unique_ptr< QPicture > picture;
    //content (with params replaced)
    QByteArray svgContent;

    /**
     * TRUE if the image represents a broken/missing path.
     *
     * \since QGIS 3.14
     */
    bool isMissingImage = false;

    bool isEqual( const QgsAbstractContentCacheEntry *other ) const override;
    int dataSize() const override;
    void dump() const override;

};

///@endcond
#endif

/**
 * \ingroup core
 * \brief A cache for images / pictures derived from SVG files
 *
 * This class supports parameter replacement in SVG files according to the SVG params specification
 * (http://www.w3.org/TR/2009/WD-SVGParamPrimer-20090616/).
 *
 * Supported parameters are:
 *
 * - \a param(fill): fill color (with no opacity value)
 * - \a param(fill-opacity): fill color opacity
 * - \a param(outline): outline color (with no opacity value)
 * - \a param(outline-opacity): outline color opacity
 * - \a param(outline-width): width of outline strokes
 *
 * E.g:
 *
 *   <circle fill="param(fill-color red)" stroke="param(pen-color black)" stroke-width="param(outline-width 1)"
 *
 * \note QgsSvgCache is not usually directly created, but rather accessed through QgsApplication::svgCache().
*/
#ifdef SIP_RUN
class CORE_EXPORT QgsSvgCache : public QgsAbstractContentCacheBase // for sip we skip to the base class and avoid the template difficulty
{
#else
class CORE_EXPORT QgsSvgCache : public QgsAbstractContentCache< QgsSvgCacheEntry >
{
#endif
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSvgCache.
     */
    QgsSvgCache( QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns an SVG drawing as a QImage.
     *
     * \param path Absolute path to SVG file.
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param fitsInCache
     * \param fixedAspectRatio fixed aspect ratio (optional)
     * \param blocking forces to wait for loading before returning image (optional).
     * \param parameters is a map of parameters to dynamically replace content in SVG.
     *
     * \warning The \a blocking parameter must NEVER be TRUE from GUI based applications (like the main QGIS
     * application) or crashes will result. Only for use in external scripts or QGIS server.
     */
    QImage svgAsImage( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                       double widthScaleFactor, bool &fitsInCache, double fixedAspectRatio = 0, bool blocking = false,
                       const QMap<QString, QString> &parameters = QMap<QString, QString>() );

    /**
     * Returns an SVG drawing as a QPicture.
     *
     * \param path Absolute path to SVG file.
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param forceVectorOutput
     * \param fixedAspectRatio fixed aspect ratio (optional)
     * \param blocking forces to wait for loading before returning image (optional)
     * \param parameters is a map of parameters to dynamically replace content in SVG.
     *
     * \note The returned QPicture contains the SVG file centered over the picture origin. I.e. if it is rendered
     * using QPainter::drawPicture( QPointF( 5, 10 ), picture ) it will be drawn centered over the point (5, 10).
     * Appropriate translation to the destination painter based on the picture's boundingRect may need to be applied
     * if rendering the SVG using the top-left or other reference point is desired.
     *
     * \warning The \a blocking parameter must NEVER be TRUE from GUI based applications (like the main QGIS
     * application) or crashes will result. Only for use in external scripts or QGIS server.
     */
    QPicture svgAsPicture( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, bool forceVectorOutput = false, double fixedAspectRatio = 0, bool blocking = false,
                           const QMap<QString, QString> &parameters = QMap<QString, QString>() );

    /**
     * Calculates the viewbox size of a (possibly cached) SVG file.
     * \param path Absolute path to SVG file.
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param fixedAspectRatio fixed aspect ratio (optional)
     * \param blocking forces to wait for loading before returning image (optional).
     * \param parameters is a map of parameters to dynamically replace content in SVG.
     * \returns viewbox size set in SVG file
     *
     * \warning The blocking parameter must NEVER be TRUE from GUI based applications (like the main QGIS
     * application) or crashes will result. Only for use in external scripts or QGIS server.
     *
     */
    QSizeF svgViewboxSize( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, double fixedAspectRatio = 0, bool blocking = false, const QMap<QString, QString> &parameters = QMap<QString, QString>() );

    /**
     * Tests if an SVG file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
     * default values in the SVG file, only the first one is considered. Blocking forces to wait for loading before returning image (optional). WARNING: the
     * blocking parameter must NEVER be TRUE from GUI based applications (like the main QGIS application) or crashes will result. Only for use in external
     * scripts or QGIS server.
    */
    void containsParams( const QString &path, bool &hasFillParam, QColor &defaultFillColor, bool &hasStrokeParam, QColor &defaultStrokeColor, bool &hasStrokeWidthParam,
                         double &defaultStrokeWidth, bool blocking = false ) const;

    /**
     * Tests if an SVG file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
     * default values in the SVG file, only the first one is considered.
     * \param path path to SVG file
     * \param hasFillParam will be TRUE if fill param present in SVG
     * \param hasDefaultFillParam will be TRUE if fill param has a default value specified
     * \param defaultFillColor will be set to default fill color specified in SVG, if present
     * \param hasFillOpacityParam will be TRUE if fill opacity param present in SVG
     * \param hasDefaultFillOpacity will be TRUE if fill opacity param has a default value specified
     * \param defaultFillOpacity will be set to default fill opacity specified in SVG, if present
     * \param hasStrokeParam will be TRUE if stroke param present in SVG
     * \param hasDefaultStrokeColor will be TRUE if stroke param has a default value specified
     * \param defaultStrokeColor will be set to default stroke color specified in SVG, if present
     * \param hasStrokeWidthParam will be TRUE if stroke width param present in SVG
     * \param hasDefaultStrokeWidth will be TRUE if stroke width param has a default value specified
     * \param defaultStrokeWidth will be set to default stroke width specified in SVG, if present
     * \param hasStrokeOpacityParam will be TRUE if stroke opacity param present in SVG
     * \param hasDefaultStrokeOpacity will be TRUE if stroke opacity param has a default value specified
     * \param defaultStrokeOpacity will be set to default stroke opacity specified in SVG, if present
     * \param blocking forces to wait for loading before returning image (optional).
     *
     * \note Available in Python bindings as containsParamsV3
     *
     * \warning The \a blocking parameter must NEVER be TRUE from GUI based applications (like the main QGIS
     * application) or crashes will result. Only for use in external scripts or QGIS server.
     *
     */
    void containsParams( const QString &path, bool &hasFillParam, bool &hasDefaultFillParam, QColor &defaultFillColor,
                         bool &hasFillOpacityParam, bool &hasDefaultFillOpacity, double &defaultFillOpacity,
                         bool &hasStrokeParam, bool &hasDefaultStrokeColor, QColor &defaultStrokeColor,
                         bool &hasStrokeWidthParam, bool &hasDefaultStrokeWidth, double &defaultStrokeWidth,
                         bool &hasStrokeOpacityParam, bool &hasDefaultStrokeOpacity, double &defaultStrokeOpacity,
                         bool blocking = false ) const SIP_PYNAME( containsParamsV3 );

    /**
     * Gets the SVG content corresponding to the given \a path.
     *
     * \a path may be a local file, remote (HTTP) url, or a base 64 encoded string (with a "base64:" prefix).
     *
     * The class default missingContent byte array is returned if the \a path could not be resolved or is broken. If
     * the \a path corresponds to a remote URL, then class default fetchingContent will be returned while the content
     * is in the process of being fetched.
     * The \a blocking boolean forces to wait for loading before returning result. The content is loaded
     * in the same thread to ensure provided the remote content.
     *
     * \warning The \a blocking parameter must NEVER be TRUE from GUI based applications (like the main QGIS application)
     * or crashes will result. Only for use in external scripts or QGIS server.
     */
    QByteArray getImageData( const QString &path, bool blocking = false ) const;

    /**
     * Gets the SVG content corresponding to the given \a path.
     *
     * \a path may be a local file, remote (HTTP) url, or a base 64 encoded string (with a "base64:" prefix).
     *
     * The parameters \a size, \a strokeWidth for width of stroke, \a widthScaleFactor for width scale factor,
     * \a fill for color of fill, \a stroke for color of stroke and \a fixedAspectRatio for fixed aspect ratio (optional)
     * are needed to get the entry from cache or creates a new entry if it does not exist already.
     *
     * The \a blocking boolean forces to wait for loading before returning image. The content is loaded
     * in the same thread to ensure provided the image.
     *
     * \warning The \a blocking parameter must NEVER be TRUE from GUI based applications (like the main QGIS application)
     * or crashes will result. Only for use in external scripts or QGIS server.
     */
#ifndef SIP_RUN
    QByteArray svgContent( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, double fixedAspectRatio = 0, bool blocking = false, const QMap<QString, QString> &parameters = QMap<QString, QString>(), bool *isMissingImage = nullptr );
#else
    QByteArray svgContent( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, double fixedAspectRatio = 0, bool blocking = false, const QMap<QString, QString> &parameters = QMap<QString, QString>() );
#endif

  signals:

    /**
     * Emit a signal to be caught by qgisapp and display a msg on status bar.
     * \deprecated Deprecated since QGIS 3.6 -- no longer emitted.
     */
    Q_DECL_DEPRECATED void statusChanged( const QString  &statusQString ) SIP_DEPRECATED;

    /**
     * Emitted when the cache has finished retrieving an SVG file from a remote \a url.
     * \since QGIS 3.2
     */
    void remoteSvgFetched( const QString &url );

  protected:

    bool checkReply( QNetworkReply *reply, const QString &path ) const override;

  private:

    void replaceParamsAndCacheSvg( QgsSvgCacheEntry *entry, bool blocking = false );
    void cacheImage( QgsSvgCacheEntry *entry );
    void cachePicture( QgsSvgCacheEntry *entry, bool forceVectorOutput = false );
    //! Returns entry from cache or creates a new entry if it does not exist already
    QgsSvgCacheEntry *cacheEntry( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                  double widthScaleFactor, double fixedAspectRatio = 0, const QMap<QString, QString> &parameters = QMap<QString, QString>(), bool blocking = false, bool *isMissingImage = nullptr );

    //! Replaces parameters in elements of a dom node and calls method for all child nodes
    void replaceElemParams( QDomElement &elem, const QColor &fill, const QColor &stroke, double strokeWidth, const QMap<QString, QString> &parameters );

    void containsElemParams( const QDomElement &elem,
                             bool &hasFillParam, bool &hasDefaultFill, QColor &defaultFill,
                             bool &hasFillOpacityParam, bool &hasDefaultFillOpacity, double &defaultFillOpacity,
                             bool &hasStrokeParam, bool &hasDefaultStroke, QColor &defaultStroke,
                             bool &hasStrokeWidthParam, bool &hasDefaultStrokeWidth, double &defaultStrokeWidth,
                             bool &hasStrokeOpacityParam, bool &hasDefaultStrokeOpacity, double &defaultStrokeOpacity ) const SIP_PYNAME( containsParamsV3 );

    //! Calculates scaling for rendered image sizes to SVG logical sizes
    double calcSizeScaleFactor( QgsSvgCacheEntry *entry, const QDomElement &docElem, QSizeF &viewboxSize ) const;

    /**
     * Returns the target size (in pixels) and calculates the \a viewBoxSize
     * for a cache \a entry.
     */
    QSize sizeForImage( const QgsSvgCacheEntry &entry, QSizeF &viewBoxSize, QSizeF &scaledSize ) const;

    /**
     * Returns a rendered image for a cached picture \a entry.
     */
    QImage imageFromCachedPicture( const QgsSvgCacheEntry &entry ) const;

    //! SVG content to be rendered if SVG file was not found.
    QByteArray mMissingSvg;

    QByteArray mFetchingSvg;

    friend class TestQgsSvgCache;
};

#endif // QGSSVGCACHE_H
