/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterresampler.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRasterResampler
{
%Docstring
Interface for resampling rasters (e.g. to have a smoother appearance)
%End

%TypeHeaderCode
#include "qgsrasterresampler.h"
#include "qgsbilinearrasterresampler.h"
#include "qgscubicrasterresampler.h"
%End
%ConvertToSubClassCode
    if ( dynamic_cast<QgsBilinearRasterResampler *>( sipCpp ) != NULL )
      sipType = sipType_QgsBilinearRasterResampler;
    else if ( dynamic_cast<QgsCubicRasterResampler *>( sipCpp ) != NULL )
      sipType = sipType_QgsCubicRasterResampler;
    else
      sipType = 0;
%End
  public:
    virtual ~QgsRasterResampler();
    virtual void resample( const QImage &srcImage, QImage &dstImage ) = 0;

    virtual QString type() const = 0;
%Docstring
Gets a descriptive type identifier for this raster resampler.
Needs to be implemented by subclasses.
%End

    virtual QgsRasterResampler *clone() const = 0 /Factory/;
%Docstring
Gets a deep copy of this object.
Needs to be reimplemented by subclasses.
Ownership is transferred to the caller.
%End
};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterresampler.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
