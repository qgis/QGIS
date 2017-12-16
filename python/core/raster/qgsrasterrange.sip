/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterrange.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





typedef QList<QgsRasterRange> QgsRasterRangeList;

class QgsRasterRange
{
%Docstring
Raster values range container. Represents range of values between min and max
including min and max value.
%End

%TypeHeaderCode
#include "qgsrasterrange.h"
%End
  public:

    QgsRasterRange();
%Docstring
 Constructor.
%End

    QgsRasterRange( double min, double max );
%Docstring
 Constructor

:param min: minimum value
:param max: max value
%End

    double min() const;
    double max() const;

    double setMin( double min );
    double setMax( double max );

    bool operator==( QgsRasterRange o ) const;


};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterrange.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
