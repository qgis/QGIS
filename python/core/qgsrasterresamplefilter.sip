
/**  Raster resample filter */

class QgsRasterResampleFilter : QgsRasterInterface
{
%TypeHeaderCode
#include <qgsrasterresamplefilter.h>
%End
    QgsRasterResampleFilter();
    QgsRasterResampleFilter( const QgsRasterResampleFilter& thePipe );

    ~QgsRasterResampleFilter();

    QgsRasterInterface * clone() const;

    int bandCount() const;

    QgsRasterInterface::DataType dataType( int bandNo ) const;

    bool setInput( QgsRasterInterface* input );

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    void setZoomedInResampler( QgsRasterResampler* r );
    const QgsRasterResampler* zoomedInResampler() const;

    void setZoomedOutResampler( QgsRasterResampler* r );
    const QgsRasterResampler* zoomedOutResampler() const;

    void setMaxOversampling( double os );
    double maxOversampling() const;

    void writeXML( QDomDocument& doc, QDomElement& parentElem );

    void readXML( const QDomElement& resamplefilterElem );
};

