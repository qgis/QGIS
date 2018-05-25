/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterresamplefilter.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRasterResampleFilter : QgsRasterInterface
{
%Docstring
Resample filter pipe for rasters.
%End

%TypeHeaderCode
#include "qgsrasterresamplefilter.h"
%End
  public:
    QgsRasterResampleFilter( QgsRasterInterface *input = 0 );

    virtual QgsRasterResampleFilter *clone() const /Factory/;


    virtual int bandCount() const;


    virtual Qgis::DataType dataType( int bandNo ) const;


    virtual bool setInput( QgsRasterInterface *input );


    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = 0 ) /Factory/;


    void setZoomedInResampler( QgsRasterResampler *r /Transfer/ );
%Docstring
Sets resampler for zoomed in scales. Takes ownership of the object
%End
    const QgsRasterResampler *zoomedInResampler() const;

    void setZoomedOutResampler( QgsRasterResampler *r /Transfer/ );
%Docstring
Sets resampler for zoomed out scales. Takes ownership of the object
%End
    const QgsRasterResampler *zoomedOutResampler() const;

    void setMaxOversampling( double os );
    double maxOversampling() const;

    virtual void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;


    virtual void readXml( const QDomElement &filterElem );

%Docstring
Sets base class members from xml. Usually called from create() methods of subclasses
%End

  protected:


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterresamplefilter.h                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
