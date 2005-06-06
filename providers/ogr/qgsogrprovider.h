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

#include "../../src/qgsvectordataprovider.h"
#include <geos.h>

#include <ogr_spatialref.h>

class QgsFeature;
class QgsField;
class OGRDataSource;
class OGRLayer;
class OGRFeature;
class OGREnvelope;
class OGRPolygon;

/**
  \class QgsOgrProvider
  \brief Data provider for ESRI shapefiles
  */
class QgsOgrProvider:public QgsVectorDataProvider
{
  public:
    QgsOgrProvider(QString uri = 0);
    virtual ~ QgsOgrProvider();
    /** Used to ask the layer for its projection as a WKT string. Implements virtual method of same name in      QgsDataProvider. */
    QString getProjectionWKT()  ;    
    /**
     * Get the first feature resulting from a select operation
     * @return QgsFeature
     */
    QgsFeature *getFirstFeature(bool fetchAttributes = false);
    /** 
     * Get the next feature resutling from a select operation
     * @return QgsFeature
     */
    QgsFeature *getNextFeature(bool fetchAttributes = false);
    /**Get the next feature resulting from a select operation.
    *@param attlist a list containing the indexes of the attribute fields to copy
    *@param getnotcommited flag indicating if not commited features should be returned
    */
    QgsFeature *getNextFeature(std::list<int> const& attlist, int featureQueueSize = 1);
    /** 
     * Get the next feature resutling from a select operation
     * @return True if the feature was read. This does not indicate
     * that the feature is valid. Use QgsFeature::isValid() to check
     * the validity of a feature before using it.
     */
    bool getNextFeature(QgsFeature &feature, bool fetchAttributes = false);

    /** Get the feature type. This corresponds to 
      WKBPoint,
      WKBLineString,
      WKBPolygon,
      WKBMultiPoint,
      WKBMultiLineString or
      WKBMultiPolygon
     * as defined in qgis.h
     */
    int geometryType() const;
    /** 
     * Get the number of features in the layer
     */
    long featureCount() const;
    /** 
     * Get the number of fields in the layer
     */
    int fieldCount() const;
    /**
     * Select features based on a bounding rectangle. Features can be retrieved 
     * with calls to getFirstFeature and getNextFeature.
     * @param mbr QgsRect containing the extent to use in selecting features
     * @param useIntersect Use geos functions to determine the selected set
     */
    void select(QgsRect * mbr, bool useIntersect = false);
    /** 
     * Set the data source specification. This may be a path or database
     * connection string
     * @uri data source specification
     */
    void setDataSourceUri(QString uri);

    /** 
     * Get the data source specification. This may be a path or database
     * connection string
     * @return data source specification
     */
    QString getDataSourceUri();

    /**
     * Identify features within the search radius specified by rect
     * @param rect Bounding rectangle of search radius
     * @return std::vector containing QgsFeature objects that intersect rect
     */
    virtual std::vector < QgsFeature > &identify(QgsRect * rect);

    /** Return the extent for this data layer
    */
    virtual QgsRect *extent();
    /**Get an attribute associated with a feature*/
    void getFeatureAttribute(OGRFeature * ogrFet, QgsFeature * f, int attindex);
    /**
     * Get the attributes associated with a feature
     */
    void getFeatureAttributes(OGRFeature * ogrFet, QgsFeature * f);
    /**
     * Get the field information for the layer
     */
    std::vector < QgsField > const & fields() const;

    /* Reset the layer - for an OGRLayer, this means clearing the
     * spatial filter and calling ResetReading
     */
    /* virtual */ void reset();

    /**Returns the minimum value of an attribut
      @param position the number of the attribute*/
    QString minValue(int position);

    /**Returns the maximum value of an attribut
      @param position the number of the attribute*/
    QString maxValue(int position);

    /**Returns true if this is a valid shapefile
    */
    bool isValid();

    /**Writes a list of features to the file*/
    bool addFeatures(std::list<QgsFeature*> const flist);

    /**Adds new attributess. Unfortunately not supported for layers with features in it*/
    bool addAttributes(std::map<QString,QString> const & name);

    /**Changes attribute values of existing features */
    bool changeAttributeValues(std::map<int,std::map<QString,QString> > const & attr_map);

    QgsDataSourceURI * getURI()
    { 
        return 0;
    }

    /**Tries to create a .qix index file for faster access if only a subset of the features is required
     @return true in case of success*/
    bool createSpatialIndex();

    /**Returns a bitmask containing the supported capabilities*/
    int capabilities() const;

  private:
    unsigned char *getGeometryPointer(OGRFeature * fet);
    std::vector < QgsField > attributeFields;
    QString dataSourceUri;
    OGRDataSource *ogrDataSource;
    OGREnvelope *extent_;
    OGRLayer *ogrLayer;
    bool valid;
    //! Flag to indicate that spatial intersect should be used in selecting features
    bool mUseIntersect;
    int geomType;
    long numberFeatures;
    enum ENDIAN
    {
      NDR = 1,
      XDR = 0
    };
    /**Flag indicating, if the minmaxcache should be renewed (true) or not (false)*/
    bool minmaxcachedirty;
    /**Matrix storing the minimum and maximum values*/
    double **minmaxcache;
    /**Fills the cash and sets minmaxcachedirty to false*/
    void fillMinMaxCash();
    //! Selection rectangle 
    OGRPolygon * mSelectionRectangle;
    /**Adds one feature*/
    bool addFeature(QgsFeature* f);
    //! The geometry factory
    geos::GeometryFactory *geometryFactory;
    //! The well known text reader
    geos::WKTReader *wktReader;

};
