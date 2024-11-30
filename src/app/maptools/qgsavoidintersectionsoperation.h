/***************************************************************************
    qgsavoidintersectionsoperation.h
    ---------------------
    begin                : 2023-09-20
    copyright            : (C) 2023 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAVOIDINTERSECTIONSOPERATION_H
#define QGSAVOIDINTERSECTIONSOPERATION_H

#include <QObject>

#include "qgis.h"
#include "qgis_app.h"
#include "qgsfeatureid.h"
#include "qgspoint.h"

class QgsVectorLayer;
class QgsGeometry;
class QgsMessageBar;

/**
 * \ingroup app
 * \brief Helper class to apply the avoid intersection operation on a geometry and treat resulting issues.
 * \since QGIS 3.34
*/
class APP_EXPORT QgsAvoidIntersectionsOperation : public QObject
{
    Q_OBJECT

  public:
    /**
     * Constructor
     */
    QgsAvoidIntersectionsOperation() = default;

    struct Result
    {
        Result()
          : operationResult( Qgis::GeometryOperationResult::NothingHappened ), geometryHasChanged( false ) {}

        /**
       * The result of an avoid intersection operation
       */
        Qgis::GeometryOperationResult operationResult;

        /**
       * True if the geometry has changed during the avoid intersection operation
       */
        bool geometryHasChanged;
    };

    /**
     * Apply the avoid intersection operation to the geometry \a geom associated to the feature \a fid
     * from the layer \a layer.
     * \param ignoreFeatures possibility to give a list of features where intersections should be ignored
     * \returns the operation result
     * \since QGIS 3.34
     */
    Result apply( QgsVectorLayer *layer, QgsFeatureId fid, QgsGeometry &geom, const QHash<QgsVectorLayer *, QSet<QgsFeatureId>> &ignoreFeatures = ( QHash<QgsVectorLayer *, QSet<QgsFeatureId>>() ) );

  signals:

    /**
     * emit a \a message with corresponding \a level
     */
    void messageEmitted( const QString &message, Qgis::MessageLevel = Qgis::MessageLevel::Info );
};

#endif
