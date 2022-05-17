/***************************************************************************
                         qgsmeshdatasetgroupstore.h
                         ---------------------
    begin                : June 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMESHDATASETGROUPSTORE_H
#define QGSMESHDATASETGROUPSTORE_H

#define SIP_NO_FILE

#include "qgsmeshdataprovider.h"
#include "qgsmeshdataset.h"

class QgsMeshLayer;

/**
 * \ingroup core
 *
 * \brief Class that can be used to store and access extra dataset group, like memory dataset (temporary)
 * Derived from QgsMeshDatasetSourceInterface, this class has same methods as QgsMeshDataProvider to access to the datasets.
 *
 * \since QGIS 3.16
 */
class QgsMeshExtraDatasetStore: public QgsMeshDatasetSourceInterface
{
  public:

    //! Adds a dataset group, returns the index of the added dataset group
    int addDatasetGroup( QgsMeshDatasetGroup *datasetGroup );

    //! Removes the dataset group with the local \a index
    void removeDatasetGroup( int index );

    //! Returns whether if the dataset groups have temporal capabilities (a least one dataset group with more than one dataset)
    bool hasTemporalCapabilities() const;

    //! Returns the relative times of the dataset index with \a index, returned value in milliseconds
    quint64 datasetRelativeTime( QgsMeshDatasetIndex index );

    //! Returns information related to the dataset group with \a groupIndex
    QString description( int groupIndex ) const;

    //! Returns a pointer to the dataset group
    QgsMeshDatasetGroup *datasetGroup( int groupIndex ) const;

    int datasetGroupCount() const override;
    int datasetCount( int groupIndex ) const override;
    QgsMeshDatasetGroupMetadata datasetGroupMetadata( int groupIndex ) const override;
    QgsMeshDatasetMetadata datasetMetadata( QgsMeshDatasetIndex index ) const override;
    QgsMeshDatasetValue datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const override;
    QgsMeshDataBlock datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const override;
    QgsMesh3dDataBlock dataset3dValues( QgsMeshDatasetIndex index, int faceIndex, int count ) const override;
    bool isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const override;
    QgsMeshDataBlock areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const override;

    //! Not implemented, always returns false
    bool addDataset( const QString &uri ) override;

    //! Not implemented, always returns empty list
    QStringList extraDatasets() const override;

    //! Not implemented, always returns true
    bool persistDatasetGroup( const QString &outputFilePath,
                              const QString &outputDriver,
                              const QgsMeshDatasetGroupMetadata &meta,
                              const QVector<QgsMeshDataBlock> &datasetValues,
                              const QVector<QgsMeshDataBlock> &datasetActive,
                              const QVector<double> &times ) override;

    //! Not implemented, always returns true
    bool persistDatasetGroup( const QString &outputFilePath,
                              const QString &outputDriver,
                              QgsMeshDatasetSourceInterface *source,
                              int datasetGroupIndex ) override;

    //! Writes the store's information in a DOM document
    QDomElement writeXml( int groupIndex, QDomDocument &doc, const QgsReadWriteContext &context );

    //! Updates the temporal capabilities
    void updateTemporalCapabilities();

  private:
    std::vector<std::unique_ptr<QgsMeshDatasetGroup>> mGroups;
};

/**
 * \ingroup core
 *
 * \brief Class used to register and access all the dataset groups related to a mesh layer
 *
 * The registered dataset group are :
 *
 * - the ones from the data provider of the mesh layer
 * - extra dataset group that can be added, for example by the mesh calculator
 *
 * Every dataset group has a unique global index group that can be different from the native index group of the dataset group.
 * This storing class has the repsonsability to assign this unique grlobal dataset group index and to link this dataset group index with the dataset group
 *
 * All dataset values or information needed can be retrieved from a QgsMeshDatasetIndex with the group index corresponding to the global group index.
 * The native group index is not exposed and global index can be obtained with datasetGroupIndexes() that returns the list of global index available.
 * The dataset index is the same than in the native source (data provider or other dataset source)
 *
 * This class also has the responsibility to handle the dataset group tree item that contain information to display the available dataset (\see QgsMeshDatasetGroupTreeItem)
 *
 * \since QGIS 3.16
 */
class QgsMeshDatasetGroupStore: public QObject
{
    Q_OBJECT

    //! Contains a pointer to the dataset source inerface and the index on this dataset groups container
    typedef QPair<QgsMeshDatasetSourceInterface *, int> DatasetGroup;

  public:
    //! Constructor
    QgsMeshDatasetGroupStore( QgsMeshLayer *layer );

    //!  Sets the persistent mesh data provider with the path of its extra dataset
    void setPersistentProvider( QgsMeshDataProvider *provider, const QStringList &extraDatasetUri );

    //! Adds persistent datasets from a file with \a path
    bool addPersistentDatasets( const QString &path );

    /**
     * Adds a extra dataset \a group, take ownership
     *
     * \note as QgsMeshDatasetGroup doesn't support reference time,
     * the dataset group is supposed to have the same reference time than the pesristent provider
     */
    bool addDatasetGroup( QgsMeshDatasetGroup *group );

    //! Saves on a file with \a filePath the dataset groups index with \a groupIndex with the specified \a driver
    bool saveDatasetGroup( QString filePath, int groupIndex, QString driver );

    //! Resets to default state the dataset groups tree item
    void resetDatasetGroupTreeItem();

    //! Returns a pointer to the root of the dataset groups tree item
    QgsMeshDatasetGroupTreeItem *datasetGroupTreeItem() const;

    /**
     * Sets the root of the dataset groups tree item.
     *
     * The \a rootItem is cloned (ownership is not transferred).
     */
    void setDatasetGroupTreeItem( const QgsMeshDatasetGroupTreeItem *rootItem );

    //! Returns a list of all group indexes
    QList<int> datasetGroupIndexes() const;

    /**
     * Returns a list of all group indexes that are enabled
     *
     * \since QGIS 3.16.3
    */
    QList<int> enabledDatasetGroupIndexes() const;

    //! Returns the count of dataset groups
    int datasetGroupCount() const;

    //! Returns the count of extra dataset groups
    int extraDatasetGroupCount() const;

    //! Returns the total count of dataset group in the store
    int datasetCount( int groupIndex ) const;

    //! Returns the metadata of the dataset group with global \a index
    QgsMeshDatasetGroupMetadata datasetGroupMetadata( const QgsMeshDatasetIndex &index ) const;

    //! Returns the metadata of the dataset with global \a index
    QgsMeshDatasetMetadata datasetMetadata( const QgsMeshDatasetIndex &index ) const;

    //! Returns the value of the dataset with global \a index and \a valueIndex
    QgsMeshDatasetValue datasetValue( const QgsMeshDatasetIndex &index, int valueIndex ) const;

    //! Returns \a count values of the dataset with global \a index and from \a valueIndex
    QgsMeshDataBlock datasetValues( const QgsMeshDatasetIndex &index, int valueIndex, int count ) const;

    //! Returns \a count 3D values of the dataset with global \a index and from \a valueIndex
    QgsMesh3dDataBlock dataset3dValues( const QgsMeshDatasetIndex &index, int faceIndex, int count ) const;

    //! Returns whether faces are active for particular dataset
    QgsMeshDataBlock areFacesActive( const QgsMeshDatasetIndex &index, int faceIndex, int count ) const;

    //! Returns whether face is active for particular dataset
    bool isFaceActive( const QgsMeshDatasetIndex &index, int faceIndex ) const;

    //! Returns the global dataset index of the dataset int the dataset group with \a groupIndex, corresponding to the relative \a time and the check \a method
    QgsMeshDatasetIndex datasetIndexAtTime( qint64 time,
                                            int groupIndex,
                                            QgsMeshDataProviderTemporalCapabilities::MatchingTemporalDatasetMethod method ) const;

    /**
     * Returns the global dataset index of the dataset int the dataset group with \a groupIndex, that is between relative times \a time1 and \a time2
     *
     * Since QGIS 3.22
     */
    QList<QgsMeshDatasetIndex> datasetIndexInTimeInterval( qint64 time1, qint64 time2, int groupIndex ) const;

    //! Returns the relative time of the dataset from the persistent provider reference time
    qint64 datasetRelativeTime( const QgsMeshDatasetIndex &index ) const;

    //! Returns whether at lea&st one of stored dataset group is temporal
    bool hasTemporalCapabilities() const;

    //! Writes the store's information in a DOM document
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context );

    //! Reads the store's information from a DOM document
    void readXml( const QDomElement &storeElem, const QgsReadWriteContext &context );

    /**
     * Returns the global dataset group index of the dataset group with native index \a globalGroupIndex in the \a source
     * Returns -1 if the group or the source is not registered
     *
     * Since QGIS 3.22
     */
    int globalDatasetGroupIndexInSource( QgsMeshDatasetSourceInterface *source, int nativeGroupIndex ) const;

  signals:
    //! Emitted after dataset groups are added
    void datasetGroupsAdded( QList<int> indexes );

  private slots:
    void onPersistentDatasetAdded( int count );

  private:
    QgsMeshLayer *mLayer = nullptr;
    QgsMeshDataProvider *mPersistentProvider = nullptr;
    QList<int> mPersistentExtraDatasetGroupIndexes;
    std::unique_ptr<QgsMeshExtraDatasetStore> mExtraDatasets;
    QMap < int, DatasetGroup> mRegistery;
    std::unique_ptr<QgsMeshDatasetGroupTreeItem> mDatasetGroupTreeRootItem;

    void removePersistentProvider();

    DatasetGroup datasetGroup( int index ) const;
    int newIndex();

    int registerDatasetGroup( const DatasetGroup &group );
    int nativeIndexToGroupIndex( QgsMeshDatasetSourceInterface *source, int providerIndex );
    void createDatasetGroupTreeItems( const QList<int> &indexes );

    //! Erases from the where this is store, not from the store (registry and tree item), for persistent dataset group, do nothing
    void eraseDatasetGroup( const DatasetGroup &group );

    //! Erases from the extra store but not from the main store (e.g. from egistry and from tree item))
    void eraseExtraDataset( int indexInExtraStore );

    void checkDatasetConsistency( QgsMeshDatasetSourceInterface *source );
    void removeUnregisteredItemFromTree();
    void unregisterGroupNotPresentInTree();

    void syncItemToDatasetGroup( int groupIndex );
};

#endif // QGSMESHDATASETGROUPSTORE_H
