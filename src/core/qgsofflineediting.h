/***************************************************************************
    offline_editing.h

    Offline Editing Plugin
    a QGIS plugin
     --------------------------------------
    Date                 : 22-Jul-2010
    Copyright            : (C) 2010 by Sourcepole
    Email                : info at sourcepole.ch
 ***************************************************************************
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

class QgsMapLayer;
class QgsVectorLayer;
struct sqlite3;

class CORE_EXPORT QgsOfflineEditing : public QObject
{
    Q_OBJECT

  public:
    enum ProgressMode
    {
      CopyFeatures = 0,
      ProcessFeatures,
      AddFields,
      AddFeatures,
      RemoveFeatures,
      UpdateFeatures,
      UpdateGeometries
    };

    QgsOfflineEditing();
    ~QgsOfflineEditing();

    /** convert current project for offline editing
     * @param offlineDataPath path to offline db file
     * @param offlineDbFile offline db file name
     * @param layerIds list of layer names to convert
     */
    bool convertToOfflineProject( const QString& offlineDataPath, const QString& offlineDbFile, const QStringList& layerIds );

    /** return true if current project is offline */
    bool isOfflineProject();

    /** synchronize to remote layers */
    void synchronize();

  signals:
    /** emit a signal that processing has started */
    void progressStarted();

    /** emit a signal that the next layer of numLayers has started processing
     * @param layer current layer index
     * @param numLayers total number of layers
     */
    void layerProgressUpdated( int layer, int numLayers );

    /** emit a signal that sets the mode for the progress of the current operation
     * @param mode progress mode
     * @param maximum total number of entities to process in the current operation
     */
    void progressModeSet( QgsOfflineEditing::ProgressMode mode, int maximum );

    /** emit a signal with the progress of the current mode
     * @param progress current index of processed entities
     */
    void progressUpdated( int progress );

    /** emit a signal that processing of all layers has finished */
    void progressStopped();

  private:
    void initializeSpatialMetadata( sqlite3 *sqlite_handle );
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
    void addFidLookup( sqlite3* db, int layerId, QgsFeatureId offlineFid, QgsFeatureId remoteFid );
    QgsFeatureId remoteFid( sqlite3* db, int layerId, QgsFeatureId offlineFid );
    QgsFeatureId offlineFid( sqlite3* db, int layerId, QgsFeatureId remoteFid );
    bool isAddedFeature( sqlite3* db, int layerId, QgsFeatureId fid );

    int sqlExec( sqlite3* db, const QString& sql );
    int sqlQueryInt( sqlite3* db, const QString& sql, int defaultValue );
    QList<int> sqlQueryInts( sqlite3* db, const QString& sql );

    QList<QgsField> sqlQueryAttributesAdded( sqlite3* db, const QString& sql );
    QgsFeatureIds sqlQueryFeaturesRemoved( sqlite3* db, const QString& sql );

    struct AttributeValueChange
    {
      QgsFeatureId fid;
      int attr;
      QString value;
    };
    typedef QList<AttributeValueChange> AttributeValueChanges;
    AttributeValueChanges sqlQueryAttributeValueChanges( sqlite3* db, const QString& sql );

    struct GeometryChange
    {
      QgsFeatureId fid;
      QString geom_wkt;
    };
    typedef QList<GeometryChange> GeometryChanges;
    GeometryChanges sqlQueryGeometryChanges( sqlite3* db, const QString& sql );

  private slots:
    void layerAdded( QgsMapLayer* layer );
    void committedAttributesAdded( const QString& qgisLayerId, const QList<QgsField>& addedAttributes );
    void committedFeaturesAdded( const QString& qgisLayerId, const QgsFeatureList& addedFeatures );
    void committedFeaturesRemoved( const QString& qgisLayerId, const QgsFeatureIds& deletedFeatureIds );
    void committedAttributeValuesChanges( const QString& qgisLayerId, const QgsChangedAttributesMap& changedAttrsMap );
    void committedGeometriesChanges( const QString& qgisLayerId, const QgsGeometryMap& changedGeometries );
};

#endif // QGS_OFFLINE_EDITING_H
