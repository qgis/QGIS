/***************************************************************************
    offline_editing.h

    Offline Editing Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 22-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_OFFLINE_EDITING_H
#define QGS_OFFLINE_EDITING_H

#include <qgsfeature.h>
#include <qgsvectorlayer.h>

#include <QObject>
#include <QString>

class QgsLegendInterface;
class QgsMapLayer;
class QgsOfflineEditingProgressDialog;
class QgsVectorLayer;
class sqlite3;

class QgsOfflineEditing : public QObject
{
    Q_OBJECT

  public:
    QgsOfflineEditing( QgsOfflineEditingProgressDialog* progressDialog );
    ~QgsOfflineEditing();

    bool convertToOfflineProject( const QString& offlineDataPath, const QString& offlineDbFile, const QStringList& layerIds );
    bool isOfflineProject();
    void synchronize( QgsLegendInterface* legendInterface );

  private:
    bool createSpatialiteDB( const QString& offlineDbPath );
    void createLoggingTables( sqlite3* db );
    void copyVectorLayer( QgsVectorLayer* layer, sqlite3* db, const QString& offlineDbPath );

    void applyAttributesAdded( QgsVectorLayer* remoteLayer, sqlite3* db, int layerId, int commitNo );
    void applyFeaturesAdded( QgsVectorLayer* offlineLayer, QgsVectorLayer* remoteLayer, sqlite3* db, int layerId );
    void applyFeaturesRemoved( QgsVectorLayer* remoteLayer, sqlite3* db, int layerId );
    void applyAttributeValueChanges( QgsVectorLayer* offlineLayer, QgsVectorLayer* remoteLayer, sqlite3* db, int layerId, int commitNo );
    void applyGeometryChanges( QgsVectorLayer* remoteLayer, sqlite3* db, int layerId, int commitNo );
    void updateFidLookup( QgsVectorLayer* remoteLayer, sqlite3* db, int layerId );
    void copySymbology( const QgsVectorLayer* sourceLayer, QgsVectorLayer* targetLayer );
    QMap<int, int> attributeLookup( QgsVectorLayer* offlineLayer, QgsVectorLayer* remoteLayer );

    void showWarning( const QString& message );

    sqlite3* openLoggingDb();
    int getOrCreateLayerId( sqlite3* db, const QString& qgisLayerId );
    int getCommitNo( sqlite3* db );
    void increaseCommitNo( sqlite3* db );
    void addFidLookup( sqlite3* db, int layerId, int offlineFid, int remoteFid );
    int remoteFid( sqlite3* db, int layerId, int offlineFid );
    int offlineFid( sqlite3* db, int layerId, int remoteFid );
    bool isAddedFeature( sqlite3* db, int layerId, int fid );

    int sqlExec( sqlite3* db, const QString& sql );
    int sqlQueryInt( sqlite3* db, const QString& sql, int defaultValue );
    QList<int> sqlQueryInts( sqlite3* db, const QString& sql );

    QList<QgsField> sqlQueryAttributesAdded( sqlite3* db, const QString& sql );
    QgsFeatureIds sqlQueryFeaturesRemoved( sqlite3* db, const QString& sql );

    struct AttributeValueChange
    {
      int fid;
      int attr;
      QString value;
    };
    typedef QList<AttributeValueChange> AttributeValueChanges;
    AttributeValueChanges sqlQueryAttributeValueChanges( sqlite3* db, const QString& sql );

    struct GeometryChange
    {
      int fid;
      QString geom_wkt;
    };
    typedef QList<GeometryChange> GeometryChanges;
    GeometryChanges sqlQueryGeometryChanges( sqlite3* db, const QString& sql );

    QgsOfflineEditingProgressDialog* mProgressDialog;

  private slots:
    void layerAdded( QgsMapLayer* layer );
    void committedAttributesAdded( const QString& qgisLayerId, const QList<QgsField>& addedAttributes );
    void committedFeaturesAdded( const QString& qgisLayerId, const QgsFeatureList& addedFeatures );
    void committedFeaturesRemoved( const QString& qgisLayerId, const QgsFeatureIds& deletedFeatureIds );
    void committedAttributeValuesChanges( const QString& qgisLayerId, const QgsChangedAttributesMap& changedAttrsMap );
    void committedGeometriesChanges( const QString& qgisLayerId, const QgsGeometryMap& changedGeometries );
};

#endif // QGS_OFFLINE_EDITING_H
