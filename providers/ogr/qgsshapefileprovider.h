/***************************************************************************
  qgsshapefileprovider.h  -  Data provider for ESRI shapefile format
  -------------------
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

#include "../../src/qgsdataprovider.h"

class QgsFeature;
class QgsField;
class OGRDataSource;
class OGRLayer;
class OGRFeature;
class OGREnvelope;

/**
  \class QgsShapeFileProvider
  \brief Data provider for ESRI shapefiles
  */
class QgsShapeFileProvider:public QgsDataProvider
{
  public:
    QgsShapeFileProvider(QString uri = 0);
    virtual ~ QgsShapeFileProvider();
    /**
     * Get the first feature resutling from a select operation
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
    */
    QgsFeature *getNextFeature(std::list<int>& attlist);
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
    int geometryType();
    /** 
     * Get the number of features in the layer
     */
    long featureCount();
    /** 
     * Get the number of fields in the layer
     */
    int fieldCount();
    /**
     * Select features based on a bounding rectangle. Features can be retrieved 
     * with calls to getFirstFeature and getNextFeature.
     * @param mbr QgsRect containing the extent to use in selecting features
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

    /** Return endian-ness for this layer
    */
    int endian();

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
    std::vector < QgsField > &fields();

    /* Reset the layer - for an OGRLayer, this means clearing the
     * spatial filter and calling ResetReading
     */
    void reset();

    /**Returns the minimum value of an attribut
      @param position the number of the attribute*/
    QString minValue(int position);

    /**Returns the maximum value of an attribut
      @param position the number of the attribute*/
    QString maxValue(int position);

    /**Returns true if this is a valid shapefile
    */
    bool isValid();

    /**Adds a feature
       @return true in case of success and false in case of failure*/
    bool addFeature(QgsFeature* f);

    /**Deletes a feature
       @param id the number of the feature
       @return true in case of success and false in case of failure*/
    bool deleteFeature(int id);

  private:
    unsigned char *getGeometryPointer(OGRFeature * fet);
    std::vector < QgsField > attributeFields;
    QString dataSourceUri;
    OGRDataSource *ogrDataSource;
    OGREnvelope *extent_;
    OGRLayer *ogrLayer;
    bool valid;
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


};
