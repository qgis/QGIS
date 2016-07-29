/***************************************************************************
  qgsindexedfeature - QgsIndexFeature
  -----------------------------------

 begin                : 15.1.2016
 Copyright            : (C) 2016 Matthias Kuhn
 Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSINDEXEDFEATURE_H
#define QGSINDEXEDFEATURE_H

#include <QVector>
#include "qgsfeature.h"

/** \ingroup core
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
