/***************************************************************************
      qgsdelimitedtextprovider.h  -  Data provider for delimted text
                             -------------------
    begin                : 2004-02-27
    copyright            : (C) 2004 by Gary E.Sherman
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
#include <map>
#include "../../src/qgsvectordataprovider.h"
class QgsFeature;
class QgsField;
class QFile;
/**
\class QgsDelimitedTextProvider
\brief Data provider for delimited text files.
*
* The provider needs to know both the path to the text file and
* the delimiter to use. Since the means to add a layer is farily
* rigid, we must provide this information encoded in a form that
* the provider can decipher and use.
* The uri must contain the path and delimiter in this format:
* /full/path/too/delimited.txt?delimiter=<delimiter>
*
* Example uri = "/home/foo/delim.txt?delimiter=|"
*/
class QgsDelimitedTextProvider : public QgsVectorDataProvider {
public:
  QgsDelimitedTextProvider(QString uri=0);
  virtual ~QgsDelimitedTextProvider();
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

  QgsFeature * getNextFeature(std::list<int>& attlist);
  
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

  /** Return endian-ness for this layer
  */  
  int endian();

  /** Return the extent for this data layer
  */
  virtual QgsRect * extent();
  /**
  * Get the attributes associated with a feature
  */
 void getFeatureAttributes(int key, QgsFeature *f); 
 /**
 * Get the field information for the layer
 */
 std::vector<QgsField>& fields();
 
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

 /**
  * Check to see if the point is withn the selection
  * rectangle
  * @param x X value of point
  * @param y Y value of point
  * @return True if point is within the rectangle
  */
 bool boundsCheck(double x, double y);

 //! We support saving as shapefile - used to add item to the
 //  layers context menu
 bool supportsSaveAsShapefile();
 
  //! Save the layer as a shapefile
  bool saveAsShapefile();
private:
  void fillMinMaxCash();
  int * getFieldLengths();
  //! Fields
  std::vector<QgsField> attributeFields;
  //! Map to store field position by name
  std::map<QString, int> fieldPositions;
  QString mDataSourceUri;
  QString mFileName;
  QString mDelimiter;
  QString mXField;
  QString mYField;
  //! Layer extent
  QgsRect *mExtent;
  //! Current selection rectangle
  QgsRect *mSelectionRectangle;
  //! Text file
  QFile *mFile;
  bool mValid;
  int mGeomType;
  long mNumberFeatures;
  //! Feature id
  long mFid;
  enum ENDIAN
  {
    NDR = 1,
    XDR = 0
  };
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
