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

#include <QColor>
#include "qgis.h"
#include <QMap>
#include <QMultiHash>
#include <QMutex>
#include <QString>
#include <QUrl>
#include <QObject>
#include <QSizeF>

#include "qgis_core.h"

class QDomElement;
class QImage;
class QPicture;

/** \ingroup core
 * \class QgsSvgCacheEntry
 */
class CORE_EXPORT QgsSvgCacheEntry
{
  public:
    QgsSvgCacheEntry();

    /** Constructor.
     * \param path Absolute path to SVG file (relative paths are not resolved).
     * \param size
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param fill color of fill
     * \param stroke color of stroke
     */
    QgsSvgCacheEntry( const QString &path, double size, double strokeWidth, double widthScaleFactor, const QColor &fill, const QColor &stroke );
    ~QgsSvgCacheEntry();

    //! QgsSvgCacheEntry cannot be copied.
    QgsSvgCacheEntry( const QgsSvgCacheEntry &rh ) = delete;
    //! QgsSvgCacheEntry cannot be copied.
    QgsSvgCacheEntry &operator=( const QgsSvgCacheEntry &rh ) = delete;

    //! Absolute path to SVG file
    QString path;
    double size; //size in pixels (cast to int for QImage)
    double strokeWidth;
    double widthScaleFactor;

    /** SVG viewbox size.
     * \since QGIS 2.14
     */
    QSizeF viewboxSize;

    QColor fill;
    QColor stroke;
    QImage *image = nullptr;
    QPicture *picture = nullptr;
    //content (with params replaced)
    QByteArray svgContent;

    //keep entries on a least, sorted by last access
    QgsSvgCacheEntry *nextEntry = nullptr;
    QgsSvgCacheEntry *previousEntry = nullptr;

    //! Don't consider image, picture, last used timestamp for comparison
    bool operator==( const QgsSvgCacheEntry &other ) const;
    //! Return memory usage in bytes
    int dataSize() const;

  private:
#ifdef SIP_RUN
    QgsSvgCacheEntry( const QgsSvgCacheEntry &rh );
#endif

};

/** \ingroup core
 * A cache for images / pictures derived from svg files. This class supports parameter replacement in svg files
according to the svg params specification (http://www.w3.org/TR/2009/WD-SVGParamPrimer-20090616/). Supported are
the parameters 'fill-color', 'pen-color', 'outline-width', 'stroke-width'. E.g. <circle fill="param(fill-color red)" stroke="param(pen-color black)" stroke-width="param(outline-width 1)"
 *
 * QgsSvgCache is not usually directly created, but rather accessed through
 * QgsApplication::svgCache().
*/
class CORE_EXPORT QgsSvgCache : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSvgCache.
     */
    QgsSvgCache( QObject *parent SIP_TRANSFERTHIS = 0 );

    ~QgsSvgCache();

    /** Get SVG as QImage.
     * \param path Absolute path to SVG file.
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param fitsInCache
     */
    QImage svgAsImage( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                       double widthScaleFactor, bool &fitsInCache );

    /** Get SVG  as QPicture&.
     * \param path Absolute path to SVG file.
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \param forceVectorOutput
     */
    QPicture svgAsPicture( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor, bool forceVectorOutput = false );

    /** Calculates the viewbox size of a (possibly cached) SVG file.
     * \param path Absolute path to SVG file.
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     * \returns viewbox size set in SVG file
     * \since QGIS 2.14
     */
    QSizeF svgViewboxSize( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor );

    /** Tests if an svg file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
      default values in the svg file, only the first one is considered*/
    void containsParams( const QString &path, bool &hasFillParam, QColor &defaultFillColor, bool &hasStrokeParam, QColor &defaultStrokeColor, bool &hasStrokeWidthParam,
                         double &defaultStrokeWidth ) const;

    /** Tests if an svg file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
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

    //! Get image data
    QByteArray getImageData( const QString &path ) const;

    //! Get SVG content
    QByteArray svgContent( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                           double widthScaleFactor );

  signals:
    //! Emit a signal to be caught by qgisapp and display a msg on status bar
    void statusChanged( const QString  &statusQString );

  protected:

    /** Creates new cache entry and returns pointer to it
     * \param path Absolute path to SVG file
     * \param size size of cached image
     * \param fill color of fill
     * \param stroke color of stroke
     * \param strokeWidth width of stroke
     * \param widthScaleFactor width scale factor
     */
    QgsSvgCacheEntry *insertSvg( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                 double widthScaleFactor );

    void replaceParamsAndCacheSvg( QgsSvgCacheEntry *entry );
    void cacheImage( QgsSvgCacheEntry *entry );
    void cachePicture( QgsSvgCacheEntry *entry, bool forceVectorOutput = false );
    //! Returns entry from cache or creates a new entry if it does not exist already
    QgsSvgCacheEntry *cacheEntry( const QString &path, double size, const QColor &fill, const QColor &stroke, double strokeWidth,
                                  double widthScaleFactor );

    //! Removes the least used items until the maximum size is under the limit
    void trimToMaximumSize();

    //Removes entry from the ordered list (but does not delete the entry itself)
    void takeEntryFromList( QgsSvgCacheEntry *entry );

  private slots:
    void downloadProgress( qint64, qint64 );

  private:
    //! Entry pointers accessible by file name
    QMultiHash< QString, QgsSvgCacheEntry * > mEntryLookup;
    //! Estimated total size of all images, pictures and svgContent
    long mTotalSize;

    //The svg cache keeps the entries on a double connected list, moving the current entry to the front.
    //That way, removing entries for more space can start with the least used objects.
    QgsSvgCacheEntry *mLeastRecentEntry = nullptr;
    QgsSvgCacheEntry *mMostRecentEntry = nullptr;

    //! Maximum cache size
    static const long MAXIMUM_SIZE = 20000000;

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

    //! Release memory and remove cache entry from mEntryLookup
    void removeCacheEntry( const QString &s, QgsSvgCacheEntry *entry );

    //! For debugging
    void printEntryList();

    //! SVG content to be rendered if SVG file was not found.
    QByteArray mMissingSvg;

    //! Mutex to prevent concurrent access to the class from multiple threads at once (may corrupt the entries otherwise).
    QMutex mMutex;

};

#endif // QGSSVGCACHE_H
