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

#include "qgsrect.h"
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



    virtual QgsCoordinateReferenceSystem getCRS();


    /**
     *   Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to getNextFeature.
     *  @param fetchAttributes list of attributes which should be fetched
     *  @param rect spatial filter
     *  @param fetchGeometry true if the feature geometry should be fetched
     *  @param useIntersect true if an accurate intersection test should be used,
     *                     false if a test based on bounding box is sufficient
     */
    virtual void select( QgsAttributeList fetchAttributes = QgsAttributeList(),
                         QgsRect rect = QgsRect(),
                         bool fetchGeometry = true,
                         bool useIntersect = false );

    /**
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     */
    virtual bool getNextFeature( QgsFeature& feature );

    /**
     * Gets the feature at the given feature ID.
     * @param featureId id of the feature
     * @param feature feature which will receive the data
     * @param fetchGeoemtry if true, geometry will be fetched from the provider
     * @param fetchAttributes a list containing the indexes of the attribute fields to copy
     * @return True when feature was found, otherwise false
     */
    virtual bool getFeatureAtId( int featureId,
                                 QgsFeature& feature,
                                 bool fetchGeometry = true,
                                 QgsAttributeList fetchAttributes = QgsAttributeList() );

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WKBTYPE geometryType() const;

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
    virtual QgsRect extent();

    /** Restart reading features from previous select operation */
    virtual void reset();

    /**Writes a list of features to the file*/
    virtual bool addFeatures( QgsFeatureList & flist );

    /**Deletes a feature*/
    virtual bool deleteFeatures( const QgsFeatureIds & id );

    /**Adds new attributess. Unfortunately not supported for layers with features in it*/
    virtual bool addAttributes( const QgsNewAttributesMap & attributes );

    /**Changes attribute values of existing features */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap & attr_map );

    /**Changes existing geometries*/
    virtual bool changeGeometryValues( QgsGeometryMap & geometry_map );

    /**Tries to create a .qix index file for faster access if only a subset of the features is required
     @return true in case of success*/
    virtual bool createSpatialIndex();

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
    /* virtual */ QString fileVectorFilters() const;

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
    virtual void uniqueValues( int index, QList<QVariant> &uniqueValues );

  protected:
    /** loads fields from input file to member attributeFields */
    void loadFields();

    /**Get an attribute associated with a feature*/
    void getFeatureAttribute( OGRFeatureH ogrFet, QgsFeature & f, int attindex );

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


  private:
    unsigned char *getGeometryPointer( OGRFeatureH fet );

    QgsFieldMap mAttributeFields;

    OGRDataSourceH ogrDataSource;
    void *extent_;

    /**This member variable receives the same value as extent_
     in the method QgsOgrProvider::extent(). The purpose is to prevent a memory leak*/
    QgsRect mExtentRect;
    OGRLayerH ogrLayer;

    // OGR Driver that was actually used to open the layer
    OGRSFDriverH ogrDriver;

    // Friendly name of the OGR Driver that was actually used to open the layer
    QString ogrDriverName;

    bool valid;
    //! Flag to indicate that spatial intersect should be used in selecting features
    bool mUseIntersect;
    int geomType;
    long numberFeatures;

    //! Selection rectangle
    OGRGeometryH mSelectionRectangle;
    /**Adds one feature*/
    bool addFeature( QgsFeature& f );
    /**Deletes one feature*/
    bool deleteFeature( int id );
};
