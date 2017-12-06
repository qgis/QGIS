/***************************************************************************
                         qgsprocessingmodeliterator.h
                         ----------------------------------
    begin                : December 2017
    copyright            : (C) 2017 by Arnaud Morvan
    email                : arnaud dot morvan at camptocamp dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGMODELITERATOR_H
#define QGSPROCESSINGMODELITERATOR_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgsprocessingalgorithm.h"
#include <memory>

///@cond NOT_STABLE

/**
 * class QgsProcessingModelIterator
 * \ingroup core
 * \since QGIS 3.2
 */
class CORE_EXPORT QgsProcessingModelIterator : public QgsProcessingAlgorithm
{
  public:
    QgsProcessingAlgorithm::Flags flags() const;

    /**
     * Move forward
     */
    virtual bool next() { return false; }
};

///@endcond

#endif // QGSPROCESSINGMODELOUTPUT_H
