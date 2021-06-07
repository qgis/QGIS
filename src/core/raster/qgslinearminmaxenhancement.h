/* **************************************************************************
              qgslinearminmaxenhancement.h -  description
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

#ifndef QGSLINEARMINMAXENHANCEMENT_H
#define QGSLINEARMINMAXENHANCEMENT_H

#include "qgis_core.h"
#include "qgscontrastenhancementfunction.h"

/**
 * \ingroup core
  * A color enhancement function that performs a linear enhanceContrast between min and max.
  */
class CORE_EXPORT QgsLinearMinMaxEnhancement : public QgsContrastEnhancementFunction
{

  public:
    QgsLinearMinMaxEnhancement( Qgis::DataType, double, double );

    int enhance( double ) override;

};

#endif
