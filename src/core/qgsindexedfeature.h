/***************************************************************************
  qgsindexedfeature - QgsIndexFeature
  -----------------------------------

 begin                : 15.1.2016
 Copyright            : (C) 2016 Matthias Kuhn
 Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINDEXEDFEATURE_H
#define QGSINDEXEDFEATURE_H

#define SIP_NO_FILE

#include <QVector>
#include "qgsfeature.h"

/**
 * \ingroup core
 * Temporarily used structure to cache order by information
 * \note not available in Python bindings
 */
class QgsIndexedFeature
{
  public:
    QVector<QVariant> mIndexes;
    QgsFeature mFeature;
};


#endif // QGSINDEXEDFEATURE_H
