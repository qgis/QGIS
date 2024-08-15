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

#include "qgis_core.h"
#include "qgsfeature.h"
#include "qgssqliteutils.h"

#include <QObject>
#include <QString>

class QgsMapLayer;
class QgsVectorLayer;

/**
 * \ingroup core
 * \brief Handles logic relating to synchronizing online and offline copies of layer data.
 */
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

    //! Type of offline database container file
    enum ContainerType
    {
      SpatiaLite,
      GPKG
    };

    QgsOfflineEditing();

    /**
     * Convert current project for offline editing
     * \param offlineDataPath Path to offline db file
     * \param offlineDbFile Offline db file name
     * \param layerIds List of layer names to convert
     * \param onlySelected Only copy selected features from layers where a selection is present
     * \param containerType defines the SQLite file container type like SpatiaLite or GPKG
     * \param layerNameSuffix Suffix string added to the offline layer name
     */
    bool convertToOfflineProject( const QString &offlineDataPath, const QString &offlineDbFile, const QStringList &layerIds, bool onlySelected = false, ContainerType containerType = SpatiaLite, const QString &layerNameSuffix = QStringLiteral( " (offline)" ) );

    //! Returns TRUE if current project is offline
    bool isOfflineProject() const;


    /**
     * Synchronize to remote layers
     * \param useTransaction enforce the remote layer modifications with the same source to be in a transaction group
     */
    void synchronize( bool useTransaction = false );

  signals:

    /**
     * Emitted when the process has started.
     */
    void progressStarted();

    /**
     * Emitted whenever a new layer is being processed.
     * It is possible to estimate the progress of the complete operation by
     * comparing the index of the current \a layer to the total amount
     * \a numLayers.
     */
    void layerProgressUpdated( int layer, int numLayers );

    /**
     * Emitted when the mode for the progress of the current operation is
     * set.
     * \param mode progress mode
     * \param maximum total number of entities to process in the current operation
     */
    void progressModeSet( QgsOfflineEditing::ProgressMode mode, long long maximum );

    /**
     * Emitted with the progress of the current mode
     * \param progress current index of processed entities
     */
    void progressUpdated( long long progress );

    //! Emitted when the processing of all layers has finished
    void progressStopped();

    /**
     * Emitted when a warning needs to be displayed.
     * \param title title string for message
     * \param message A descriptive message for the warning
     */
    void warning( const QString &title, const QString &message );

  private:
    void initializeSpatialMetadata( sqlite3 *sqlite_handle );
    bool createOfflineDb( const QString &offlineDbPath, ContainerType containerType = SpatiaLite );
    void createLoggingTables( sqlite3 *db );

    void convertToOfflineLayer( QgsVectorLayer *layer, sqlite3 *db, const QString &offlineDbPath, bool onlySelected, ContainerType containerType = SpatiaLite, const QString &layerNameSuffix = QStringLiteral( " (offline)" ) );

    void applyAttributesAdded( QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId, int commitNo );
    void applyFeaturesAdded( QgsVectorLayer *offlineLayer, QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId );
    void applyFeaturesRemoved( QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId );
    void applyAttributeValueChanges( QgsVectorLayer *offlineLayer, QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId, int commitNo );
    void applyGeometryChanges( QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId, int commitNo );
    void updateFidLookup( QgsVectorLayer *remoteLayer, sqlite3 *db, int layerId );

    /**
     * Returns the layer pk attribute index. If the pk is composite, return -1.
     */
    int getLayerPkIdx( const QgsVectorLayer *layer ) const;

    QMap<int, int> attributeLookup( QgsVectorLayer *offlineLayer, QgsVectorLayer *remoteLayer );

    void showWarning( const QString &message );

    sqlite3_database_unique_ptr openLoggingDb();
    int getOrCreateLayerId( sqlite3 *db, const QString &qgisLayerId );
    int getCommitNo( sqlite3 *db );
    void increaseCommitNo( sqlite3 *db );
    void addFidLookup( sqlite3 *db, int layerId, QgsFeatureId offlineFid, QgsFeatureId remoteFid, const QString &remotePk );
    QgsFeatureId remoteFid( sqlite3 *db, int layerId, QgsFeatureId offlineFid, QgsVectorLayer *remoteLayer );
    QgsFeatureId offlineFid( sqlite3 *db, int layerId, QgsFeatureId remoteFid );
    bool isAddedFeature( sqlite3 *db, int layerId, QgsFeatureId fid );

    int sqlExec( sqlite3 *db, const QString &sql );
    int sqlQueryInt( sqlite3 *db, const QString &sql, int defaultValue );
    QString sqlQueryStr( sqlite3 *db, const QString &sql, QString &defaultValue );
    QList<int> sqlQueryInts( sqlite3 *db, const QString &sql );
    QString sqlEscape( QString value ) const;

    QList<QgsField> sqlQueryAttributesAdded( sqlite3 *db, const QString &sql );
    QgsFeatureIds sqlQueryFeaturesRemoved( sqlite3 *db, const QString &sql );

    struct AttributeValueChange
    {
      QgsFeatureId fid;
      int attr;
      QString value;
    };
    typedef QList<AttributeValueChange> AttributeValueChanges;
    AttributeValueChanges sqlQueryAttributeValueChanges( sqlite3 *db, const QString &sql );

    struct GeometryChange
    {
      QgsFeatureId fid;
      QString geom_wkt;
    };
    typedef QList<GeometryChange> GeometryChanges;
    GeometryChanges sqlQueryGeometryChanges( sqlite3 *db, const QString &sql );

  private slots:
    void setupLayer( QgsMapLayer *layer );
    void committedAttributesAdded( const QString &qgisLayerId, const QList<QgsField> &addedAttributes );
    void committedFeaturesAdded( const QString &qgisLayerId, const QgsFeatureList &addedFeatures );
    void committedFeaturesRemoved( const QString &qgisLayerId, const QgsFeatureIds &deletedFeatureIds );
    void committedAttributeValuesChanges( const QString &qgisLayerId, const QgsChangedAttributesMap &changedAttrsMap );
    void committedGeometriesChanges( const QString &qgisLayerId, const QgsGeometryMap &changedGeometries );
    void startListenFeatureChanges();
    void stopListenFeatureChanges();
};

#endif // QGS_OFFLINE_EDITING_H
