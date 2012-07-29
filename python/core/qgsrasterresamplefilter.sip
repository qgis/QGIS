class QgsRasterResampleFilter
{
%TypeHeaderCode
    #include "qgsrasterresamplefilter.h"
%End

  public:
    QgsRasterResampleFilter( QgsRasterFace* input = 0 );
    ~QgsRasterResampleFilter();

    void * readBlock( int bandNo, const QgsRectangle & extent, int width, int height );

    /**Set resampler for zoomed in scales. Takes ownership of the object*/
    void setZoomedInResampler( QgsRasterResampler* r );
    const QgsRasterResampler* zoomedInResampler();

    /**Set resampler for zoomed out scales. Takes ownership of the object*/
    void setZoomedOutResampler( QgsRasterResampler* r );
    const QgsRasterResampler* zoomedOutResampler() const;

    void setMaxOversampling( double os );
    double maxOversampling() const;

    virtual void writeXML( QDomDocument& doc, QDomElement& parentElem ) const = 0;

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& rendererElem );
};

