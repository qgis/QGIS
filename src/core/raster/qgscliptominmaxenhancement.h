/* **************************************************************************
              qgscliptominmaxenhancement.h -  description
                       -------------------
begin                : Fri Nov 16 2007
copyright            : (C) 2007 by Peter J. Ersts
email                : ersts@amnh.org

****************************************************************************/

/* **************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLIPTOMINMAXENHANCEMENT_H
#define QGSCLIPTOMINMAXENHANCEMENT_H

#include "qgis_core.h"
#include "qgscontrastenhancementfunction.h"

/**
 * \ingroup core
 * A raster contrast enhancement that will clip a value to the specified min/max range.
 * For example if a min max range of [10,240] is specified in the constructor, and
 * a value of 250 is called using enhance(), the value will be truncated ('clipped')
 * to 240.
 */
class CORE_EXPORT QgsClipToMinMaxEnhancement : public QgsContrastEnhancementFunction
{

  public:
    QgsClipToMinMaxEnhancement( Qgis::DataType, double, double );

    int enhance( double ) override;

    bool isValueInDisplayableRange( double ) override;
};

#endif
