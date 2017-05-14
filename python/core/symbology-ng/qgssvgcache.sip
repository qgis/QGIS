class QgsSvgCacheEntry
{
%TypeHeaderCode
#include <qgssvgcache.h>
%End

  public:
    QgsSvgCacheEntry();
    QgsSvgCacheEntry( const QString& path, double size, double strokeWidth, double widthScaleFactor, const QColor& fill, const QColor& stroke );
    ~QgsSvgCacheEntry();

    //! Absolute path to SVG file
    QString path;
    double size; //size in pixels (cast to int for QImage)
    double strokeWidth;
    double widthScaleFactor;

    /** SVG viewbox size.
     * @note added in QGIS 2.14
     */
    QSizeF viewboxSize;

    QColor fill;
    QColor stroke;
    QImage *image;
    QPicture *picture;
    //content (with params replaced)
    QByteArray svgContent;

    //keep entries on a least, sorted by last access
    QgsSvgCacheEntry *nextEntry;
    QgsSvgCacheEntry *previousEntry;

    /** Don't consider image, picture, last used timestamp for comparison*/
    bool operator==( const QgsSvgCacheEntry &other ) const;
    /** Return memory usage in bytes*/
    int dataSize() const;

  private:

    QgsSvgCacheEntry( const QgsSvgCacheEntry &rh );
};

/** A cache for images / pictures derived from svg files. This class supports parameter replacement in svg files
according to the svg params specification (http://www.w3.org/TR/2009/WD-SVGParamPrimer-20090616/). Supported are
the parameters 'fill-color', 'pen-color', 'outline-width', 'stroke-width'. E.g. <circle fill="param(fill-color red)" stroke="param(pen-color black)" stroke-width="param(outline-width 1)"*/
class QgsSvgCache : QObject
{
%TypeHeaderCode
#include <qgssvgcache.h>
%End

  public:

    QgsSvgCache( QObject *parent /TransferThis/ = 0 );

    ~QgsSvgCache();

    QImage svgAsImage( const QString& path, double size, const QColor& fill, const QColor& stroke, double strokeWidth,
                              double widthScaleFactor, bool& fitsInCache );

    QPicture svgAsPicture( const QString& path, double size, const QColor& fill, const QColor& stroke, double strokeWidth,
                                  double widthScaleFactor, bool forceVectorOutput = false );

    QSizeF svgViewboxSize( const QString& path, double size, const QColor& fill, const QColor& stroke, double strokeWidth,
                           double widthScaleFactor );

    /** Tests if an svg file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
      default values in the svg file, only the first one is considered*/
    void containsParams( const QString& path, bool& hasFillParam, QColor& defaultFillColor, bool& hasStrokeParam, QColor& defaultStrokeColor, bool& hasStrokeWidthParam,
                         double &defaultStrokeWidth ) const;

    /** Tests if an svg file contains parameters for fill, stroke color, stroke width. If yes, possible default values are returned. If there are several
     * default values in the svg file, only the first one is considered.
     * @param path path to SVG file
     * @param hasFillParam will be true if fill param present in SVG
     * @param hasDefaultFillParam will be true if fill param has a default value specified
     * @param defaultFillColor will be set to default fill color specified in SVG, if present
     * @param hasFillOpacityParam will be true if fill opacity param present in SVG
     * @param hasDefaultFillOpacity will be true if fill opacity param has a default value specified
     * @param defaultFillOpacity will be set to default fill opacity specified in SVG, if present
     * @param hasStrokeParam will be true if stroke param present in SVG
     * @param hasDefaultStrokeColor will be true if stroke param has a default value specified
     * @param defaultStrokeColor will be set to default stroke color specified in SVG, if present
     * @param hasStrokeWidthParam will be true if stroke width param present in SVG
     * @param hasDefaultStrokeWidth will be true if stroke width param has a default value specified
     * @param defaultStrokeWidth will be set to default stroke width specified in SVG, if present
     * @param hasStrokeOpacityParam will be true if stroke opacity param present in SVG
     * @param hasDefaultStrokeOpacity will be true if stroke opacity param has a default value specified
     * @param defaultStrokeOpacity will be set to default stroke opacity specified in SVG, if present
     * @note available in python bindings as containsParamsV3
     * @note added in QGIS 2.14
     */
    void containsParams( const QString& path, bool& hasFillParam, bool& hasDefaultFillParam, QColor& defaultFillColor,
                         bool& hasFillOpacityParam, bool& hasDefaultFillOpacity, double& defaultFillOpacity,
                         bool& hasStrokeParam, bool& hasDefaultStrokeColor, QColor& defaultStrokeColor,
                         bool& hasStrokeWidthParam, bool& hasDefaultStrokeWidth, double& defaultStrokeWidth,
                         bool &hasStrokeOpacityParam, bool &hasDefaultStrokeOpacity, double &defaultStrokeOpacity ) const /PyName=containsParamsV3/;

    /** Get image data*/
    QByteArray getImageData( const QString &path ) const;

    /** Get SVG content*/
    QByteArray svgContent( const QString& path, double size, const QColor& fill, const QColor& stroke, double strokeWidth,
                                  double widthScaleFactor );

  signals:
    /** Emit a signal to be caught by qgisapp and display a msg on status bar */
    void statusChanged( const QString&  statusQString );

  protected:

    QgsSvgCacheEntry* insertSvg( const QString& path, double size, const QColor& fill, const QColor& stroke, double strokeWidth,
                                 double widthScaleFactor );

    void replaceParamsAndCacheSvg( QgsSvgCacheEntry *entry );
    void cacheImage( QgsSvgCacheEntry *entry );
    void cachePicture( QgsSvgCacheEntry *entry, bool forceVectorOutput = false );
    /** Returns entry from cache or creates a new entry if it does not exist already*/
    QgsSvgCacheEntry* cacheEntry( const QString& path, double size, const QColor& fill, const QColor& stroke, double strokeWidth,
                                  double widthScaleFactor );

    /** Removes the least used items until the maximum size is under the limit*/
    void trimToMaximumSize();

    //Removes entry from the ordered list (but does not delete the entry itself)
    void takeEntryFromList( QgsSvgCacheEntry *entry );

};
