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
#include <QMap>
#include <QMultiHash>
#include <QMutex>
#include <QString>
#include <QUrl>

class QDomElement;
class QImage;
class QPicture;

class CORE_EXPORT QgsSvgCacheEntry
{
  public:
    QgsSvgCacheEntry();
    /** Constructor.
     * @param file Absolute path to SVG file (relative paths are not resolved).
     * @param size
     * @param outlineWidth width of outline
     * @param widthScaleFactor width scale factor
     * @param rasterScaleFactor raster scale factor
     * @param fill color of fill
     * @param outline color of outline
     * @param lookupKey the key string used in QgsSvgCache for quick lookup of this entry (relative or absolute path)
     */
    QgsSvgCacheEntry( const QString& file, double size, double outlineWidth, double widthScaleFactor, double rasterScaleFactor, const QColor& fill, const QColor& outline, const QString& lookupKey = QString() );
    ~QgsSvgCacheEntry();

    //! Absolute path to SVG file
    QString file;
    //! Lookup key used by QgsSvgCache's hashtable (relative or absolute path). Needed for removal from the hashtable
    QString lookupKey;
    double size; //size in pixels (cast to int for QImage)
    double outlineWidth;
    double widthScaleFactor;
    double rasterScaleFactor;
    QColor fill;
    QColor outline;
    QImage* image;
    QPicture* picture;
    //content (with params replaced)
    QByteArray svgContent;

    //keep entries on a least, sorted by last access
    QgsSvgCacheEntry* nextEntry;
    QgsSvgCacheEntry* previousEntry;

    /** Don't consider image, picture, last used timestamp for comparison*/
    bool operator==( const QgsSvgCacheEntry& other ) const;
    /** Return memory usage in bytes*/
    int dataSize() const;
};

/** A cache for images / pictures derived from svg files. This class supports parameter replacement in svg files
according to the svg params specification (http://www.w3.org/TR/2009/WD-SVGParamPrimer-20090616/). Supported are
the parameters 'fill-color', 'pen-color', 'outline-width', 'stroke-width'. E.g. <circle fill="param(fill-color red)" stroke="param(pen-color black)" stroke-width="param(outline-width 1)"*/
class CORE_EXPORT QgsSvgCache : public QObject
{
    Q_OBJECT

  public:

    static QgsSvgCache* instance();
    ~QgsSvgCache();

    /** Get SVG as QImage.
     * @param file Absolute or relative path to SVG file.
     * @param size size of cached image
     * @param fill color of fill
     * @param outline color of outline
     * @param outlineWidth width of outline
     * @param widthScaleFactor width scale factor
     * @param rasterScaleFactor raster scale factor
     * @param fitsInCache
     */
    const QImage& svgAsImage( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                              double widthScaleFactor, double rasterScaleFactor, bool& fitsInCache );
    /** Get SVG  as QPicture&.
     * @param file Absolute or relative path to SVG file.
     * @param size size of cached image
     * @param fill color of fill
     * @param outline color of outline
     * @param outlineWidth width of outline
     * @param widthScaleFactor width scale factor
     * @param rasterScaleFactor raster scale factor
     * @param forceVectorOutput
     */
    const QPicture& svgAsPicture( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                  double widthScaleFactor, double rasterScaleFactor, bool forceVectorOutput = false );

    /** Tests if an svg file contains parameters for fill, outline color, outline width. If yes, possible default values are returned. If there are several
      default values in the svg file, only the first one is considered*/
    void containsParams( const QString& path, bool& hasFillParam, QColor& defaultFillColor, bool& hasOutlineParam, QColor& defaultOutlineColor, bool& hasOutlineWidthParam,
                         double& defaultOutlineWidth ) const;

    /** Get image data*/
    QByteArray getImageData( const QString &path ) const;

    /** Get SVG content*/
    const QByteArray& svgContent( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                  double widthScaleFactor, double rasterScaleFactor );

  signals:
    /** Emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( const QString&  theStatusQString );

  protected:
    //! protected constructor
    QgsSvgCache( QObject * parent = 0 );

    /** Creates new cache entry and returns pointer to it
     * @param file Absolute or relative path to SVG file. If the path is relative the file is searched by QgsSymbolLayerV2Utils::symbolNameToPath() in SVG paths.
    in settings svg/searchPathsForSVG
     * @param size size of cached image
     * @param fill color of fill
     * @param outline color of outline
     * @param outlineWidth width of outline
     * @param widthScaleFactor width scale factor
     * @param rasterScaleFactor raster scale factor
     */
    QgsSvgCacheEntry* insertSVG( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                 double widthScaleFactor, double rasterScaleFactor );

    void replaceParamsAndCacheSvg( QgsSvgCacheEntry* entry );
    void cacheImage( QgsSvgCacheEntry* entry );
    void cachePicture( QgsSvgCacheEntry* entry, bool forceVectorOutput = false );
    /** Returns entry from cache or creates a new entry if it does not exist already*/
    QgsSvgCacheEntry* cacheEntry( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                  double widthScaleFactor, double rasterScaleFactor );

    /** Removes the least used items until the maximum size is under the limit*/
    void trimToMaximumSize();

    //Removes entry from the ordered list (but does not delete the entry itself)
    void takeEntryFromList( QgsSvgCacheEntry* entry );

  private slots:
    void downloadProgress( qint64, qint64 );

  private:
    /** Entry pointers accessible by file name*/
    QMultiHash< QString, QgsSvgCacheEntry* > mEntryLookup;
    /** Estimated total size of all images, pictures and svgContent*/
    long mTotalSize;

    //The svg cache keeps the entries on a double connected list, moving the current entry to the front.
    //That way, removing entries for more space can start with the least used objects.
    QgsSvgCacheEntry* mLeastRecentEntry;
    QgsSvgCacheEntry* mMostRecentEntry;

    //Maximum cache size
    static const long mMaximumSize = 20000000;

    /** Replaces parameters in elements of a dom node and calls method for all child nodes*/
    void replaceElemParams( QDomElement& elem, const QColor& fill, const QColor& outline, double outlineWidth );

    void containsElemParams( const QDomElement& elem, bool& hasFillParam, QColor& defaultFill, bool& hasOutlineParam, QColor& defaultOutline,
                             bool& hasOutlineWidthParam, double& defaultOutlineWidth ) const;

    /** Release memory and remove cache entry from mEntryLookup*/
    void removeCacheEntry( QString s, QgsSvgCacheEntry* entry );

    /** For debugging*/
    void printEntryList();

    /** SVG content to be rendered if SVG file was not found. */
    QByteArray mMissingSvg;

    //! Mutex to prevent concurrent access to the class from multiple threads at once (may corrupt the entries otherwise).
    QMutex mMutex;
};

#endif // QGSSVGCACHE_H
