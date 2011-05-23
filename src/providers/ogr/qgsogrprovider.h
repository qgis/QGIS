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
/* $Id$ */

#include "qgsdataitem.h"
#include "qgsrectangle.h"
#include "qgsvectordataprovider.h"

class QgsFeature;
class QgsField;

#include <ogr_api.h>

/**
  \class QgsOgrProvider
  \brief Data provider for ESRI shapefiles
  */
class QgsOgrProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    /**
     * Constructor of the vector provider
     * @param uri  uniform resource locator (URI) for a dataset
     */
    QgsOgrProvider( QString const & uri = "" );

    /**
     * Destructor
     */
    virtual ~QgsOgrProvider();

    virtual QgsCoordinateReferenceSystem crs();

    /**
     * Sub-layers handled by this provider, in order from bottom to top
     *
     * Sub-layers are used when the provider's source can combine layers
     * it knows about in some way before it hands them off to the provider.
     */
    virtual QStringList subLayers() const;

    /**
     *   Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

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
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     */
    virtual bool nextFeature( QgsFeature& feature );

    /**
     * Gets the feature at the given feature ID.
     * @param featureId id of the feature
     * @param feature feature which will receive the data
     * @param fetchGeoemtry if true, geometry will be fetched from the provider
     * @param fetchAttributes a list containing the indexes of the attribute fields to copy
     * @return True when feature was found, otherwise false
     */
    virtual bool featureAtId( int featureId,
                              QgsFeature& feature,
                              bool fetchGeometry = true,
                              QgsAttributeList fetchAttributes = QgsAttributeList() );

    /** Accessor for sql where clause used to limit dataset */
    virtual QString subsetString();

    virtual bool supportsSubsetString() { return true; }

    /** mutator for sql where clause used to limit dataset size */
    virtual bool setSubsetString( QString theSQL, bool updateFeatureCount = true );

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const;

    /** return the number of layers for the current data source

    @note

    Should this be subLayerCount() instead?
    */
    virtual size_t layerCount() const;

    /**
     * Get the number of features in the layer
     */
    virtual long featureCount() const;

    /**
     * Get the number of fields in the layer
     */
    virtual uint fieldCount() const;

    /**
     * Get the field information for the layer
     */
    virtual const QgsFieldMap & fields() const;

    /** Return the extent for this data layer
     */
    virtual QgsRectangle extent();

    /** Restart reading features from previous select operation */
    virtual void rewind();

    /**Writes a list of features to the file*/
    virtual bool addFeatures( QgsFeatureList & flist );

    /**Deletes a feature*/
    virtual bool deleteFeatures( const QgsFeatureIds & id );

    /**Adds new attributess. Unfortunately not supported for layers with features in it*/
    virtual bool addAttributes( const QList<QgsField> &attributes );

    /**Changes attribute values of existing features */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap & attr_map );

    /**Changes existing geometries*/
    virtual bool changeGeometryValues( QgsGeometryMap & geometry_map );

    /**Tries to create a .qix index file for faster access if only a subset of the features is required
     @return true in case of success*/
    virtual bool createSpatialIndex();

    /**Create an attribute index on the datasource*/
    virtual bool createAttributeIndex( int field );

    /** Returns a bitmask containing the supported capabilities
        Note, some capabilities may change depending on whether
        a spatial filter is active on this provider, so it may
        be prudent to check this value per intended operation.
        See the OGRLayer::TestCapability API for details.
      */
    virtual int capabilities() const;

    virtual void setEncoding( const QString& e );


    /** return vector file filter string

      Returns a string suitable for a QFileDialog of vector file formats
      supported by the data provider.  Naturally this will be an empty string
      for those data providers that do not deal with plain files, such as
      databases and servers.

      @note

      It'd be nice to eventually be raster/vector neutral.
    */
    /* virtual */
    QString fileVectorFilters() const;
    /** return a string containing the available database drivers */
    QString databaseDrivers() const;
    /** return a string containing the available directory drivers */
    QString protocolDrivers() const;
    /** return a string containing the available protocol drivers */
    QString directoryDrivers() const;

    /**Returns true if this is a valid shapefile
    */
    bool isValid();

    /** Returns the minimum value of an attribute
     *  @param index the index of the attribute */
    QVariant minimumValue( int index );

    /** Returns the maximum value of an attribute
     *  @param index the index of the attribute */
    QVariant maximumValue( int index );

    /** Return the unique values of an attribute
     *  @param index the index of the attribute
     *  @param values reference to the list of unique values */
    virtual void uniqueValues( int index, QList<QVariant> &uniqueValues, int limit = -1 );

    /** return a provider name

    Essentially just returns the provider key.  Should be used to build file
    dialogs so that providers can be shown with their supported types. Thus
    if more than one provider supports a given format, the user is able to
    select a specific provider to open that file.

    @note

    Instead of being pure virtual, might be better to generalize this
    behavior and presume that none of the sub-classes are going to do
    anything strange with regards to their name or description?

    */
    QString name() const;


    /** return description

      Return a terse string describing what the provider is.

      @note

      Instead of being pure virtual, might be better to generalize this
      behavior and presume that none of the sub-classes are going to do
      anything strange with regards to their name or description?

     */
    QString description() const;

    /** Returns true if the provider is strict about the type of inserted features
          (e.g. no multipolygon in a polygon layer)
          @note: added in version 1.4*/
    virtual bool doesStrictFeatureTypeCheck() const { return false;}

  protected:
    /** loads fields from input file to member attributeFields */
    void loadFields();

    /**Get an attribute associated with a feature*/
    void getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature & f, int attindex );

    /** find out the number of features of the whole layer */
    void recalculateFeatureCount();

    /** tell OGR, which fields to fetch in nextFeature/featureAtId (ie. which not to ignore) */
    void setRelevantFields( bool fetchGeometry, const QgsAttributeList& fetchAttributes );

  private:
    bool crsFromWkt( QgsCoordinateReferenceSystem &srs, const char *wkt );
    unsigned char *getGeometryPointer( OGRFeatureH fet );
    QgsFieldMap mAttributeFields;
    OGRDataSourceH ogrDataSource;
    void *extent_;

    /**This member variable receives the same value as extent_
     in the method QgsOgrProvider::extent(). The purpose is to prevent a memory leak*/
    QgsRectangle mExtentRect;
    OGRLayerH ogrLayer;
    OGRLayerH ogrOrigLayer;

    //! path to filename
    QString mFilePath;

    //! layer name
    QString mLayerName;

    //! layer index
    int mLayerIndex;

    //! String used to define a subset of the layer
    QString mSubsetString;

    // OGR Driver that was actually used to open the layer
    OGRSFDriverH ogrDriver;

    // Friendly name of the OGR Driver that was actually used to open the layer
    QString ogrDriverName;

    bool valid;
    //! Flag to indicate that spatial intersect should be used in selecting features
    bool mUseIntersect;
    int geomType;
    long featuresCounted;

    //! Selection rectangle
    OGRGeometryH mSelectionRectangle;
    /**Adds one feature*/
    bool addFeature( QgsFeature& f );
    /**Deletes one feature*/
    bool deleteFeature( int id );

    QString quotedIdentifier( QString field );

    /**Calls OGR_L_SyncToDisk and recreates the spatial index if present*/
    bool syncToDisc();
};

class QgsOgrLayerItem : public QgsLayerItem
{
    Q_OBJECT
  public:
    QgsOgrLayerItem( QgsDataItem* parent, QString name, QString path, QString uri, LayerType layerType );
    ~QgsOgrLayerItem();

    bool setCrs( QgsCoordinateReferenceSystem crs );
    Capability capabilities();
};

