class QgsSvgCacheEntry
{
%TypeHeaderCode
#include <qgssvgcache.h>
%End

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
     */
    QgsSvgCacheEntry( const QString& file, double size, double outlineWidth, double widthScaleFactor, double rasterScaleFactor, const QColor& fill, const QColor& outline );
    ~QgsSvgCacheEntry();

    QString file;
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

    /**Don't consider image, picture, last used timestamp for comparison*/
    bool operator==( const QgsSvgCacheEntry& other ) const;
    /**Return memory usage in bytes*/
    int dataSize() const;
};

/**A cache for images / pictures derived from svg files. This class supports parameter replacement in svg files
according to the svg params specification (http://www.w3.org/TR/2009/WD-SVGParamPrimer-20090616/). Supported are
the parameters 'fill-color', 'pen-color', 'outline-width', 'stroke-width'. E.g. <circle fill="param(fill-color red)" stroke="param(pen-color black)" stroke-width="param(outline-width 1)"*/
class QgsSvgCache : QObject
{
%TypeHeaderCode
#include <qgssvgcache.h>
%End

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

    /**Tests if an svg file contains parameters for fill, outline color, outline width. If yes, possible default values are returned. If there are several
      default values in the svg file, only the first one is considered*/
    void containsParams( const QString& path, bool& hasFillParam, QColor& defaultFillColor, bool& hasOutlineParam, QColor& defaultOutlineColor, bool& hasOutlineWidthParam,
                         double& defaultOutlineWidth ) const;

    /**Get image data*/
    QByteArray getImageData( const QString &path ) const;

    /**Get SVG content*/
    const QByteArray& svgContent( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                  double widthScaleFactor, double rasterScaleFactor );

  signals:
    /** Emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( const QString&  theStatusQString );

  protected:
    //! protected constructor
    QgsSvgCache( QObject * parent /TransferThis/ = 0 );

    /**Creates new cache entry and returns pointer to it
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
    /**Returns entry from cache or creates a new entry if it does not exist already*/
    QgsSvgCacheEntry* cacheEntry( const QString& file, double size, const QColor& fill, const QColor& outline, double outlineWidth,
                                  double widthScaleFactor, double rasterScaleFactor );

    /**Removes the least used items until the maximum size is under the limit*/
    void trimToMaximumSize();

    //Removes entry from the ordered list (but does not delete the entry itself)
    void takeEntryFromList( QgsSvgCacheEntry* entry );
};
