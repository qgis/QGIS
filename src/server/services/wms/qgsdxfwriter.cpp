/***************************************************************************
                        qgsdxfexport.cpp
  -------------------------------------------------------------------
Date                 : 20 December 2016
Copyright            : (C) 2015 by
email                : marco.hugentobler at sourcepole dot com (original code)
Copyright            : (C) 2016 by
email                : david dot marteau at 3liz dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmodule.h"
#include "qgsdxfwriter.h"
#include "qgswmsutils.h"
#include "qgswmsservertransitional.h"
#include "qgsmaplayer.h"
#include "qgsvectorlayer.h"
#include "qgsdxfexport.h"

namespace QgsWms
{

  namespace
  {

    QMap<QString, QString> parseFormatOptions( const QString& optionString )
    {
      QMap<QString, QString> options;

      QStringList optionsList = optionString.split( QStringLiteral( ";" ) );
      QStringList::const_iterator optionsIt = optionsList.constBegin();
      for ( ; optionsIt != optionsList.constEnd(); ++optionsIt )
      {
        int equalIdx = optionsIt->indexOf( QLatin1String( ":" ) );
        if ( equalIdx > 0 && equalIdx < ( optionsIt->length() - 1 ) )
        {
          options.insert( optionsIt->left( equalIdx ).toUpper(),
                          optionsIt->right( optionsIt->length() - equalIdx - 1 ).toUpper() );
        }
      }
      return options;
    }

    void readDxfLayerSettings( QgsWmsServer& server, QgsWmsConfigParser* configParser,
                               QList< QPair<QgsVectorLayer *, int > >& layers,
                               const QMap<QString, QString>& options )
    {
      QSet<QString> wfsLayers = QSet<QString>::fromList( configParser->wfsLayerNames() );

      QStringList layerAttributes;
      QMap<QString, QString>::const_iterator layerAttributesIt = options.find( QStringLiteral( "LAYERATTRIBUTES" ) );
      if ( layerAttributesIt != options.constEnd() )
      {
        layerAttributes = options.value( QStringLiteral( "LAYERATTRIBUTES" ) ).split( QStringLiteral( "," ) );
      }

      //LAYERS and STYLES
      QStringList layerList, styleList;
      server.readLayersAndStyles( layerList, styleList );

      for ( int i = 0; i < layerList.size(); ++i )
      {
        QString layerName = layerList.at( i );
        QString styleName;
        if ( styleList.size() > i )
        {
          styleName = styleList.at( i );
        }

        QList<QgsMapLayer*> layerList = configParser->mapLayerFromStyle( layerName, styleName );
        QList<QgsMapLayer*>::const_iterator layerIt = layerList.constBegin();
        for ( ; layerIt != layerList.constEnd(); ++layerIt )
        {
          if ( !( *layerIt ) )
          {
            continue;
          }

          //vector layer?
          if (( *layerIt )->type() != QgsMapLayer::VectorLayer )
          {
            continue;
          }

          QgsVectorLayer* vlayer = static_cast<QgsVectorLayer*>( *layerIt );

          int layerAttribute = -1;
          if ( layerAttributes.size() > i )
          {
            layerAttribute = vlayer->pendingFields().indexFromName( layerAttributes.at( i ) );
          }

          //only wfs layers are allowed to be published
          if ( !wfsLayers.contains( vlayer->name() ) )
          {
            continue;
          }

          layers.append( qMakePair( vlayer, layerAttribute ) );
        }
      }
    }

  }

  void writeAsDxf( QgsServerInterface* serverIface,  const QString& version, const QgsServerRequest& request, QgsServerResponse& response )
  {
    Q_UNUSED( version );

    QgsWmsConfigParser* configParser = getConfigParser( serverIface );

    QgsDxfExport dxf;
    QgsServerRequest::Parameters params = request.parameters();

    QgsRectangle extent = parseBbox( params.value( QStringLiteral( "BBOX" ) ) );
    dxf.setExtent( extent );

    QMap<QString, QString> formatOptionsMap = parseFormatOptions( params.value( QStringLiteral( "FORMAT_OPTIONS" ) ) );

    QgsWmsServer server( serverIface->configFilePath(),
                         *serverIface->serverSettings(),
                         params,
                         configParser,
                         serverIface->accessControls() );

    QList< QPair<QgsVectorLayer *, int > > layers;
    readDxfLayerSettings( server, configParser, layers, formatOptionsMap );
    dxf.addLayers( layers );

    dxf.setLayerTitleAsName( formatOptionsMap.contains( QStringLiteral( "USE_TITLE_AS_LAYERNAME" ) ) );

    //MODE
    QMap<QString, QString>::const_iterator modeIt = formatOptionsMap.find( QStringLiteral( "MODE" ) );

    QgsDxfExport::SymbologyExport se;
    if ( modeIt == formatOptionsMap.constEnd() )
    {
      se = QgsDxfExport::NoSymbology;
    }
    else
    {
      if ( modeIt->compare( QLatin1String( "SymbolLayerSymbology" ), Qt::CaseInsensitive ) == 0 )
      {
        se = QgsDxfExport::SymbolLayerSymbology;
      }
      else if ( modeIt->compare( QLatin1String( "FeatureSymbology" ), Qt::CaseInsensitive ) == 0 )
      {
        se = QgsDxfExport::FeatureSymbology;
      }
      else
      {
        se = QgsDxfExport::NoSymbology;
      }
    }
    dxf.setSymbologyExport( se );

    //SCALE
    QMap<QString, QString>::const_iterator scaleIt = formatOptionsMap.find( QStringLiteral( "SCALE" ) );
    if ( scaleIt != formatOptionsMap.constEnd() )
    {
      dxf.setSymbologyScaleDenominator( scaleIt->toDouble() );
    }

    QString codec = QStringLiteral( "ISO-8859-1" );
    QMap<QString, QString>::const_iterator codecIt = formatOptionsMap.find( QStringLiteral( "CODEC" ) );
    if ( codecIt != formatOptionsMap.constEnd() )
    {
      codec = formatOptionsMap.value( QStringLiteral( "CODEC" ) );
    }

    // Write output
    response.setHeader( "Content-Type", "application/dxf" );
    dxf.writeToFile( response.io(), codec );
  }


} // samespace QgsWms
