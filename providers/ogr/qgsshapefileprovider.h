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

#include "../../src/qgsvectordataprovider.h"

class QgsFeature;
class QgsField;
class OGRDataSource;
class OGRLayer;
class OGRFeature;
class OGREnvelope;
class OGRPolygon;

/**
  \class QgsShapeFileProvider
  \brief Data provider for ESRI shapefiles
  */
class QgsShapeFileProvider:public QgsVectorDataProvider
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
    *@param getnotcommited flag indicating if not commited features should be returned
    */
    QgsFeature *getNextFeature(std::list<int>& attlist, bool getnotcommited=false);
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

    /**
     Enables editing capabilities of the provider (if supported)
     @return false in case of error or if the provider does not support editing
    */
    virtual bool startEditing();

    /**
       Disables the editing capabilities of the provider
    */
    virtual void stopEditing();

    /**
       Commits changes
       @return false in case of problems
    */
    virtual bool commitChanges();

    /**
     Discards changes
     @return false in case of problems
    */
    virtual bool rollBack();

    /**Returns true if the provider is in editing mode*/
    virtual bool isEditable() const {return mEditable;}

    /**Returns true if the provider has been modified since the last commit*/
    virtual bool isModified() const {return mModified;}

 protected:
    
     /**Commits a feature
       @return true in case of success and false in case of failure*/
    virtual bool commitFeature(QgsFeature* f);
    /**Features which are added but not yet commited*/
    std::list<QgsFeature*> mAddedFeatures;
    /**Flag indicating wheter the provider is in editing mode or not*/
    bool mEditable;
    /**Flag indicating wheter the provider has been modified since the last commit*/
    bool mModified;

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
    /**If getNextFeature needs to returns pointers to not commited features, 
    this member points to the latest feature*/
    std::list<QgsFeature*>::iterator mAddedFeaturesIt;
    /**Flag indicating, if the minmaxcache should be renewed (true) or not (false)*/
    bool minmaxcachedirty;
    /**Matrix storing the minimum and maximum values*/
    double **minmaxcache;
    /**Fills the cash and sets minmaxcachedirty to false*/
    void fillMinMaxCash();
    //! Selection rectangle 
    OGRPolygon * mSelectionRectangle;


};
