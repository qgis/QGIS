/***************************************************************************
                              qgswfsprovider.h
                              -------------------
  begin                : July 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
                         (C) 2016 by Even Rouault
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSPROVIDER_H
#define QGSWFSPROVIDER_H

#include "qgis.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsogcutils.h"
#include "qgsvectordataprovider.h"
#include "qgswfscapabilities.h"
#include "qgswfsfeatureiterator.h"
#include "qgswfsdatasourceuri.h"

#include "qgsprovidermetadata.h"

class QgsRectangle;
class QgsWFSSharedData;


/**
 * \ingroup WFSProvider
 *
 * \brief A provider reading/write features from/into a WFS server.
 *
 * Below quick design notes on the whole provider.
 *
 * QgsWFSProvider class purpose:
 *
 * - in constructor, do a GetCapabilities request to determine server-side feature limit,
 *   paging capabilities, WFS version, edition capabilities. Do a DescribeFeatureType request
 *   to determine fields, geometry name and type.
 * - in other methods, mostly WFS-T related operations.
 *
 * QgsWFSSharedData class purpose:
 *
 * - contains logic shared by QgsWFSProvider, QgsWFSFeatureIterator and QgsWFSFeatureDownloader.
 * - one of its main function is to maintain a on-disk cache of the features retrieved
 *   from the server. This cache is a SpatiaLite database.
 *
 * QgsWFSRequest class purpose: abstract base class to create WFS network requests,
 * such as QgsWFSCapabilities, QgsWFSDescribeFeatureType, QgsWFSFeatureDownloader,
 * QgsWFSFeatureHitsAsyncRequest, QgsWFSFeatureHitsRequest, QgsWFSTransactionRequest
 *
 * QgsWFSDataSourceURI class purpose: wrapper above QgsDataSourceUri to get/set
 * the specific attributes of a WFS URI.
 *
 */
class QgsWFSProvider final: public QgsVectorDataProvider
{
    Q_OBJECT
  public:

    static const QString WFS_PROVIDER_KEY;
    static const QString WFS_PROVIDER_DESCRIPTION;

    explicit QgsWFSProvider( const QString &uri, const QgsDataProvider::ProviderOptions &providerOptions, const QgsWfsCapabilities::Capabilities &caps = QgsWfsCapabilities::Capabilities() );
    ~QgsWFSProvider() override;

    /* Inherited from QgsVectorDataProvider */

    QgsAbstractFeatureSource *featureSource() const override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest &request = QgsFeatureRequest() ) const override;

    QgsWkbTypes::Type wkbType() const override;
    long long featureCount() const override;

    QgsFields fields() const override;

    QgsCoordinateReferenceSystem crs() const override;

    QString subsetString() const override;
    bool setSubsetString( const QString &theSQL, bool updateFeatureCount = true ) override;

    bool supportsSubsetString() const override { return true; }

    /* Inherited from QgsDataProvider */

    QgsRectangle extent() const override;
    bool isValid() const override;
    QString name() const override;
    QString description() const override;

    static QString providerKey();

    QgsVectorDataProvider::Capabilities capabilities() const override;

    QString storageType() const override { return QStringLiteral( "OGC WFS (Web Feature Service)" ); }

    /* new functions */

    QString geometryAttribute() const;

    const QString processSQLErrorMsg() const { return mProcessSQLErrorMsg; }

    const QString processSQLWarningMsg() const { return mProcessSQLWarningMsg; }

    //Editing operations

    bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = QgsFeatureSink::Flags() ) override;
    bool deleteFeatures( const QgsFeatureIds &id ) override;
    bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;
    bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    QVariantMap metadata() const override;
    QString translateMetadataKey( const QString &mdKey ) const override;
    QString translateMetadataValue( const QString &mdKey, const QVariant &value ) const override;

    bool empty() const override;

    std::shared_ptr<QgsWFSSharedData> sharedData() const { return mShared; }

    void handlePostCloneOperations( QgsVectorDataProvider *source ) override;

    static QgsWfsCapabilities::Capabilities getCachedCapabilities( const QString &uri );
    static QString buildFilterByGeometryType( const QgsWfsCapabilities::Capabilities &caps,
        const QString &geometryElement,
        const QString &function );
    static QString buildIsNullGeometryFilter( const QgsWfsCapabilities::Capabilities &caps,
        const QString &geometryElement );
    static QString buildGeometryCollectionFilter( const QgsWfsCapabilities::Capabilities &caps,
        const QString &geometryElement );

    //! Perform an initial GetFeature request with a 1-feature limit.
    void issueInitialGetFeature();

  private slots:

    void featureReceivedAnalyzeOneFeature( QVector<QgsFeatureUniqueIdPair> );

    void pushErrorSlot( const QString &errorMsg );


  private:
    //! Mutable data shared between provider and feature sources
    std::shared_ptr<QgsWFSSharedData> mShared;

    //! Field set by featureReceivedAnalyzeOneFeature() if a "description" field is set in the sample feature
    bool mSampleFeatureHasDescription = false;

    //! Field set by featureReceivedAnalyzeOneFeature() if a "identifier" field is set in the sample feature
    bool mSampleFeatureHasIdentifier = false;

    //! Field set by featureReceivedAnalyzeOneFeature() if a "name" field is set in the sample feature
    bool mSampleFeatureHasName = false;

    /**
     * Invalidates cache of shared object
    */
    void reloadProviderData() override;

    friend class QgsWFSFeatureSource;

    /**
     * Create the geometry element
     */
    QDomElement geometryElement( const QgsGeometry &geometry, QDomDocument &transactionDoc );

    //! Set mShared->mLayerPropertiesList from describeFeatureDocument
    bool setLayerPropertiesListFromDescribeFeature( QDomDocument &describeFeatureDocument, const QStringList &typenameList, QString &errorMsg );

    //! backup of mShared->mLayerPropertiesList on the feature type when there is no sql request
    QList< QgsOgcUtils::LayerProperties > mLayerPropertiesListWhenNoSqlRequest;

  protected:

    //! String used to define a subset of the layer
    QString mSubsetString;

    //! Flag if provider is valid
    bool mValid = true;
    //! Namespace URL of the server (comes from DescribeFeatureDocument)
    QString mApplicationNamespace;
    //! Server capabilities for this layer (generated from capabilities document)
    QgsVectorDataProvider::Capabilities mCapabilities = QgsVectorDataProvider::Capabilities();
    //! Fields of this typename. Might be different from mShared->mFields in case of SELECT
    QgsFields mThisTypenameFields;

    QString mProcessSQLErrorMsg;
    QString mProcessSQLWarningMsg;

    /**
     * Collects information about the field types. Is called internally from QgsWFSProvider ctor.
     * The method gives back the name of
     * the geometry attribute and the thematic attributes with their types.
    */
    bool describeFeatureType( QString &geometryAttribute,
                              QgsFields &fields, QgsWkbTypes::Type &geomType );

    /**
     * For a given typename, reads the name of the geometry attribute, the
     * thematic attributes and their types from a dom document. Returns true in case of success.
    */
    bool readAttributesFromSchema( QDomDocument &schemaDoc,
                                   const QString &prefixedTypename,
                                   QString &geometryAttribute,
                                   QgsFields &fields, QgsWkbTypes::Type &geomType, QString &errorMsg );

    //helper methods for WFS-T

    /**
     * Sends the transaction document to the server using HTTP POST
     * \returns true if transmission to the server succeeded, otherwise false
     * \note true does not automatically mean that the transaction succeeded
    */
    bool sendTransactionDocument( const QDomDocument &doc, QDomDocument &serverResponse );

    //! Creates a transaction element and adds it (normally as first element) to the document
    QDomElement createTransactionElement( QDomDocument &doc ) const;

    //! True if the server response means success
    bool transactionSuccess( const QDomDocument &serverResponse ) const;
    //! Returns the inserted ids
    QStringList insertedFeatureIds( const QDomDocument &serverResponse ) const;
    //! Retrieve version and capabilities for this layer from GetCapabilities document (will be stored in mCapabilities)
    bool getCapabilities();
    //! Records provider error
    void handleException( const QDomDocument &serverResponse );
    //! Converts DescribeFeatureType schema geometry property type to WKBType
    QgsWkbTypes::Type geomTypeFromPropertyType( const QString &attName, const QString &propType );
    //! Convert the value to its appropriate XML representation
    QString convertToXML( const QVariant &value );

    bool processSQL( const QString &sqlString, QString &errorMsg, QString &warningMsg );
};


#endif /* QGSWFSPROVIDER_H */
