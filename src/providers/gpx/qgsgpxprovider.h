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

#ifndef QGSGPXPROVIDER_H
#define QGSGPXPROVIDER_H

#include "qgsvectordataprovider.h"
#include "gpsdata.h"


class QgsFeature;
class QgsField;
class QFile;
class QDomDocument;
class QgsGPSData;

class QgsGPXFeatureIterator;

/**
\class QgsGPXProvider
\brief Data provider for GPX (GPS eXchange) files
* This provider adds the ability to load GPX files as vector layers.
*
*/
class QgsGPXProvider : public QgsVectorDataProvider
{
    Q_OBJECT

  public:

    QgsGPXProvider( QString uri = QString() );
    virtual ~QgsGPXProvider();

    /* Functions inherited from QgsVectorDataProvider */

    virtual QgsAbstractFeatureSource* featureSource() const;

    /**
     *   Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest& request );

    /**
     * Get feature type.
     * @return int representing the feature type
     */
    virtual QGis::WkbType geometryType() const;

    /**
     * Number of features in the layer
     * @return long containing number of features
     */
    virtual long featureCount() const;

    /**
     * Get the field information for the layer
     */
    virtual const QgsFields& fields() const;

    /**
     * Adds a list of features
     * @return true in case of success and false in case of failure
     */
    virtual bool addFeatures( QgsFeatureList & flist );

    /**
     * Deletes a feature
     * @param id list containing feature ids to delete
     * @return true in case of success and false in case of failure
     */
    virtual bool deleteFeatures( const QgsFeatureIds & id );

    /**
     * Changes attribute values of existing features.
     * @param attr_map a map containing changed attributes
     * @return true in case of success and false in case of failure
     */
    virtual bool changeAttributeValues( const QgsChangedAttributesMap & attr_map );

    virtual int capabilities() const;

    /**
     * Returns the default value for field specified by @c fieldId
     */
    virtual QVariant defaultValue( int fieldId );


    /* Functions inherited from QgsDataProvider */

    /** Return the extent for this data layer
     */
    virtual QgsRectangle extent();

    /**Returns true if this is a valid delimited file
     */
    virtual bool isValid();

    /** return a provider name */
    virtual QString name() const;

    /** return description */
    virtual QString description() const;

    virtual QgsCoordinateReferenceSystem crs();


    /* new functions */

    void changeAttributeValues( QgsGPSObject& obj,
                                const QgsAttributeMap& attrs );

    /** Adds one feature (used by addFeatures()) */
    bool addFeature( QgsFeature& f );


    enum DataType
    {
      WaypointType = 1,
      RouteType = 2,
      TrackType = 4,

      TrkRteType = RouteType | TrackType,
      AllType = WaypointType | RouteType | TrackType

    };

    enum Attribute { NameAttr = 0, EleAttr, SymAttr, NumAttr,
                     CmtAttr, DscAttr, SrcAttr, URLAttr, URLNameAttr
                 };

  private:

    QgsGPSData* data;

    //! Fields
    QgsFields attributeFields;
    //! map from field index to attribute
    QVector<int> indexToAttr;

    QString mFileName;

    DataType mFeatureType;

    static const char* attr[];
    static QVariant::Type attrType[];
    static DataType attrUsed[];
    static const int attrCount;

    bool mValid;
    long mNumberFeatures;

    struct wkbPoint
    {
      char byteOrder;
      unsigned wkbType;
      double x;
      double y;
    };
    wkbPoint mWKBpt;

    friend class QgsGPXFeatureSource;
};

#endif
