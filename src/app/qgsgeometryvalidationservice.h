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
#include "qgsgeometrycheckcontext.h"

class QgsProject;
class QgsMapLayer;
class QgsVectorLayer;
class QgsGeometryCheck;
class QgsSingleGeometryCheck;
class QgsSingleGeometryCheckError;
class QgsGeometryCheckError;
class QgsFeedback;
class QgsFeaturePool;
class QgsMessageBar;
class QgsMessageBarItem;

/**
 * This service connects to all layers in a project and triggers validation
 * of features whenever they are edited.
 * It is responsible for executing validation checks and sending out signals
 * upon failure and success.
 * It will also make sure, that a layer can only be saved, if all errors have
 * been resolved.
 */
class QgsGeometryValidationService : public QObject
{
    Q_OBJECT

  public:
    struct FeatureError
    {
        FeatureError() = default;
        FeatureError( QgsFeatureId fid, QgsGeometry::Error error )
          : featureId( fid )
          , error( error )
        {}
        QgsFeatureId featureId = std::numeric_limits<QgsFeatureId>::min();
        QgsGeometry::Error error;
    };

    typedef QList<FeatureError> FeatureErrors;

    QgsGeometryValidationService( QgsProject *project );

    void fixError( QgsGeometryCheckError *error, int method );

    void triggerTopologyChecks( QgsVectorLayer *layer, bool stopEditing );

    void setMessageBar( QgsMessageBar *messageBar );

  signals:

    /**
     * Emitted when geometry checks for this layer have been disabled and
     * any existing cached result should be cleared.
     */
    void singleGeometryCheckCleared( QgsVectorLayer *layer );

    void geometryCheckStarted( QgsVectorLayer *layer, QgsFeatureId fid );
    void geometryCheckCompleted( QgsVectorLayer *layer, QgsFeatureId fid, const QList<std::shared_ptr<QgsSingleGeometryCheckError>> &errors );
    void topologyChecksUpdated( QgsVectorLayer *layer, const QList<std::shared_ptr<QgsGeometryCheckError>> &errors );
    void topologyChecksCleared( QgsVectorLayer *layer );
    void topologyErrorUpdated( QgsVectorLayer *layer, QgsGeometryCheckError *error );

    void warning( const QString &message );
    void clearWarning();

  private slots:
    void onLayersAdded( const QList<QgsMapLayer *> &layers );
    void onFeatureAdded( QgsVectorLayer *layer, QgsFeatureId fid );
    void onGeometryChanged( QgsVectorLayer *layer, QgsFeatureId fid, const QgsGeometry &geometry );
    void onFeatureDeleted( QgsVectorLayer *layer, QgsFeatureId fid );
    void onBeforeCommitChanges( QgsVectorLayer *layer, bool stopEditing );
    void onEditingStopped( QgsVectorLayer *layer );

  private:
    void showMessage( const QString &message );
    void cleanupLayerChecks( QgsVectorLayer *layer );
    void enableLayerChecks( QgsVectorLayer *layer );

    void cancelTopologyCheck( QgsVectorLayer *layer );

    void clearTopologyChecks( QgsVectorLayer *layer );

    void invalidateTopologyChecks( QgsVectorLayer *layer );

    void processFeature( QgsVectorLayer *layer, QgsFeatureId fid );

    QgsProject *mProject = nullptr;

    struct VectorLayerCheckInformation
    {
        QList<QgsSingleGeometryCheck *> singleFeatureChecks;
        QMap<QgsFeatureId, QList<std::shared_ptr<QgsSingleGeometryCheckError>>> singleFeatureCheckErrors;
        QList<QgsGeometryCheck *> topologyChecks;
        QFutureWatcher<void> *topologyCheckFutureWatcher = nullptr;
        QList<QgsFeedback *> topologyCheckFeedbacks; // will be deleted when topologyCheckFutureWatcher is delteed
        QList<std::shared_ptr<QgsGeometryCheckError>> topologyCheckErrors;
        QList<QMetaObject::Connection> connections;
        std::shared_ptr<QgsGeometryCheckContext> context;
        bool commitPending = false;
    };

    QReadWriteLock mTopologyCheckLock;
    QHash<QgsVectorLayer *, VectorLayerCheckInformation> mLayerChecks;
    QMap<QString, QgsFeaturePool *> mFeaturePools;
    QgsMessageBar *mMessageBar = nullptr;
    QgsMessageBarItem *mMessageBarItem = nullptr;

    // when checks do complete successfully and changes need to be saved
    // this variable is used to indicate that it's safe to bypass the checks
    bool mBypassChecks = false;
};

#endif // QGSGEOMETRYVALIDATIONSERVICE_H
