/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgshuesaturationfilter.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsHueSaturationFilter : QgsRasterInterface
{
%Docstring
Color and saturation filter pipe for rasters.
%End

%TypeHeaderCode
#include "qgshuesaturationfilter.h"
%End
  public:

    enum GrayscaleMode
    {
      GrayscaleOff,
      GrayscaleLightness,
      GrayscaleLuminosity,
      GrayscaleAverage
    };

    QgsHueSaturationFilter( QgsRasterInterface *input = 0 );

    virtual QgsHueSaturationFilter *clone() const /Factory/;


    virtual int bandCount() const;


    virtual Qgis::DataType dataType( int bandNo ) const;


    virtual bool setInput( QgsRasterInterface *input );


    virtual QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = 0 ) /Factory/;


    void setSaturation( int saturation );
    int saturation() const;
%Docstring

:rtype: int
%End

    void setGrayscaleMode( QgsHueSaturationFilter::GrayscaleMode grayscaleMode );
    QgsHueSaturationFilter::GrayscaleMode grayscaleMode() const;
%Docstring

:rtype: QgsHueSaturationFilter.GrayscaleMode
%End

    void setColorizeOn( bool colorizeOn );
    bool colorizeOn() const;
%Docstring

:rtype: bool
%End
    void setColorizeColor( const QColor &colorizeColor );
    QColor colorizeColor() const;
%Docstring

:rtype: QColor
%End
    void setColorizeStrength( int colorizeStrength );
    int colorizeStrength() const;
%Docstring

:rtype: int
%End

    virtual void writeXml( QDomDocument &doc, QDomElement &parentElem ) const;


    virtual void readXml( const QDomElement &filterElem );

%Docstring
Sets base class members from xml. Usually called from create() methods of subclasses
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgshuesaturationfilter.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
