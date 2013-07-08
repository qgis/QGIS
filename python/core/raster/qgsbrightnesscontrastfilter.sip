class QgsBrightnessContrastFilter : QgsRasterInterface
{
%TypeHeaderCode
#include <qgsbrightnesscontrastfilter.h>
%End
  public:
    QgsBrightnessContrastFilter( QgsRasterInterface *input = 0 );
    ~QgsBrightnessContrastFilter();

    QgsRasterInterface * clone() const /Factory/;

    int bandCount() const;

    QGis::DataType dataType( int bandNo ) const;

    bool setInput( QgsRasterInterface* input );

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height );

    void setBrightness( int brightness );
    int brightness() const;

    void setContrast( int contrast );
    int contrast() const;

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const;

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& filterElem );
};
