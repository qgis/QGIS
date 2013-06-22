class QgsHueSaturationFilter : QgsRasterInterface
{
%TypeHeaderCode
#include <qgshuesaturationfilter.h>
%End
  public:

    // Available modes for converting a raster to grayscale
    enum GrayscaleMode
    {
      GrayscaleOff,
      GrayscaleLightness,
      GrayscaleLuminosity,
      GrayscaleAverage
    };

    QgsHueSaturationFilter( QgsRasterInterface *input = 0 );
    ~QgsHueSaturationFilter();

    QgsRasterInterface * clone() const /Factory/;

    int bandCount() const;

    QGis::DataType dataType( int bandNo ) const;

    bool setInput( QgsRasterInterface* input );

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height );

    void setSaturation( int saturation );
    int saturation() const;

    void setGrayscaleMode( QgsHueSaturationFilter::GrayscaleMode grayscaleMode );
    QgsHueSaturationFilter::GrayscaleMode grayscaleMode() const;

    void setColorizeOn( bool colorizeOn );
    bool colorizeOn() const;
    void setColorizeColor( QColor colorizeColor );
    QColor colorizeColor() const;
    void setColorizeStrength( int colorizeStrength );
    int colorizeStrength() const;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& filterElem );
};
