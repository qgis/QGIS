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
class CORE_EXPORT QgsMapLayerServerProperties
{
    Q_GADGET

  public:

    /**
     * \brief MetadataUrl structure.
     * MetadataUrl is a a link to the detailed, standardized metadata about the data.
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

      /**
       * Compare two MetadataUrl structure.
       */
      bool operator==( const QgsMapLayerServerProperties::MetadataUrl &other ) const;
    };

    /**
     * Constructor - Creates a Map Layer QGIS Server Properties
     *
     * \param layer The map layer
     */
    QgsMapLayerServerProperties( QgsMapLayer *layer = nullptr );

    /**
     * Returns a list of metadataUrl resources associated for the layer.
     * \returns the list of metadata URLs
     * \see setMetadataUrls()
     */
    QList <QgsMapLayerServerProperties::MetadataUrl> metadataUrls() const { return mMetadataUrls; };


    /**
     * Sets a the list of metadata URL for the layer
     * \see metadataUrls()
     * \see addMetadataUrl()
     */
    void setMetadataUrls( const QList<QgsMapLayerServerProperties::MetadataUrl> &metaUrls ) { mMetadataUrls = metaUrls; };

    /**
     * Add a metadataUrl for the layer
     * \see setMetadataUrls()
     */
    void addMetadataUrl( const MetadataUrl &metaUrl ) { mMetadataUrls << metaUrl; };

    /**
     * Saves server properties to xml under the layer node
     */
    void writeXml( QDomNode &layer_node, QDomDocument &document ) const;

    /**
     * Reads server properties from project file.
     */
    void readXml( const QDomNode &layer_node );

  private:
    QgsMapLayer *mLayer = nullptr;
    QList< QgsMapLayerServerProperties::MetadataUrl > mMetadataUrls;

};

#endif // QGSMAPLAYERSERVERPROPERTIES_H
