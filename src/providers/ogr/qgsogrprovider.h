/***************************************************************************
            qgsogrprovider.h Data provider for ESRI shapefile format
                    Formerly known as qgsshapefileprovider.h
begin                : Oct 29, 2003
copyright            : (C) 2003 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSOGRPROVIDER_H
#define QGSOGRPROVIDER_H

#include "QTextCodec"

#include "qgsrectangle.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgsvectorlayerexporter.h"

class QgsField;
class QgsVectorLayerExporter;

class QgsOgrFeatureIterator;

#include <gdal.h>

/**
  \class QgsOgrProvider
  \brief Data provider for ESRI shapefiles
  */
class QgsOgrProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    //! Convert a vector layer to a vector file
    static QgsVectorLayerExporter::ExportError createEmptyLayer(
      const QString &uri,
      const QgsFields &fields,
      QgsWkbTypes::Type wkbType,
      const QgsCoordinateReferenceSystem &srs,
      bool overwrite,
      QMap<int, int> *oldToNewAttrIdxMap,
      QString *errorMessage = nullptr,
      const QMap<QString, QVariant> *options = nullptr
    );

    /**
     * Constructor of the vector provider
     * \param uri  uniform resource locator (URI) for a dataset
     */
    explicit QgsOgrProvider( QString const &uri = "" );

    virtual ~QgsOgrProvider();

    virtual QgsAbstractFeatureSource *featureSource() const override;

    virtual QgsCoordinateReferenceSystem crs() const override;
    virtual QStringList subLayers() const override;
    virtual QString storageType() const override;
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    virtual QString subsetString() const override;
    virtual bool supportsSubsetString() const override { return true; }
    virtual bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;
    virtual QgsWkbTypes::Type wkbType() const override;
    virtual size_t layerCount() const;
    virtual long featureCount() const override;
    virtual QgsFields fields() const override;
    virtual QgsRectangle extent() const override;
    QVariant defaultValue( int fieldId ) const override;
    virtual void updateExtents() override;
    virtual bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = 0 ) override;
    virtual bool deleteFeatures( const QgsFeatureIds &id ) override;
    virtual bool addAttributes( const QList<QgsField> &attributes ) override;
    virtual bool deleteAttributes( const QgsAttributeIds &attributes ) override;
    virtual bool renameAttributes( const QgsFieldNameMap &renamedAttributes ) override;
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    virtual bool createSpatialIndex() override;
    virtual bool createAttributeIndex( int field ) override;
    virtual QgsVectorDataProvider::Capabilities capabilities() const override;
    virtual void setEncoding( const QString &e ) override;
    virtual bool enterUpdateMode() override;
    virtual bool leaveUpdateMode() override;
    virtual bool isSaveAndLoadStyleToDatabaseSupported() const override;
    QString fileVectorFilters() const override;
    //! Return a string containing the available database drivers
    QString databaseDrivers() const;
    //! Return a string containing the available directory drivers
    QString protocolDrivers() const;
    //! Return a string containing the available protocol drivers
    QString directoryDrivers() const;

    bool isValid() const override;
    QVariant minimumValue( int index ) const override;
    QVariant maximumValue( int index ) const override;
    virtual QSet< QVariant > uniqueValues( int index, int limit = -1 ) const override;
    virtual QStringList uniqueStringsMatching( int index, const QString &substring, int limit = -1,
        QgsFeedback *feedback = nullptr ) const override;

    QString name() const override;
    QString description() const override;
    virtual bool doesStrictFeatureTypeCheck() const override;

    //! Return OGR geometry type
    static OGRwkbGeometryType getOgrGeomType( OGRLayerH ogrLayer );

    //! Get single flatten geometry type
    static OGRwkbGeometryType ogrWkbSingleFlatten( OGRwkbGeometryType type );

    QString layerName() const { return mLayerName; }

    QString filePath() const { return mFilePath; }

    int layerIndex() const { return mLayerIndex; }

    QByteArray quotedIdentifier( const QByteArray &field ) const;

    /**
     * A forced reload invalidates the underlying connection.
     * E.g. in case a shapefile is replaced, the old file will be closed
     * and the new file will be opened.
     */
    void forceReload() override;
    void reloadData() override;

  protected:
    //! Loads fields from input file to member attributeFields
    void loadFields();

    //! Find out the number of features of the whole layer
    void recalculateFeatureCount();

    //! Tell OGR, which fields to fetch in nextFeature/featureAtId (ie. which not to ignore)
    void setRelevantFields( OGRLayerH ogrLayer, bool fetchGeometry, const QgsAttributeList &fetchAttributes );

    //! Convert a QgsField to work with OGR
    static bool convertField( QgsField &field, const QTextCodec &encoding );

    //! Clean shapefile from features which are marked as deleted
    void repack();

    //! Invalidate extent and optionally force its low level recomputation
    void invalidateCachedExtent( bool bForceRecomputeExtent );

    enum OpenMode
    {
      OpenModeInitial,
      OpenModeSameAsCurrent,
      OpenModeForceReadOnly,
      OpenModeForceUpdate,
    };

    void open( OpenMode mode );
    void close();

  private:
    unsigned char *getGeometryPointer( OGRFeatureH fet );
    QString ogrWkbGeometryTypeName( OGRwkbGeometryType type ) const;

    //! Starts a transaction if possible and return true in that case
    bool startTransaction();

    //! Commits a transaction
    bool commitTransaction();

    QgsFields mAttributeFields;

    //! Map of field index to default value
    QMap<int, QString> mDefaultValues;

    bool mFirstFieldIsFid;
    GDALDatasetH mGDALDataset;
    mutable OGREnvelope *mExtent;
    bool mForceRecomputeExtent;

    /** This member variable receives the same value as extent_
     in the method QgsOgrProvider::extent(). The purpose is to prevent a memory leak*/
    mutable QgsRectangle mExtentRect;
    OGRLayerH ogrLayer;
    OGRLayerH ogrOrigLayer;

    //! path to filename
    QString mFilePath;

    //! layer name
    QString mLayerName;

    //! layer index
    int mLayerIndex;

    //! was a sub layer requested?
    bool mIsSubLayer;

    /** Optional geometry type for layers with multiple geometries,
     *  otherwise wkbUnknown. This type is always flatten (2D) and single, it means
     *  that 2D, 25D, single and multi types are mixed in one sublayer */
    OGRwkbGeometryType mOgrGeometryTypeFilter;

    //! current spatial filter
    QgsRectangle mFetchRect;

    //! String used to define a subset of the layer
    QString mSubsetString;

    // GDAL Driver that was actually used to open the layer
    GDALDriverH mGDALDriver;

    // Friendly name of the GDAL Driver that was actually used to open the layer
    QString mGDALDriverName;

    bool mValid;

    OGRwkbGeometryType mOGRGeomType;
    long mFeaturesCounted;

    mutable QStringList mSubLayerList;

    bool addFeaturePrivate( QgsFeature &f, QgsFeatureSink::Flags flags );
    //! Deletes one feature
    bool deleteFeature( QgsFeatureId id );

    //! Calls OGR_L_SyncToDisk and recreates the spatial index if present
    bool syncToDisc();

    OGRLayerH setSubsetString( OGRLayerH layer, GDALDatasetH ds );

    friend class QgsOgrFeatureSource;

    //! Whether the file is opened in write mode
    bool mWriteAccess;

    //! Whether the file can potentially be opened in write mode (but not necessarily currently)
    bool mWriteAccessPossible;

    //! Whether the open mode of the datasource changes w.r.t calls to enterUpdateMode() / leaveUpdateMode()
    bool mDynamicWriteAccess;

    bool mShapefileMayBeCorrupted;

    //! Converts the geometry to the layer type if necessary. Takes ownership of the passed geometry
    OGRGeometryH ConvertGeometryIfNecessary( OGRGeometryH );

    int mUpdateModeStackDepth;

    void computeCapabilities();

    QgsVectorDataProvider::Capabilities mCapabilities;

    bool doInitialActionsForEdition();
};


class QgsOgrProviderUtils
{
  public:
    static void setRelevantFields( OGRLayerH ogrLayer, int fieldCount, bool fetchGeometry, const QgsAttributeList &fetchAttributes, bool firstAttrIsFid );
    static OGRLayerH setSubsetString( OGRLayerH layer, GDALDatasetH ds, QTextCodec *encoding, const QString &subsetString, bool &origFidAdded );
    static QByteArray quotedIdentifier( QByteArray field, const QString &driverName );

    /** Quote a value for placement in a SQL string.
     */
    static QString quotedValue( const QVariant &value );

    static GDALDatasetH GDALOpenWrapper( const char *pszPath, bool bUpdate, GDALDriverH *phDriver );
    static void GDALCloseWrapper( GDALDatasetH mhDS );
};

#endif // QGSOGRPROVIDER_H
