/***************************************************************************
                             qgsvectorlayerserverproperties.h
                              ------------------
  begin                : August 23, 2019
  copyright            : (C) 2019 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERSERVERPROPERTIES_H
#define QGSVECTORLAYERSERVERPROPERTIES_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include <QMap>
#include <QString>
#include <QMetaType>
#include <QVariant>

class QgsVectorLayer;

class QDomNode;
class QDomDocument;

/**
 * \ingroup core
 * Manages QGIS Server properties for a vector layer
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsVectorLayerServerProperties
{
    Q_GADGET

  public:

    /**
     * Predefined/Restricted WMS Dimension name
     * \since QGIS 3.10
     */
    enum PredefinedWmsDimensionName
    {
      TIME,
      DATE,
      ELEVATION
    };
    Q_ENUM( PredefinedWmsDimensionName )

    /**
     * Setting to define QGIS Server WMS Dimension.
     * \since QGIS 3.10
     */
    struct CORE_EXPORT WmsDimensionInfo
    {

      /**
       * Selection behavior for QGIS Server WMS Dimension default display
       * \since QGIS 3.10
       */
      enum DefaultDisplay
      {
        AllValues = 0, //!< Display all values of the dimension
        MinValue = 1, //!< Add selection to current selection
        MaxValue = 2, //!< Modify current selection to include only select features which match
        ReferenceValue = 3, //!< Remove from current selection
      };

      /**
       * Constructor for WmsDimensionInfo.
       */
      explicit WmsDimensionInfo( const QString &dimName,
                                 const QString &dimFieldName,
                                 const QString &dimEndFieldName = QString(),
                                 const QString &dimUnits = QString(),
                                 const QString &dimUnitSymbol = QString(),
                                 const int &dimDefaultDisplayType = QgsVectorLayerServerProperties::WmsDimensionInfo::AllValues,
                                 const QVariant &dimReferenceValue = QVariant() )
        : name( dimName )
        , fieldName( dimFieldName )
        , endFieldName( dimEndFieldName )
        , units( dimUnits )
        , unitSymbol( dimUnitSymbol )
        , defaultDisplayType( dimDefaultDisplayType )
        , referenceValue( dimReferenceValue )
      {}
      QString name;
      QString fieldName;
      QString endFieldName;
      QString units;
      QString unitSymbol;
      int defaultDisplayType;
      QVariant referenceValue;
    };

    /**
     * Constructor - Creates a Vector Layer QGIS Server Properties
     *
     * \param layer  The vector layer
     */
    QgsVectorLayerServerProperties( QgsVectorLayer *layer = nullptr );

    /**
     * Returns WMS Dimension default display labels
     * \since QGIS 3.10
     */
    static QMap<int, QString> wmsDimensionDefaultDisplayLabels();

    /**
     * Adds a QGIS Server WMS Dimension
     * \param wmsDimInfo QGIS Server WMS Dimension object with, name, field, etc
     * \returns TRUE if QGIS Server WMS Dimension has been successfully added
     * \since QGIS 3.10
     */
    bool addWmsDimension( const QgsVectorLayerServerProperties::WmsDimensionInfo &wmsDimInfo );

    /**
     * Removes a QGIS Server WMS Dimension
     * \returns TRUE if QGIS Server WMS Dimension was found and successfully removed
     * \since QGIS 3.10
     */
    bool removeWmsDimension( const QString &wmsDimName );

    /**
     * Returns the QGIS Server WMS Dimension list.
     * \since QGIS 3.10
     */
    const QList<QgsVectorLayerServerProperties::WmsDimensionInfo> wmsDimensions() const;


    /**
     * Saves server properties to xml under the layer node
     * \since QGIS 3.10
     */
    void writeXml( QDomNode &layer_node, QDomDocument &document ) const;

    /**
     * Reads server properties from project file.
     * \since QGIS 3.10
     */
    void readXml( const QDomNode &layer_node );

  private:                       // Private attributes

    QgsVectorLayer *mLayer = nullptr;

    //!stores QGIS Server WMS Dimension definitions
    QList<QgsVectorLayerServerProperties::WmsDimensionInfo> mWmsDimensions;
};

#endif // QGSVECTORLAYERSERVERPROPERTIES_H
