
/**  Raster resample filter */

class QgsRasterResampleFilter : QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterresamplefilter.h>
%End

  public:
    QgsRasterResampleFilter( QgsRasterInterface *input = 0 );
    ~QgsRasterResampleFilter();

    QgsRasterInterface * clone() const /Factory/;

    int bandCount() const;

    QgsRasterInterface::DataType dataType( int bandNo ) const;

    bool setInput( QgsRasterInterface* input );

    void *readBlock( int bandNo, const QgsRectangle &extent, int width, int height );

    /**Set resampler for zoomed in scales. Takes ownership of the object*/
    void setZoomedInResampler( QgsRasterResampler* r /Transfer/ );
    const QgsRasterResampler* zoomedInResampler() const;

    /**Set resampler for zoomed out scales. Takes ownership of the object*/
    void setZoomedOutResampler( QgsRasterResampler* r /Transfer/ );
    const QgsRasterResampler* zoomedOutResampler() const;

    void setMaxOversampling( double os );
    double maxOversampling() const;

    void writeXML( QDomDocument& doc, QDomElement& parentElem );

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& resamplefilterElem );
};

