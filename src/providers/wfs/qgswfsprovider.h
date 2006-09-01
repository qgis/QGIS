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

  /**Makes a GetCapabilities and returns the typenamse and crs supported by the server. This function is static because it is usually used from dialogs when no WFS provider object yet exists.
     @param typenames a list of layers provided by the server
     @param a list of crs supported by the server. The place in the list corresponds to the \
     typenames list (means that the crs list at position 0 is a crs for typename at position 0 etc.)
     @return 0 in case of success*/
  static int getCapabilities(const QString& uri, QgsWFSProvider::REQUEST_ENCODING e, std::list<QString>& typenames, std::list< std::list<QString> >& crs);

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
  QgsRect* mFilter;
  /**Flag if precise intersection test is needed. Otherwise, every feature is returned (even if a filter is set)*/
  bool mUseIntersect;
  /**Stores all the features*/
  std::vector<QgsFeature*> mFeatures;
  /**Iterator on the feature vector for use in reset(), getNextFeature(), etc...*/
  std::vector<QgsFeature*>::iterator mFeatureIterator;
  /**Geometry type of the features in this layer*/
  mutable QGis::WKBTYPE mWKBType;
  /**Source SRS*/
  QgsSpatialRefSys* mSourceSRS;
  
  /**Collects information about the field types. Is called internally from QgsWFSProvider::getFeature*/
  int describeFeatureType(const QString& uri, std::vector<QgsField>& fields);

  //encoding specific methods of getCapabilities
  static int getCapabilitiesGET(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs);
  static int getCapabilitiesPOST(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs);
  static int getCapabilitiesSOAP(const QString& uri, std::list<QString>& typenames, std::list< std::list<QString> >& crs);
  //encoding specific methods of getFeature
  int getFeatureGET(const QString& uri, const QString& geometryAttribute);
  int getFeaturePOST(const QString& uri, const QString& geometryAttribute);
  int getFeatureSOAP(const QString& uri, const QString& geometryAttribute);
  //encoding specific methods of describeFeatureType
  int describeFeatureTypeGET(const QString& uri, std::vector<QgsField>& fields);
  int describeFeatureTypePOST(const QString& uri, std::vector<QgsField>& fields);
  int describeFeatureTypeSOAP(const QString& uri, std::vector<QgsField>& fields);
  /**Evaluates the <gml:boundedBy> element
   @return 0 in case of success*/
  int getExtentFromGML(QgsRect* extent, const QDomElement& wfsCollectionElement) const;
  /**Turns GML into QGIS features
   @param wfsCollectionElement reference to the GML parent element
   @param geometryAttribute the name of the attribute containing the geometry
   @param features the vector where pointers to the features are filled (the features have to be deleted after usage)
   @return 0 in case of success*/
  int getFeaturesFromGML(const QDomElement& wfsCollectionElement, const QString& geometryAttribute, std::vector<QgsFeature*>& features) const;
  /**Turns a GML geometry attribute element (and its contents) into wkb. This function delegates the work to the geometry type specific functions below.
   @param wkb allocated geometry data
   @param wkbSize size of the allocated data
   @param type wkb type (point/multipoint/line/multiline/polygon/multipolygon)
   @return 0 in case of success*/
  int getWkbFromGML(const QDomNode& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromGMLPoint(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromGMLLineString(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromGMLPolygon(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromGMLMultiSurface(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  int getWkbFromMultiCurve(const QDomElement& geometryElement, unsigned char** wkb, int* wkbSize, QGis::WKBTYPE* type) const;
  /**Takes a <gml:pos> or <gml:posList> element and fills the coordinates into the passed list
   @param coords the list where the coordinates are filled into
   @param elem the <gml:pos> or <gml:posList> element
   @return 0 in case of success*/
  int readCoordinatesFromPosList(std::list<QgsPoint>& coords, const QDomElement elem) const;
  
  /**Tries to create a QgsSpatialRefSys object and assign it to mSourceSRS. Returns 0 in case of success*/
  int setSRSFromGML(const QDomElement& wfsCollectionElement);
};

#endif
