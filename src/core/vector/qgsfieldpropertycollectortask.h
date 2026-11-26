/***************************************************************************
  qgsfieldpropertycollectortask.h
  --------------------------------------
  Date                 : November 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall.dawson@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELDPROPERTYCOLLECTORTASK_H
#define QGSFIELDPROPERTYCOLLECTORTASK_H

#include "qgis.h"
#include "qgis_core.h"
#include "qgsfields.h"
#include "qgstaskmanager.h"

#define SIP_NO_FILE

class QgsVectorLayer;
class QgsVectorLayerFeatureSource;

/**
 * \class QgsFieldPropertyCollectorTask
 * \ingroup core
 *
 * \brief A task which collects properties about field data in a layer, e.g. the maximum length of strings in a field.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 4.6
 */
class CORE_EXPORT QgsFieldPropertyCollectorTask : public QgsTask
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsFieldPropertyCollectorTask for the specified \a layer.
     */
    QgsFieldPropertyCollectorTask( QgsVectorLayer *layer );
    ~QgsFieldPropertyCollectorTask() override;

    bool run() override;

    /**
     * Returns the maximum length of strings in the field with the specified \a index.
     */
    qsizetype maximumStringFieldLength( int index ) { return mMaxFieldLengths[index]; }

    /**
     * Returns the fields used for data collection.
     */
    QgsFields fields() const { return mFields; }

  private:
    std::unique_ptr<QgsVectorLayerFeatureSource> mSource;
    QgsFields mFields;
    std::vector<qsizetype> mMaxFieldLengths;
};

#endif // QGSFIELDPROPERTYCOLLECTORTASK_H
