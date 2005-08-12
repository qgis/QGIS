/***************************************************************************
      qgsgpxprovider.h  -  Data provider for GPS eXchange files
                             -------------------
    begin                : 2004-04-14
    copyright            : (C) 2004 by Lars Luthman
    email                : larsl@users.sourceforge.net

    Partly based on qgsdelimitedtextprovider.h, (C) 2004 Gary E. Sherman
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <map>

#include "../../src/qgsvectordataprovider.h"
#include "gpsdata.h"


class QgsFeature;
class QgsField;
class QFile;
class QDomDocument;
class GPSData;


/**
\class QgsGPXProvider
\brief Data provider for GPX (GPS eXchange) files
* This provider adds the ability to load GPX files as vector layers.
* 
*/
class QgsGPXProvider : public QgsVectorDataProvider {
public:
  QgsGPXProvider(QString uri=0);
  virtual ~QgsGPXProvider();
  
  /**
    *   Returns the permanent storage type for this layer as a friendly name.
    */
  QString storageType();

  /** Used to ask the layer for its projection as a WKT string. Implements 
      virtual method of same name in QgsDataProvider. */
  QString getProjectionWKT();
  /**
   * Get the first feature resulting from a select operation
   * @return QgsFeature
   */
  QgsFeature * getFirstFeature(bool fetchAttributes=false);
  /** 
  * Get the next feature resutling from a select operation
  * @return QgsFeature
  */
  QgsFeature * getNextFeature(bool fetchAttributes=false);
  bool getNextFeature(QgsFeature &feature, bool fetchAttributes=false);
  QgsFeature * getNextFeature(std::list<int> const & attlist, int featureQueueSize = 1);
  
  /** Get the feature type. This corresponds to 
      WKBPoint,
      WKBLineString,
      WKBPolygon,
      WKBMultiPoint,
      WKBMultiLineString or
      WKBMultiPolygon
  * as defined in qgis.h
  * This provider will always return WKBPoint
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
   */
  void select(QgsRect *mbr, bool useIntersect=false);
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
  virtual std::vector<QgsFeature>& identify(QgsRect *rect);
  
  /** Return the extent for this data layer
   */
  virtual QgsRect * extent();
  
  /**
   * Get the field information for the layer
   */
  std::vector<QgsField> const & fields() const;
  
  /* Reset the layer (ie move the file pointer to the head
     of the file.
  */
  void reset();
    
  /**Returns the minimum value of an attribute
     @param position the number of the attribute*/
  QString minValue(int position);
  
  /**Returns the maximum value of an attribute
     @param position the number of the attribute*/
  QString maxValue(int position);
  
  /**Returns true if this is a valid delimited file
   */
  bool isValid();

  /**Adds a list of features
     @return true in case of success and false in case of failure*/
  bool addFeatures(std::list<QgsFeature*> flist);
  
  bool deleteFeatures(std::list<int> const & id);
  
  bool changeAttributeValues(std::map<int,std::map<QString,QString> > const & 
			     attr_map);
  
  void changeAttributeValues(GPSObject& obj, 
			     const std::map<QString, QString>& attrs);
  
  /** Adds one feature (used by addFeatures()) */
  bool QgsGPXProvider::addFeature(QgsFeature* f);
  
  /**Returns the default value for attribute @c attr for feature @c f. */
  QString getDefaultValue(const QString& attr, QgsFeature* f);
  
  /**
   * Check to see if the point is withn the selection
   * rectangle
   * @param x X value of point
   * @param y Y value of point
   * @return True if point is within the rectangle
   */
  bool boundsCheck(double x, double y);

  int capabilities() const {
    return AddFeatures | DeleteFeatures | ChangeAttributeValues;
  }
  
  QgsDataSourceURI *getURI()
  {
      return 0;
  }

  /** return the number of layers for the current data source

  @note 

  Should this be subLayerCount() instead?
  */
  size_t layerCount() const;

  
private:
  
  /** Internal function used by the other getNextFeature() functions. */
  bool getNextFeature(QgsFeature* feature, std::list<int> const & attlist);

  bool mEditable;
  GPSData* data;
  void fillMinMaxCash();
  //! Fields
  std::vector<QgsField> attributeFields;
  std::list<int> mAllAttributes;
  //! Map to store field position by name
  std::map<QString, int> fieldPositions;
  QString mDataSourceUri;
  QString mFileName;
  enum { WaypointType, RouteType, TrackType } mFeatureType;
  enum Attribute { NameAttr = 0, EleAttr, SymAttr, NumAttr, 
		   CmtAttr, DscAttr, SrcAttr, URLAttr, URLNameAttr };
  static const char* attr[];
  //! Current selection rectangle
  QgsRect *mSelectionRectangle;
  bool mValid;
  int mGeomType;
  long mNumberFeatures;
  
  //! Current waypoint iterator
  GPSData::WaypointIterator mWptIter;
  //! Current route iterator
  GPSData::RouteIterator mRteIter;
  //! Current track iterator
  GPSData::TrackIterator mTrkIter;

  /**Flag indicating, if the minmaxcache should be renewed (true) or not (false)*/
  bool mMinMaxCacheDirty;
  /**Matrix storing the minimum and maximum values*/
  double** mMinMaxCache;
  /**Fills the cash and sets minmaxcachedirty to false*/
  void mFillMinMaxCash();
  struct wkbPoint{
    char byteOrder;
    unsigned wkbType;
    double x;
    double y;
  };
  wkbPoint mWKBpt;
  
};
