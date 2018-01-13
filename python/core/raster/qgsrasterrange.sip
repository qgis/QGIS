/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/raster/qgsrasterrange.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





typedef QList<QgsRasterRange> QgsRasterRangeList;
%Docstring
*************************************************************************
*
This program is free software; you can redistribute it and/or modify  *
it under the terms of the GNU General Public License as published by  *
the Free Software Foundation; either version 2 of the License, or     *
(at your option) any later version.                                   *
*
**************************************************************************
%End

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
