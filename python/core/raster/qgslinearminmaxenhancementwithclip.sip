/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgslinearminmaxenhancementwithclip.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLinearMinMaxEnhancementWithClip : QgsContrastEnhancementFunction
{
%Docstring
A linear enhanceContrast enhancement that first clips to min max and then enhanceContrastes
linearly between min and max.
%End

%TypeHeaderCode
#include "qgslinearminmaxenhancementwithclip.h"
%End
  public:
    QgsLinearMinMaxEnhancementWithClip( Qgis::DataType, double, double );

    virtual int enhance( double );


    virtual bool isValueInDisplayableRange( double );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgslinearminmaxenhancementwithclip.h                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
