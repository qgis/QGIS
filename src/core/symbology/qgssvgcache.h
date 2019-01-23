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
     */
    QgsSvgCacheEntry( const QString &path, double size, double strokeWidth, double widthScaleFactor, const QColor &fill, const QColor &stroke,
                      double fixedAspectRatio = 0 ) ;

    //! QgsSvgCacheEntry cannot be copied.
    QgsSvgCacheEntry( const QgsSvgCacheEntry &rh ) = delete;
    //! QgsSvgCacheEntry cannot be copied.
    QgsSvgCacheEntry &operator=( const QgsSvgCacheEntry &rh ) = delete;

    double size = 0.0; //size in pixels (cast to int for QImage)
    double strokeWidth = 0;
    double widthScaleFactor = 1.0;

    //! Fixed aspect ratio
    double fixedAspectRatio = 0;

    /**
     * SVG viewbox size.
     * \since QGIS 2.14
     */
    QSizeF viewboxSize;

    QColor fill = Qt::black;
    QColor stroke = Qt::black;
    std::unique_ptr< QImage > image;
    std::unique_ptr< QPicture > picture;
    //content (with params replaced)
    QByteArray svgContent;

    bool isEqual( const QgsAbstractContentCacheEntry *other ) const override;
    int dataSize() const override;
    void dump() const override;

};

///@endcond
#endif

/**
 * \ingroup core
 * A cache for images / pictures derived from svg files. This class supports parameter replacement in svg files
according to the svg params specification (http://www.w3.org/TR/2009/WD-SVGParamPrimer-20090616/). Supported are
the parameters 'fill-color', 'pen-color', 'outline-width', 'stroke-width'. E.g. <circle fill="param(fill-color red)" stroke="param(pen-color black)" stroke-width="param(outline-width 1)"
 *
 * QgsSvgCache is not usually directly created, but rather accessed through
 * QgsApplication::svgCache().
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
     * Gets SVG as QImage.
     * \param path Absolute path to SVG file.
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param fitsInCache
     * \param fixedAspectRatio fixed aspect ratio (optional)
     */
    QImage svgAsImage( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                       double widthScaleFactor, bool &fitsInCache, double fixedAspectRatio = 0 );

    /**
     * Gets SVG  as QPicture&.
     * \param path Absolute path to SVG file.
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param forceVectorOutput
     * \param fixedAspectRatio fixed aspect ratio (optional)
     */
    QPicture svgAsPicture( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, bool forceVectorOutput = false, double fixedAspectRatio = 0 );

    /**
     * Calculates the viewbox size of a (possibly cached) SVG file.
     * \param path Absolute path to SVG file.
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param fixedAspectRatio fixed aspect ratio (optional)
     * \returns viewbox size set in SVG file
     * \since QGIS 2.14
     */
    QSizeF svgViewboxSize( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, double fixedAspectRatio = 0 );

    /**
     * Tests if an svg file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
      default values in the svg file, only the first one is considered*/
    void containsParams( const QString &path, bool &hasFillParam, QColor &defaultFillColor, bool &hasStrokeParam, QColor &defaultStrokeColor, bool &hasStrokeWidthParam,
                         double &defaultStrokeWidth ) const;

    /**
     * Tests if an svg file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
     * default values in the svg file, only the first one is considered.
     * \param path path to SVG file
     * \param hasFillParam will be true if fill param present in SVG
     * \param hasDefaultFillParam will be true if fill param has a default value specified
     * \param defaultFillColor will be set to default fill color specified in SVG, if present
     * \param hasFillOpacityParam will be true if fill opacity param present in SVG
     * \param hasDefaultFillOpacity will be true if fill opacity param has a default value specified
     * \param defaultFillOpacity will be set to default fill opacity specified in SVG, if present
     * \param hasStrokeParam will be true if stroke param present in SVG
     * \param hasDefaultStrokeColor will be true if stroke param has a default value specified
     * \param defaultStrokeColor will be set to default stroke color specified in SVG, if present
     * \param hasStrokeWidthParam will be true if stroke width param present in SVG
     * \param hasDefaultStrokeWidth will be true if stroke width param has a default value specified
     * \param defaultStrokeWidth will be set to default stroke width specified in SVG, if present
     * \param hasStrokeOpacityParam will be true if stroke opacity param present in SVG
     * \param hasDefaultStrokeOpacity will be true if stroke opacity param has a default value specified
     * \param defaultStrokeOpacity will be set to default stroke opacity specified in SVG, if present
     * \note available in Python bindings as containsParamsV3
     * \since QGIS 2.14
     */
    void containsParams( const QString &path, bool &hasFillParam, bool &hasDefaultFillParam, QColor &defaultFillColor,
                         bool &hasFillOpacityParam, bool &hasDefaultFillOpacity, double &defaultFillOpacity,
                         bool &hasStrokeParam, bool &hasDefaultStrokeColor, QColor &defaultStrokeColor,
                         bool &hasStrokeWidthParam, bool &hasDefaultStrokeWidth, double &defaultStrokeWidth,
                         bool &hasStrokeOpacityParam, bool &hasDefaultStrokeOpacity, double &defaultStrokeOpacity ) const SIP_PYNAME( containsParamsV3 );

    //! Gets image data
    QByteArray getImageData( const QString &path ) const;

    //! Gets SVG content
    QByteArray svgContent( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, double fixedAspectRatio = 0 );

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

    void replaceParamsAndCacheSvg( QgsSvgCacheEntry *entry );
    void cacheImage( QgsSvgCacheEntry *entry );
    void cachePicture( QgsSvgCacheEntry *entry, bool forceVectorOutput = false );
    //! Returns entry from cache or creates a new entry if it does not exist already
    QgsSvgCacheEntry *cacheEntry( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                  double widthScaleFactor, double fixedAspectRatio = 0 );

    //! Replaces parameters in elements of a dom node and calls method for all child nodes
    void replaceElemParams( QDomElement &elem, const QColor &fill, const QColor &stroke, double strokeWidth );

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
