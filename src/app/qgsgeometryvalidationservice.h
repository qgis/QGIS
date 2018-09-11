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

#include "qgsfeature.h"

class QgsProject;
class QgsMapLayer;
class QgsVectorLayer;
// TODO: Should be retrieved from registry!!
class QgsIsValidGeometryCheck;

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
    ~QgsGeometryValidationService();

    bool validationActive( QgsVectorLayer *layer, QgsFeatureId feature ) const;

  signals:
    void geometryCheckStarted( QgsVectorLayer *layer, QgsFeatureId fid );
    void geometryCheckCompleted( QgsVectorLayer *layer, QgsFeatureId fid, const QList<QgsGeometry::Error> &errors );

  private slots:
    void onLayersAdded( const QList<QgsMapLayer *> &layers );
    void onFeatureAdded( QgsVectorLayer *layer, QgsFeatureId fid );
    void onGeometryChanged( QgsVectorLayer *layer, QgsFeatureId fid, const QgsGeometry &geometry );
    void onFeatureDeleted( QgsVectorLayer *layer, QgsFeatureId fid );

  private:
    void cancelChecks( QgsVectorLayer *layer, QgsFeatureId fid );

    void processFeature( QgsVectorLayer *layer, QgsFeatureId fid );

    QgsIsValidGeometryCheck *mIsValidGeometryCheck;

    QgsProject *mProject;

    QMap<QgsVectorLayer *, bool> mActiveChecks;
};

#endif // QGSGEOMETRYVALIDATIONSERVICE_H
