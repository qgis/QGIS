/***************************************************************************
                      qgsgeometryvalidationservice.h
                     --------------------------------------
Date                 : 7.9.2018
Copyright            : (C) 2018 by Matthias Kuhn
email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOMETRYVALIDATIONSERVICE_H
#define QGSGEOMETRYVALIDATIONSERVICE_H

#include <QObject>
#include <QMap>
#include <QFuture>
#include <QReadWriteLock>

#include "qgsfeature.h"

class QgsProject;
class QgsMapLayer;
class QgsVectorLayer;
class QgsGeometryCheck;
class QgsSingleGeometryCheck;
class QgsSingleGeometryCheckError;
class QgsGeometryCheckError;
class QgsFeedback;
class QgsFeaturePool;
struct QgsGeometryCheckContext;

/**
 * This service connects to all layers in a project and triggers validation
 * of features whenever they are edited.
 */
class QgsGeometryValidationService : public QObject
{
    Q_OBJECT

  public:
    struct FeatureError
    {
      FeatureError()
      {}
      FeatureError( QgsFeatureId fid, QgsGeometry::Error error )
        : featureId( fid )
        , error( error )
      {}
      QgsFeatureId featureId = std::numeric_limits<QgsFeatureId>::min();
      QgsGeometry::Error error;
    };

    typedef QList<FeatureError> FeatureErrors;

    QgsGeometryValidationService( QgsProject *project );
    ~QgsGeometryValidationService() = default;

    void fixError( const QgsGeometryCheckError *error, int method );

    void triggerTopologyChecks( QgsVectorLayer *layer );

  signals:
    void geometryCheckStarted( QgsVectorLayer *layer, QgsFeatureId fid );
    void geometryCheckCompleted( QgsVectorLayer *layer, QgsFeatureId fid, const QList<std::shared_ptr<QgsSingleGeometryCheckError>> &errors );
    void topologyChecksUpdated( QgsVectorLayer *layer, const QList<std::shared_ptr<QgsGeometryCheckError> > &errors );
    void topologyChecksCleared( QgsVectorLayer *layer );

    void warning( const QString &message );

  private slots:
    void onLayersAdded( const QList<QgsMapLayer *> &layers );
    void onFeatureAdded( QgsVectorLayer *layer, QgsFeatureId fid );
    void onGeometryChanged( QgsVectorLayer *layer, QgsFeatureId fid, const QgsGeometry &geometry );
    void onFeatureDeleted( QgsVectorLayer *layer, QgsFeatureId fid );
    void onBeforeCommitChanges( QgsVectorLayer *layer );

  private:
    void enableLayerChecks( QgsVectorLayer *layer );

    void cancelTopologyCheck( QgsVectorLayer *layer );

    void invalidateTopologyChecks( QgsVectorLayer *layer );

    void processFeature( QgsVectorLayer *layer, QgsFeatureId fid );

    QgsProject *mProject = nullptr;

    struct VectorLayerCheckInformation
    {
      QList< QgsSingleGeometryCheck * > singleFeatureChecks;
      QList< QgsGeometryCheck *> topologyChecks;
      QFutureWatcher<void> *topologyCheckFutureWatcher = nullptr;
      QList<QgsFeedback *> topologyCheckFeedbacks; // will be deleted when topologyCheckFutureWatcher is delteed
      QList<std::shared_ptr<QgsGeometryCheckError>> topologyCheckErrors;
      QList<QMetaObject::Connection> connections;
      QgsGeometryCheckContext *context = nullptr;
    };

    QReadWriteLock mTopologyCheckLock;
    QHash<QgsVectorLayer *, VectorLayerCheckInformation> mLayerChecks;
    QMap<QString, QgsFeaturePool *> mFeaturePools;
};

#endif // QGSGEOMETRYVALIDATIONSERVICE_H
