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
#include "qgsvectordataprovider.h"
#include "qgswfscapabilities.h"
#include "qgswfsfeatureiterator.h"
#include "qgswfsdatasourceuri.h"

class QgsRectangle;
class QgsWFSSharedData;


/** \ingroup WFSProvider
 *
 * A provider reading/write features from/into a WFS server.
 *
 * Below quick design notes on the whole provider.
 *
 * QgsWFSProvider class purpose:
 * - in constructor, do a GetCapabilities request to determine server-side feature limit,
     paging capabilities, WFS version, edition capabilities. Do a DescribeFeatureType request
     to determine fields, geometry name and type.
 * - in other methods, mostly WFS-T related operations.
 *
 * QgsWFSSharedData class purpose:
 * - contains logic shared by QgsWFSProvider, QgsWFSFeatureIterator and QgsWFSFeatureDownloader.
 * - one of its main function is to maintain a on-disk cache of the features retrieved
 *   from the server. This cache is a Spatialite database.
 *
 * QgsWFSRequest class purpose: abstract base class to create WFS network requests,
 * such as QgsWFSCapabilities, QgsWFSDescribeFeatureType, QgsWFSFeatureDownloader,
 * QgsWFSFeatureHitsAsyncRequest, QgsWFSFeatureHitsRequest, QgsWFSTransactionRequest
 *
 * QgsWFSDataSourceURI class purpose: wrapper above QgsDataSourceURI to get/set
 * the specific attributes of a WFS URI.
 *
 */
class QgsWFSProvider : public QgsVectorDataProvider
{
    Q_OBJECT
  public:

    explicit QgsWFSProvider( const QString& uri, const QgsWFSCapabilities::Capabilities &caps = QgsWFSCapabilities::Capabilities() );
    ~QgsWFSProvider();

    /* Inherited from QgsVectorDataProvider */

    virtual QgsAbstractFeatureSource* featureSource() const override;

    QgsFeatureIterator getFeatures( const QgsFeatureRequest& request = QgsFeatureRequest() ) override;

    QGis::WkbType geometryType() const override;
    long featureCount() const override;

    const QgsFields& fields() const override;

    virtual QgsCoordinateReferenceSystem crs() override;

    /** Accessor for sql where clause used to limit dataset */
    virtual QString subsetString() override;

    /** Mutator for sql where clause used to limit dataset size */
    virtual bool setSubsetString( const QString& theSQL, bool updateFeatureCount = true ) override;

    virtual bool supportsSubsetString() override { return true; }

    /* Inherited from QgsDataProvider */

    QgsRectangle extent() override;
    bool isValid() override;
    QString name() const override;
    QString description() const override;

    virtual int capabilities() const override;

    /* new functions */

    QString geometryAttribute() const;

    const QString processSQLErrorMsg() const { return mProcessSQLErrorMsg; }

    const QString processSQLWarningMsg() const { return mProcessSQLWarningMsg; }

    //Editing operations
    /**
     * Adds a list of features
     * @return true in case of success and false in case of failure
     */
    virtual bool addFeatures( QgsFeatureList &flist ) override;

    /**
     * Deletes one or more features
     * @param id list containing feature ids to delete
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteFeatures( const QgsFeatureIds &id ) override;

    /**
     * Changes geometries of existing features
     * @param geometry_map   A QgsGeometryMap whose index contains the feature IDs
     *                       that will have their geometries changed.
     *                       The second map parameter being the new geometries themselves
     * @return               True in case of success and false in case of failure
     */
    virtual bool changeGeometryValues( const QgsGeometryMap &geometry_map ) override;

    /**
     * Changes attribute values of existing features.
     * @param attr_map a map containing changed attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;

  public slots:
    /** Reloads the data from the source. Needs to be implemented by providers with data caches to
      synchronize with changes in the data source*/
    virtual void reloadData() override;

  private slots:

    void featureReceivedAnalyzeOneFeature( QVector<QgsWFSFeatureGmlIdPair> );

    void pushErrorSlot( const QString& errorMsg );

  private:
    /** Mutable data shared between provider and feature sources */
    QSharedPointer<QgsWFSSharedData> mShared;

    friend class QgsWFSFeatureSource;

  protected:

    //! String used to define a subset of the layer
    QString mSubsetString;

    /** Geometry type of the features in this layer*/
    mutable QGis::WkbType mWKBType;
    /** Flag if provider is valid*/
    bool mValid;
    /** Namespace URL of the server (comes from DescribeFeatureDocument)*/
    QString mApplicationNamespace;
    /** Server capabilities for this layer (generated from capabilities document)*/
    int mCapabilities;
    /** Fields of this typename. Might be different from mShared->mFields in case of SELECT */
    QgsFields mThisTypenameFields;

    QString mProcessSQLErrorMsg;
    QString mProcessSQLWarningMsg;

    /** Collects information about the field types. Is called internally from QgsWFSProvider ctor.
       The method gives back the name of
       the geometry attribute and the thematic attributes with their types*/
    bool describeFeatureType( QString& geometryAttribute,
                              QgsFields& fields, QGis::WkbType& geomType );

    /** For a given typename, reads the name of the geometry attribute, the
        thematic attributes and their types from a dom document. Returns true in case of success*/
    bool readAttributesFromSchema( QDomDocument& schemaDoc,
                                   const QString& prefixedTypename,
                                   QString& geometryAttribute,
                                   QgsFields& fields, QGis::WkbType& geomType, QString& errorMsg );

    //helper methods for WFS-T

    /** Sends the transaction document to the server using HTTP POST
      @return true if transmission to the server succeeded, otherwise false
        note: true does not automatically mean that the transaction succeeded*/
    bool sendTransactionDocument( const QDomDocument& doc, QDomDocument& serverResponse );

    /** Creates a transaction element and adds it (normally as first element) to the document*/
    QDomElement createTransactionElement( QDomDocument& doc ) const;

    /** True if the server response means success*/
    bool transactionSuccess( const QDomDocument& serverResponse ) const;
    /** Returns the inserted ids*/
    QStringList insertedFeatureIds( const QDomDocument& serverResponse ) const;
    /** Retrieve version and capabilities for this layer from GetCapabilities document (will be stored in mCapabilites)*/
    bool getCapabilities();
    /** Records provider error*/
    void handleException( const QDomDocument& serverResponse );
    /** Converts DescribeFeatureType schema geometry property type to WKBType*/
    QGis::WkbType geomTypeFromPropertyType( const QString& attName, const QString& propType );
    /** Convert the value to its appropriate XML representation */
    QString convertToXML( const QVariant& value );

    bool processSQL( const QString& sqlString, QString& errorMsg, QString& warningMsg );
};

#endif /* QGSWFSPROVIDER_H */
