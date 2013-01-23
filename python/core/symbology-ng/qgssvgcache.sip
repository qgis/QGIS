class QgsSvgCacheEntry
{
%TypeHeaderCode
#include <qgssvgcache.h>
%End

  public:
    QgsSvgCacheEntry();
    QgsSvgCacheEntry( const QString& file, double size, double outlineWidth, double widthScaleFactor, double rasterScaleFctor, const QColor& fill, const QColor& outline );
    ~QgsSvgCacheEntry();

    QString file;
    int size; //size in pixel
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

    const QImage& svgAsImage( const QString& file, int size, const QColor& fill, const QColor& outline, double outlineWidth,
                              double widthScaleFactor, double rasterScaleFactor, bool& fitsInCache );
    const QPicture& svgAsPicture( const QString& file, int size, const QColor& fill, const QColor& outline, double outlineWidth,
                                  double widthScaleFactor, double rasterScaleFactor );

    /**Tests if an svg file contains parameters for fill, outline color, outline width. If yes, possible default values are returned. If there are several
      default values in the svg file, only the first one is considered*/
    void containsParams( const QString& path, bool& hasFillParam, QColor& defaultFillColor, bool& hasOutlineParam, QColor& defaultOutlineColor, bool& hasOutlineWidthParam,
                         double& defaultOutlineWidth ) const;

  protected:
    //! protected constructor
    QgsSvgCache( QObject* parent = 0 );

    /**Creates new cache entry and returns pointer to it*/
    QgsSvgCacheEntry* insertSVG( const QString& file, int size, const QColor& fill, const QColor& outline, double outlineWidth,
                                 double widthScaleFactor, double rasterScaleFactor );

    void replaceParamsAndCacheSvg( QgsSvgCacheEntry* entry );
    void cacheImage( QgsSvgCacheEntry* entry );
    void cachePicture( QgsSvgCacheEntry* entry );
    /**Returns entry from cache or creates a new entry if it does not exist already*/
    QgsSvgCacheEntry* cacheEntry( const QString& file, int size, const QColor& fill, const QColor& outline, double outlineWidth,
                                  double widthScaleFactor, double rasterScaleFactor );

    /**Removes the least used items until the maximum size is under the limit*/
    void trimToMaximumSize();

    //Removes entry from the ordered list (but does not delete the entry itself)
    void takeEntryFromList( QgsSvgCacheEntry* entry );
};
