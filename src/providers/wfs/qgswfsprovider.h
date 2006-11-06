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
#include "qgsrect.h"
#include "qgsvectordataprovider.h"
#include <indexStrtree.h>

class QgsRect;

/**A provider reading features from a WFS server*/
class QgsWFSProvider: public QgsVectorDataProvider
{
 public:

  enum REQUEST_ENCODING
    {
      GET,
      POST,
      SOAP /*Note that this goes also through HTTP POST but additionally uses soap envelope and friends*/
    };

  QgsWFSProvider(const QString& uri);
  ~QgsWFSProvider();
  QgsFeature* getFirstFeature(bool fetchAttributes = false);
  QgsFeature* getNextFeature(bool fetchAttributes = false);
  QgsFeature* getNextFeature(std::list<int> const & attlist, int featureQueueSize = 1);
  bool getNextFeature(QgsFeature &feature, bool fetchAttributes = false); /*legacy*/
  int geometryType() const;
  long featureCount() const;
  int fieldCount() const;
  std::vector<QgsField> const & fields() const;
  void reset();
  QString minValue(int position);
  QString maxValue(int position);
  std::vector<QgsFeature>& identify(QgsRect *rect); /*legacy*/
  QString getProjectionWKT();
  QgsRect* extent();
  bool isValid();
  QString name() const;
  QString description() const;
  size_t layerCount() const {return 1;}
  QgsDataSourceURI* getURI() {return 0;}
  virtual void select(QgsRect *mbr, bool useIntersect=false);

  /**Sets the encoding type in which the provider makes requests and interprets
   results. Posibilities are GET, POST, SOAP*/
  void setEncoding(QgsWFSProvider::REQUEST_ENCODING e) {mEncoding = e;}

  /**Makes a GetFeatures, receives the features from the wfs server (as GML), converts them to QgsFeature and \
     stores them in a vector*/
  int getFeature(const QString& uri);
  /**Return Srid number from mSourceSRS*/
  int getSrid();

  

 protected:
  std::vector<QgsField> mFields;
  /**The encoding used for request/response. Can be GET, POST or SOAP*/
  REQUEST_ENCODING mEncoding;
  /**Bounding box for the layer*/
  QgsRect mExtent;
  /**Spatial filter for the layer*/
  QgsRect mSpatialFilter;
  /**Flag if precise intersection test is needed. Otherwise, every feature is returned (even if a filter is set)*/
  bool mUseIntersect;
  /**A spatial index for fast access to a feature subset*/
  geos::STRtree mSpatialIndex;
  /**Stores all the inserted rectangles and features. This is used to clean up the memory in the destructor*/
  std::list< std::pair<geos::Envelope*, QgsFeature*> > mEnvelopesAndFeatures;
  /**Vector where the QgsFeature* of a query are inserted*/
  std::vector<void*>* mSelectedFeatures;
  /**Iterator on the feature vector for use in reset(), getNextFeature(), etc...*/
  std::vector<void*>::iterator mFeatureIterator;
  /**Geometry type of the features in this layer*/
  mutable QGis::WKBTYPE mWKBType;
  /**Source SRS*/
  QgsSpatialRefSys* mSourceSRS;
  /**Stores the minimum/maximum values for each attribute
   The position in the vector is equal to the position of an attribute in the layers attribute vector*/
  std::vector< std::pair<QString, QString> > mMinMaxCash;
  int mFeatureCount;
  /**Flag if provider is valid*/
  bool mValid;

  /**Goes through all the features and their attributes and populates mMinMaxCash with entries*/
  void fillMinMaxCash();
  
  /**Collects information about the field types. Is called internally from QgsWFSProvider::getFeature*/
  int describeFeatureType(const QString& uri, std::vector<QgsField>& fields);

  //encoding specific methods of getFeature
  int getFeatureGET(const QString& uri, const QString& geometryAttribute);
  int getFeaturePOST(const QString& uri, const QString& geometryAttribute);
  int getFeatureSOAP(const QString& uri, const QString& geometryAttribute);
  //encoding specific methods of describeFeatureType
  int describeFeatureTypeGET(const QString& uri, std::vector<QgsField>& fields);
  int describeFeatureTypePOST(const QString& uri, std::vector<QgsField>& fields);
  int describeFeatureTypeSOAP(const QString& uri, std::vector<QgsField>& fields);

  //GML2 specific methods
  int getExtentFromGML2(QgsRect* extent, const QDomElement& wfsCollectionElement) const;
  
  int getFeaturesFromGML2(const QDomElement& wfsCollectionElement, const QString& geometryAttribute);

  int getWkbFromGML2(const QDomNode& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  /**Creates WKB from a <Point> element*/
  int getWkbFromGML2Point(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  /**Creates WKB from a <Polygon> element*/
  int getWkbFromGML2Polygon(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  /**Creates WKB from a <LineString> element*/
  int getWkbFromGML2LineString(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  /**Creates WKB from a <MultiPoint> element*/
  int getWkbFromGML2MultiPoint(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  /**Creates WKB from a <MultiLineString> element*/
  int getWkbFromGML2MultiLineString(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  /**Creates WKB from a <MultiPolygon> element*/
  int getWkbFromGML2MultiPolygon(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  /**Reads the <gml:coordinates> element and extracts the coordinates as points
     @param coords list where the found coordinates are appended
     @param elem the <gml:coordinates> element
     @return 0 in case of success*/
  int readGML2Coordinates(std::list<QgsPoint>& coords, const QDomElement elem) const;
  /**Tries to create a QgsSpatialRefSys object and assign it to mSourceSRS. Returns 0 in case of success*/
  int setSRSFromGML2(const QDomElement& wfsCollectionElement);

  //GML3 specific methods. Not needed at the moment as most servers support GML2
#if 0
  /**Evaluates the <gml:boundedBy> element
   @return 0 in case of success*/
  int getExtentFromGML3(QgsRect* extent, const QDomElement& wfsCollectionElement) const;
  /**Turns GML into QGIS features
   @param wfsCollectionElement reference to the GML parent element
   @param geometryAttribute the name of the attribute containing the geometry
   @param features the vector where pointers to the features are filled (the features have to be deleted after usage)
   @return 0 in case of success*/
  int getFeaturesFromGML3(const QDomElement& wfsCollectionElement, const QString& geometryAttribute, std::vector<QgsFeature*>& features) const;
  /**Turns a GML geometry attribute element (and its contents) into wkb. This function delegates the work to the geometry type specific functions below.
   @param wkb allocated geometry data
   @param wkbSize size of the allocated data
   @param type wkb type (point/multipoint/line/multiline/polygon/multipolygon)
   @return 0 in case of success*/
  int getWkbFromGML3(const QDomNode& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromGML3Point(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromGML3LineString(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromGML3Polygon(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromGML3MultiSurface(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromGML3MultiCurve(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  /**Takes a <gml:pos> or <gml:posList> element and fills the coordinates into the passed list
   @param coords the list where the coordinates are filled into
   @param elem the <gml:pos> or <gml:posList> element
   @return 0 in case of success*/
  int readCoordinatesFromPosList(std::list<QgsPoint>& coords, const QDomElement elem) const;
  
  /**Tries to create a QgsSpatialRefSys object and assign it to mSourceSRS. Returns 0 in case of success*/
  int setSRSFromGML3(const QDomElement& wfsCollectionElement);
#endif //0 methods for GML3
};

#endif
