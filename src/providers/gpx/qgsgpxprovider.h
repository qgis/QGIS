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


#include "qgsvectordataprovider.h"
#include "gpsdata.h"


class QgsFeature;
class QgsField;
class QFile;
class QDomDocument;
class QgsGPSData;


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

    /**
     *   Returns the permanent storage type for this layer as a friendly name.
     */
    virtual QString storageType() const;

    /** Select features based on a bounding rectangle. Features can be retrieved with calls to nextFeature.
     * @param fetchAttributes list of attributes which should be fetched
     * @param rect spatial filter
     * @param fetchGeometry true if the feature geometry should be fetched
     * @param useIntersect true if an accurate intersection test should be used,
     *                     false if a test based on bounding box is sufficient
     */
    virtual void select( QgsAttributeList fetchAttributes = QgsAttributeList(),
                         QgsRectangle rect = QgsRectangle(),
                         bool fetchGeometry = true,
                         bool useIntersect = false );

    /**
     * Get the next feature resulting from a select operation.
     * @param feature feature which will receive data from the provider
     * @return true when there was a feature to fetch, false when end was hit
     */
    virtual bool nextFeature( QgsFeature& feature );

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
     * Get the number of fields in the layer
     */
    virtual uint fieldCount() const;

    /**
     * Get the field information for the layer
     */
    virtual const QgsFieldMap & fields() const;

    /** Restart reading features from previous select operation */
    virtual void rewind();

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

    /**
     * Check to see if the point is withn the selection
     * rectangle
     * @param x X value of point
     * @param y Y value of point
     * @return True if point is within the rectangle
     */
    bool boundsCheck( double x, double y );


  private:

    QgsGPSData* data;

    //! Fields
    QgsFieldMap attributeFields;

    QString mFileName;

    enum { WaypointType, RouteType, TrackType } mFeatureType;
    enum Attribute { NameAttr = 0, EleAttr, SymAttr, NumAttr,
                     CmtAttr, DscAttr, SrcAttr, URLAttr, URLNameAttr
                 };
    static const char* attr[];
    //! Current selection rectangle
    QgsRectangle *mSelectionRectangle;
    bool mValid;
    long mNumberFeatures;

    //! Current waypoint iterator
    QgsGPSData::WaypointIterator mWptIter;
    //! Current route iterator
    QgsGPSData::RouteIterator mRteIter;
    //! Current track iterator
    QgsGPSData::TrackIterator mTrkIter;

    struct wkbPoint
    {
      char byteOrder;
      unsigned wkbType;
      double x;
      double y;
    };
    wkbPoint mWKBpt;

};
