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
#include "qgsfields.h"

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
    explicit QgsGPXProvider( const QString &uri = QString() );
    virtual ~QgsGPXProvider();

    /* Functions inherited from QgsVectorDataProvider */

    virtual QgsAbstractFeatureSource *featureSource() const override;
    virtual QString storageType() const override;
    virtual QgsFeatureIterator getFeatures( const QgsFeatureRequest &request ) const override;
    virtual QgsWkbTypes::Type wkbType() const override;
    virtual long featureCount() const override;
    virtual QgsFields fields() const override;
    virtual bool addFeatures( QgsFeatureList &flist, QgsFeatureSink::Flags flags = 0 ) override;
    virtual bool deleteFeatures( const QgsFeatureIds &id ) override;
    virtual bool changeAttributeValues( const QgsChangedAttributesMap &attr_map ) override;
    virtual QgsVectorDataProvider::Capabilities capabilities() const override;
    virtual QVariant defaultValue( int fieldId ) const override;


    /* Functions inherited from QgsDataProvider */

    virtual QgsRectangle extent() const override;
    virtual bool isValid() const override;
    virtual QString name() const override;
    virtual QString description() const override;
    virtual QgsCoordinateReferenceSystem crs() const override;


    /* new functions */

    void changeAttributeValues( QgsGPSObject &obj,
                                const QgsAttributeMap &attrs );

    bool addFeature( QgsFeature &f, QgsFeatureSink::Flags flags = 0 ) override;


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

    QgsGPSData *data = nullptr;

    //! Fields
    QgsFields attributeFields;
    //! map from field index to attribute
    QVector<int> indexToAttr;

    QString mFileName;

    DataType mFeatureType = WaypointType;

    static const char *ATTR[];
    static QVariant::Type attrType[];
    static DataType attrUsed[];
    static const int ATTR_COUNT;

    bool mValid = false;

    friend class QgsGPXFeatureSource;
};

#endif
