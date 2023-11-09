/***************************************************************************
                             qgsmaplayerserverproperties.h
                             ------------------
  begin                : June 21, 2021
  copyright            : (C) 2021 by Etienne Trimaille
  email                : etrimaille at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPLAYERSERVERPROPERTIES_H
#define QGSMAPLAYERSERVERPROPERTIES_H

#include "qgis_sip.h"
#include "qgis_core.h"
#include <QMap>
#include <QString>
#include <QMetaType>
#include <QVariant>

class QgsMapLayer;

class QDomNode;
class QDomDocument;

/**
 * \ingroup core
 * \brief Manages QGIS Server properties for a map layer
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsServerMetadataUrlProperties
{
    Q_GADGET

  public:

    /**
     * \brief MetadataUrl structure.
     * MetadataUrl is a link to the detailed, standardized metadata about the data.
     * \ingroup core
     */
    struct CORE_EXPORT MetadataUrl
    {

      /**
       * Constructor for MetadataUrl.
       */
      MetadataUrl( const QString &url = QString(), const QString &type = QString(), const QString &format = QString() )
        : url( url )
        , type( type )
        , format( format )
      {}

      /**
       * URL of the link
       */
      QString url;

      /**
       * Link type. Suggested to use FGDC or TC211.
       */
      QString type;

      /**
       * Format specification of online resource. It is strongly suggested to either use text/plain or text/xml.
       */
      QString format;

      // TODO c++20 - replace with = default

      //! Compare two MetadataUrl structure.
      bool operator==( const QgsServerMetadataUrlProperties::MetadataUrl &other ) const;
    };

    virtual ~QgsServerMetadataUrlProperties() = default;

    /**
     * Returns a list of metadataUrl resources associated for the layer.
     * \returns the list of metadata URLs
     * \see setMetadataUrls()
     */
    QList<QgsServerMetadataUrlProperties::MetadataUrl> metadataUrls() const { return mMetadataUrls; };

    /**
     * Sets a the list of metadata URL for the layer
     * \see metadataUrls()
     * \see addMetadataUrl()
     */
    void setMetadataUrls( const QList<QgsServerMetadataUrlProperties::MetadataUrl> &metaUrls ) { mMetadataUrls = metaUrls; };

    /**
     * Add a metadataUrl for the layer
     * \see setMetadataUrls()
     */
    void addMetadataUrl( const QgsServerMetadataUrlProperties::MetadataUrl &metaUrl ) { mMetadataUrls << metaUrl; };

    //! Gets the parent layer
    virtual const QgsMapLayer *layer() const = 0;

  protected:
    //! Saves server properties to xml under the layer node
    void writeXml( QDomNode &layer_node, QDomDocument &document ) const SIP_SKIP;

    //! Reads server properties from project file.
    void readXml( const QDomNode &layer_node ) SIP_SKIP;

    /**
     * Copy properties to another instance
     *
     * \param properties The properties to copy to
     */
    void copyTo( QgsServerMetadataUrlProperties *properties ) const SIP_SKIP;

    //! Reset properties to default
    void reset() SIP_SKIP;

  private:
    QList<MetadataUrl> mMetadataUrls;

};


/**
 * \ingroup core
 * \brief Manages QGIS Server properties for Wms dimensions
 * \since QGIS 3.22
 */
class CORE_EXPORT QgsServerWmsDimensionProperties
{
    Q_GADGET

  public:

    /**
     * Predefined/Restricted WMS Dimension name
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
                                 const int &dimDefaultDisplayType = QgsServerWmsDimensionProperties::WmsDimensionInfo::AllValues,
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

    virtual ~QgsServerWmsDimensionProperties() = default;

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
    bool addWmsDimension( const QgsServerWmsDimensionProperties::WmsDimensionInfo &wmsDimInfo );

    /**
     * Removes a QGIS Server WMS Dimension
     * \returns TRUE if QGIS Server WMS Dimension was found and successfully removed
     * \since QGIS 3.10
     */
    bool removeWmsDimension( const QString &wmsDimName );

    /**
     * Set WMS Dimensions
     *
     * \param dimensions the new dimensions that replace the current ones.
     */
    void setWmsDimensions( const QList<QgsServerWmsDimensionProperties::WmsDimensionInfo> &dimensions );

    /**
     * Returns the QGIS Server WMS Dimension list.
     * \since QGIS 3.10
     */
    const QList<QgsServerWmsDimensionProperties::WmsDimensionInfo> wmsDimensions() const;

    //! Gets the parent layer
    virtual const QgsMapLayer *layer() const = 0;

  protected:

    /**
     * Saves server properties to xml under the layer node
     */
    void writeXml( QDomNode &layer_node, QDomDocument &document ) const SIP_SKIP;

    /**
     * Reads server properties from project file.
     */
    void readXml( const QDomNode &layer_node ) SIP_SKIP;

    /**
     * Copy properties to another instance
     *
     * \param properties The properties to copy to
     */
    void copyTo( QgsServerWmsDimensionProperties *properties ) const SIP_SKIP;

    /**
     * Reset properties to default
     */
    void reset() SIP_SKIP;

  private:                       // Private attributes
    //!stores QGIS Server WMS Dimension definitions
    QList<WmsDimensionInfo> mWmsDimensions;
};


/**
 * \ingroup core
 * \brief Manages QGIS Server properties for a map layer
 * \since QGIS 3.10
 */
class CORE_EXPORT QgsMapLayerServerProperties: public QgsServerMetadataUrlProperties, public QgsServerWmsDimensionProperties
{
    Q_GADGET

  public:

    /**
     * Constructor - Creates a Map Layer QGIS Server Properties
     *
     * \param layer The map layer
     */
    QgsMapLayerServerProperties( QgsMapLayer *layer = nullptr );

    /**
     * Copy properties to another instance
     *
     * \param properties The properties to copy to
     */
    void copyTo( QgsMapLayerServerProperties *properties ) const;

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

    /**
     * Reset properties to default
     * \since QGIS 3.22
     */
    void reset();

    //! Gets the parent layer
    const QgsMapLayer *layer() const override { return mLayer; };

  private:
    QgsMapLayer *mLayer = nullptr;
};

/**
 * \ingroup core
 * \brief Convenient class for API compatibility
 * \deprecated since QGIS 3.22
 * \since QGIS 3.10
 */
// XXX How to make a proper SIP type alias ?
//using QgsVectorLayerServerProperties = QgsMapLayerServerProperties;
class CORE_EXPORT QgsVectorLayerServerProperties: public QgsMapLayerServerProperties
{
    Q_GADGET
};

#endif // QGSMAPLAYERSERVERPROPERTIES_H

