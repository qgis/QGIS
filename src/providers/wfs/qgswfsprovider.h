/***************************************************************************
                              qgswfsprovider.h
                              -------------------
  begin                : July 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
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

#include <QDomElement>
#include "qgis.h"
#include "qgsrectangle.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsvectordataprovider.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"

class QgsRectangle;
class QgsSpatialIndex;

/**A provider reading features from a WFS server*/
class QgsWFSProvider: public QgsVectorDataProvider
{
    Q_OBJECT
  public:

    enum REQUEST_ENCODING
    {
      GET,
      FILE  //reads from a file on disk
    };

    QgsWFSProvider( const QString& uri );
    ~QgsWFSProvider();

    /* Inherited from QgsVectorDataProvider */

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to nextFeature.
     *  @param fetchAttributes list of attributes which should be fetched
     *  @param rect spatial filter
     *  @param fetchGeometry true if the feature geometry should be fetched
     *  @param useIntersect true if an accurate intersection test should be used,
     *                     false if a test based on bounding box is sufficient
     */
    virtual void select( QgsAttributeList fetchAttributes = QgsAttributeList(),
                         QgsRectangle rect = QgsRectangle(),
                         bool fetchGeometry = true,
                         bool useIntersect = false );

    /**
     * Gets the feature at the given feature ID.
     * @param featureId of the feature to be returned
     * @param feature which will receive the data
     * @param fetchGeometry flag which if true, will cause the geometry to be fetched from the provider
     * @param fetchAttributes a list containing the indexes of the attribute fields to copy
     * @return True when feature was found, otherwise false
     *
     * Default implementation traverses all features until it finds the one with correct ID.
     * In case the provider supports reading the feature directly, override this function.
     */
    virtual bool featureAtId( QgsFeatureId featureId,
                              QgsFeature& feature,
                              bool fetchGeometry = true,
                              QgsAttributeList fetchAttributes = QgsAttributeList() );

    /**
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     */
    virtual bool nextFeature( QgsFeature& feature );

    QGis::WkbType geometryType() const;
    long featureCount() const;
    uint fieldCount() const;
    const QgsFieldMap & fields() const;
    void rewind();

    virtual QgsCoordinateReferenceSystem crs();

    /* Inherited from QgsDataProvider */

    QgsRectangle extent();
    bool isValid();
    QString name() const;
    QString description() const;

    virtual int capabilities() const;

    /* new functions */

    /**Sets the encoding type in which the provider makes requests and interprets
     results. Possibilities are GET, POST, SOAP*/
    void setRequestEncoding( QgsWFSProvider::REQUEST_ENCODING e ) {mRequestEncoding = e;}

    /**Makes a GetFeatures, receives the features from the wfs server (as GML), converts them to QgsFeature and
       stores them in a vector*/
    int getFeature( const QString& uri );

    //Editing operations
    /**
     * Adds a list of features
     * @return true in case of success and false in case of failure
     */
    virtual bool addFeatures( QgsFeatureList &flist );

    /**
     * Deletes one or more features
     * @param id list containing feature ids to delete
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteFeatures( const QgsFeatureIds &id );

    /**
     * Changes geometries of existing features
     * @param geometry_map   A QgsGeometryMap whose index contains the feature IDs
     *                       that will have their geometries changed.
     *                       The second map parameter being the new geometries themselves
     * @return               True in case of success and false in case of failure
     */
    virtual bool changeGeometryValues( QgsGeometryMap & geometry_map );

    /**
     * Changes attribute values of existing features.
     * @param attr_map a map containing changed attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map );

    /**Reloads the data from the the source. Needs to be implemented by providers with data caches to
      synchronize with changes in the data source*/
    virtual void reloadData();

    /**Collects information about the field types. Is called internally from QgsWFSProvider ctor. The method delegates the work to request specific ones and gives back the name of the geometry attribute and the thematic attributes with their types*/
    int describeFeatureType( const QString& uri, QString& geometryAttribute,
                             QgsFieldMap& fields, QGis::WkbType& geomType );

  signals:
    void dataReadProgressMessage( QString message );

  private slots:
    /**Receives the progress signals from QgsWFSData::dataReadProgress, generates a string
     and emits the dataReadProgressMessage signal*/
    void handleWFSProgressMessage( int done, int total );

    /**Sets mNetworkRequestFinished flag to true*/
    void networkRequestFinished();

  private:
    bool mNetworkRequestFinished;

  protected:
    /**Thematic attributes*/
    QgsFieldMap mFields;
    /**Name of geometry attribute*/
    QString mGeometryAttribute;
    /**The encoding used for request/response. Can be GET, POST or SOAP*/
    REQUEST_ENCODING mRequestEncoding;
    /**Bounding box for the layer*/
    QgsRectangle mExtent;
    /**Spatial filter for the layer*/
    QgsRectangle mSpatialFilter;
    /**Flag if precise intersection test is needed. Otherwise, every feature is returned (even if a filter is set)*/
    bool mUseIntersect;
    /**A spatial index for fast access to a feature subset*/
    QgsSpatialIndex *mSpatialIndex;
    /**Vector where the ids of the selected features are inserted*/
    QList<QgsFeatureId> mSelectedFeatures;
    /**Iterator on the feature vector for use in rewind(), nextFeature(), etc...*/
    QList<QgsFeatureId>::iterator mFeatureIterator;
    /**Map <feature Id / feature> */
    QMap<QgsFeatureId, QgsFeature* > mFeatures;
    /**Stores the relation between provider ids and WFS server ids*/
    QMap<QgsFeatureId, QString > mIdMap;
    /**Geometry type of the features in this layer*/
    mutable QGis::WkbType mWKBType;
    /**Source CRS*/
    QgsCoordinateReferenceSystem mSourceCRS;
    int mFeatureCount;
    /**Flag if provider is valid*/
    bool mValid;
    /**Namespace URL of the server (comes from DescribeFeatureDocument)*/
    QString mWfsNamespace;
    /**Server capabilities for this layer (generated from capabilities document)*/
    int mCapabilities;
    /**GetRenderedOnly: layer asociated with this provider*/
    QgsVectorLayer *mLayer;
    /**GetRenderedOnly: fetch only features within canvas extent to be rendered*/
    bool mGetRenderedOnly;
    /**GetRenderedOnly initializaiton flat*/
    bool mInitGro;
    /**if GetRenderedOnly, extent specified in WFS getFeatures; else empty (no constraint)*/
    QgsRectangle mGetExtent;

    //encoding specific methods of getFeature
    int getFeatureGET( const QString& uri, const QString& geometryAttribute );
    int getFeaturePOST( const QString& uri, const QString& geometryAttribute );
    int getFeatureSOAP( const QString& uri, const QString& geometryAttribute );
    int getFeatureFILE( const QString& uri, const QString& geometryAttribute );
    //encoding specific methods of describeFeatureType
    int describeFeatureTypeGET( const QString& uri, QString& geometryAttribute, QgsFieldMap& fields, QGis::WkbType& geomType );
    int describeFeatureTypePOST( const QString& uri, QString& geometryAttribute, QgsFieldMap& fields );
    int describeFeatureTypeSOAP( const QString& uri, QString& geometryAttribute, QgsFieldMap& fields );
    int describeFeatureTypeFile( const QString& uri, QString& geometryAttribute, QgsFieldMap& fields, QGis::WkbType& geomType );

    /**Reads the name of the geometry attribute, the thematic attributes and their types from a dom document. Returns 0 in case of success*/
    int readAttributesFromSchema( QDomDocument& schemaDoc, QString& geometryAttribute, QgsFieldMap& fields, QGis::WkbType& geomType );
    /**This method tries to guess the geometry attribute and the other attribute names from the .gml file if no schema is present. Returns 0 in case of success*/
    int guessAttributesFromFile( const QString& uri, QString& geometryAttribute, std::list<QString>& thematicAttributes ) const;

    /**Copies feature attributes / geometry from f to feature*/
    void copyFeature( QgsFeature* f, QgsFeature& feature, bool fetchGeometry, QgsAttributeList fetchAttributes );

    //GML2 specific methods
    int getExtentFromGML2( QgsRectangle* extent, const QDomElement& wfsCollectionElement ) const;

    int getFeaturesFromGML2( const QDomElement& wfsCollectionElement, const QString& geometryAttribute );

    int getWkbFromGML2( const QDomNode& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const;
    /**Creates WKB from a <Point> element*/
    int getWkbFromGML2Point( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const;
    /**Creates WKB from a <Polygon> element*/
    int getWkbFromGML2Polygon( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const;
    /**Creates WKB from a <LineString> element*/
    int getWkbFromGML2LineString( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const;
    /**Creates WKB from a <MultiPoint> element*/
    int getWkbFromGML2MultiPoint( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const;
    /**Creates WKB from a <MultiLineString> element*/
    int getWkbFromGML2MultiLineString( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const;
    /**Creates WKB from a <MultiPolygon> element*/
    int getWkbFromGML2MultiPolygon( const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WkbType* type ) const;
    /**Reads the <gml:coordinates> element and extracts the coordinates as points
       @param coords list where the found coordinates are appended
       @param elem the <gml:coordinates> element
       @return 0 in case of success*/
    int readGML2Coordinates( std::list<QgsPoint>& coords, const QDomElement elem ) const;
    /**Tries to create a QgsCoordinateReferenceSystem object and assign it to mSourceCRS. Returns 0 in case of success*/
    int setCRSFromGML2( const QDomElement& wfsCollectionElement );


    //methods to write GML2

    QDomElement createGeometryElem( QgsGeometry* g, QDomDocument& doc ) /*const*/;
    QDomElement createLineStringElem( QgsGeometry* geom, QDomDocument& doc ) const;
    QDomElement createMultiLineStringElem( QgsGeometry* geom, QDomDocument& doc ) const;
    QDomElement createPointElem( QgsGeometry* geom, QDomDocument& doc ) const;
    QDomElement createMultiPointElem( QgsGeometry* geom, QDomDocument& doc ) const;
    QDomElement createPolygonElem( QgsGeometry* geom, QDomDocument& doc ) const;
    QDomElement createMultiPolygonElem( QgsGeometry* geom, QDomDocument& doc ) const;

    /**Create a GML coordinate string from a point list.
      @param points list of data points
      @param coordString out: GML coord string
      @return 0 in case of success*/
    QDomElement createCoordinateElem( const QVector<QgsPoint> points, QDomDocument& doc ) const;

    //helper methods for WFS-T

    /**Returns HTTP parameter value from url (or empty string if it does not exist)*/
    QString parameterFromUrl( const QString& name ) const;

    /**Removes a possible namespace prefix from a typename*/
    void removeNamespacePrefix( QString& tname ) const;
    /**Returns namespace prefix (or an empty string if there is no prefix)*/
    QString nameSpacePrefix( const QString& tname ) const;

    /**Sends the transaction document to the server using HTTP POST
      @return true if transmission to the server succeeded, otherwise false
        note: true does not automatically mean that the transaction succeeded*/
    bool sendTransactionDocument( const QDomDocument& doc, QDomDocument& serverResponse );

    /**Creates a transaction element and adds it (normally as first element) to the document*/
    QDomElement createTransactionElement( QDomDocument& doc ) const;

    /**True if the server response means success*/
    bool transactionSuccess( const QDomDocument& serverResponse ) const;
    /**Returns the inserted ids*/
    QStringList insertedFeatureIds( const QDomDocument& serverResponse ) const;
    /**Returns a key suitable for new items*/
    QgsFeatureId findNewKey() const;
    /**Retrieve capabilities for this layer from GetCapabilities document (will be stored in mCapabilites)*/
    void getLayerCapabilities();
    /**Takes <Operations> element and updates the capabilities*/
    void appendSupportedOperations( const QDomElement& operationsElem, int& capabilities ) const;
    /**Shows a message box with the exception string (or does nothing if the xml document is not an exception)*/
    void handleException( const QDomDocument& serverResponse ) const;
    /**Initializes "Cache Features" inactive processing*/
    bool initGetRenderedOnly( QgsRectangle );
    /**Converts DescribeFeatureType schema geometry property type to WKBType*/
    QGis::WkbType geomTypeFromPropertyType( QString attName, QString propType );

    void deleteData();
};

#endif
