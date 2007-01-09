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

#include "qgsvectordataprovider.h"
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
class QgsGPXProvider : public QgsVectorDataProvider
{
  
public:
  
  QgsGPXProvider(QString uri = QString());
  virtual ~QgsGPXProvider();
  
  /* Functions inherited from QgsVectorDataProvider */
  
  /**
   *   Returns the permanent storage type for this layer as a friendly name.
   */
  virtual QString storageType() const;

  /**
   * Select features based on a bounding rectangle. Features can be retrieved 
   * with calls to getFirstFeature and getNextFeature.
   * @param mbr QgsRect containing the extent to use in selecting features
   */
  virtual void select(QgsRect mbr, bool useIntersect=false);

  /**
   * Get the next feature resulting from a select operation.
   * @param feature feature which will receive data from the provider
   * @param fetchGeoemtry if true, geometry will be fetched from the provider
   * @param fetchAttributes a list containing the indexes of the attribute fields to copy
   * @param featureQueueSize  a hint to the provider as to how many features are likely to be retrieved in a batch
   * @return true when there was a feature to fetch, false when end was hit
   */
  virtual bool getNextFeature(QgsFeature& feature,
                              bool fetchGeometry = true,
                              QgsAttributeList fetchAttributes = QgsAttributeList(),
                              uint featureQueueSize = 1);
  
  /**
   * Get feature type.
   * @return int representing the feature type
   */
  virtual QGis::WKBTYPE geometryType() const;

  /**
   * Number of features in the layer
   * @return long containing number of features
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
  
  /** 
   * Reset the layer (ie move the file pointer to the head of the file.
   */
  virtual void reset();
    
  /**Returns the minimum value of an attribute
  @param position the number of the attribute*/
  virtual QString minValue(uint position);
  
  /**Returns the maximum value of an attribute
  @param position the number of the attribute*/
  virtual QString maxValue(uint position);
  
  /**
   * Adds a list of features
   * @return true in case of success and false in case of failure
   */
  virtual bool addFeatures(QgsFeatureList & flist);

  /** 
   * Deletes a feature
   * @param id list containing feature ids to delete
   * @return true in case of success and false in case of failure
   */
  virtual bool deleteFeatures(const QgsFeatureIds & id);
  
  /**
   * Changes attribute values of existing features.
   * @param attr_map a map containing changed attributes
   * @return true in case of success and false in case of failure 
   */
  virtual bool changeAttributeValues(const QgsChangedAttributesMap & attr_map);
  
  virtual int capabilities() const;
  
  /**Returns the default value for attribute @c attr for feature @c f. */
  virtual QString getDefaultValue(const QString& attr, QgsFeature* f);
  
  
  /* Functions inherited from QgsDataProvider */
  
  /** Return the extent for this data layer
   */
  virtual QgsRect extent();
  
  /**Returns true if this is a valid delimited file
   */
  virtual bool isValid();

  /** return a provider name */
  virtual QString name() const;

  /** return description */
  virtual QString description() const;

  virtual void setSRS(const QgsSpatialRefSys& theSRS);

  virtual QgsSpatialRefSys getSRS();
  
  
  /* new functions */

  void changeAttributeValues(GPSObject& obj, 
                             const QgsAttributeMap& attrs);
  
  /** Adds one feature (used by addFeatures()) */
  bool addFeature(QgsFeature& f);
  
  /**
   * Check to see if the point is withn the selection
   * rectangle
   * @param x X value of point
   * @param y Y value of point
   * @return True if point is within the rectangle
   */
  bool boundsCheck(double x, double y);


private:
  
  /** Internal function used by the other getNextFeature() functions. */
  bool getNextFeature(QgsFeature* feature, std::list<int> const & attlist);

  bool mEditable;
  GPSData* data;
  void fillMinMaxCash();
  //! Fields
  QgsFieldMap attributeFields;
  
  //! Map to store field position by name
  std::map<QString, int> fieldPositions;

  QString mFileName;

  enum { WaypointType, RouteType, TrackType } mFeatureType;
  enum Attribute { NameAttr = 0, EleAttr, SymAttr, NumAttr, 
		   CmtAttr, DscAttr, SrcAttr, URLAttr, URLNameAttr };
  static const char* attr[];
  //! Current selection rectangle
  QgsRect *mSelectionRectangle;
  bool mValid;
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
