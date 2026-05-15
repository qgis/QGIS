/***************************************************************************
                              qgswmsutils.cpp
                              -------------------------
  begin                : December 20 , 2016
  copyright            : (C) 2007 by Marco Hugentobler  ( parts from qgswmshandler)
                         (C) 2014 by Alessandro Pasotti ( parts from qgswmshandler)
                         (C) 2016 by David Marteau
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
                         david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                  *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsutils.h"

#include "qgslayertree.h"
#include "qgsmediancut.h"
#include "qgsmodule.h"
#include "qgsproject.h"
#include "qgsserverprojectutils.h"
#include "qgswmsserviceexception.h"

#include <QRegularExpression>
#include <QString>

using namespace Qt::StringLiterals;

namespace QgsWms
{
  QString implementationVersion()
  {
    return u"1.3.0"_s;
  }

  QUrl serviceUrl( const QgsServerRequest &request, const QgsProject *project, const QgsServerSettings &settings )
  {
    QUrl href;
    href.setUrl( QgsServerProjectUtils::wmsServiceUrl( project ? *project : *QgsProject::instance(), request, settings ) );

    // Build default url
    if ( href.isEmpty() )
    {
      static const QSet<QString> sFilter { u"REQUEST"_s, u"VERSION"_s, u"SERVICE"_s, u"LAYERS"_s, u"STYLES"_s, u"SLD_VERSION"_s, u"_DC"_s };

      href = request.originalUrl();
      QUrlQuery q( href );

      const QList<QPair<QString, QString>> queryItems = q.queryItems();
      for ( const QPair<QString, QString> &param : queryItems )
      {
        if ( sFilter.contains( param.first.toUpper() ) )
          q.removeAllQueryItems( param.first );
      }

      href.setQuery( q );
    }

    return href;
  }


  ImageOutputFormat parseImageFormat( const QString &format )
  {
    if ( format.compare( "png"_L1, Qt::CaseInsensitive ) == 0 || format.compare( "image/png"_L1, Qt::CaseInsensitive ) == 0 )
    {
      return ImageOutputFormat::PNG;
    }
    else if ( format.compare( "jpg "_L1, Qt::CaseInsensitive ) == 0 || format.compare( "image/jpeg"_L1, Qt::CaseInsensitive ) == 0 )
    {
      return ImageOutputFormat::JPEG;
    }
    else if ( format.compare( "webp"_L1, Qt::CaseInsensitive ) == 0 || format.compare( "image/webp"_L1, Qt::CaseInsensitive ) == 0 )
    {
      return ImageOutputFormat::WEBP;
    }
    else
    {
      // lookup for png with mode
      const thread_local QRegularExpression modeExpr = QRegularExpression( u"image/png\\s*;\\s*mode=([^;]+)"_s, QRegularExpression::CaseInsensitiveOption );

      const QRegularExpressionMatch match = modeExpr.match( format );
      const QString mode = match.captured( 1 );
      if ( mode.compare( "16bit"_L1, Qt::CaseInsensitive ) == 0 )
        return ImageOutputFormat::PNG16;
      if ( mode.compare( "8bit"_L1, Qt::CaseInsensitive ) == 0 )
        return ImageOutputFormat::PNG8;
      if ( mode.compare( "1bit"_L1, Qt::CaseInsensitive ) == 0 )
        return ImageOutputFormat::PNG1;
    }

    return ImageOutputFormat::Unknown;
  }

  // Write image response
  void writeImage( QgsServerResponse &response, QImage &img, const QString &formatStr, int imageQuality )
  {
    const ImageOutputFormat outputFormat = parseImageFormat( formatStr );
    QImage result;
    QString saveFormat;
    QString contentType;
    switch ( outputFormat )
    {
      case ImageOutputFormat::PNG:
        result = img;
        contentType = u"image/png"_s;
        saveFormat = u"PNG"_s;
        break;
      case ImageOutputFormat::PNG8:
      {
        QVector<QRgb> colorTable;

        // Rendering is made with the format QImage::Format_ARGB32_Premultiplied
        // So we need to convert it in QImage::Format_ARGB32 in order to properly build
        // the color table.
        const QImage img256 = img.convertToFormat( QImage::Format_ARGB32 );
        medianCut( colorTable, 256, img256 );
        result = img256.convertToFormat( QImage::Format_Indexed8, colorTable, Qt::ColorOnly | Qt::ThresholdDither | Qt::ThresholdAlphaDither | Qt::NoOpaqueDetection );
      }
        contentType = u"image/png"_s;
        saveFormat = u"PNG"_s;
        break;
      case ImageOutputFormat::PNG16:
        result = img.convertToFormat( QImage::Format_ARGB4444_Premultiplied );
        contentType = u"image/png"_s;
        saveFormat = u"PNG"_s;
        break;
      case ImageOutputFormat::PNG1:
        result = img.convertToFormat( QImage::Format_Mono, Qt::MonoOnly | Qt::ThresholdDither | Qt::ThresholdAlphaDither | Qt::NoOpaqueDetection );
        contentType = u"image/png"_s;
        saveFormat = u"PNG"_s;
        break;
      case ImageOutputFormat::JPEG:
        result = img;
        contentType = u"image/jpeg"_s;
        saveFormat = u"JPEG"_s;
        break;
      case ImageOutputFormat::WEBP:
        result = img;
        contentType = u"image/webp"_s;
        saveFormat = u"WEBP"_s;
        break;
      case ImageOutputFormat::Unknown:
        QgsMessageLog::logMessage( u"Unsupported format string %1"_s.arg( formatStr ) );
        saveFormat = u"Unknown"_s;
        break;
    }

    // Preserve DPI, some conversions, in particular the one for 8bit will drop this information
    result.setDotsPerMeterX( img.dotsPerMeterX() );
    result.setDotsPerMeterY( img.dotsPerMeterY() );

    if ( outputFormat != ImageOutputFormat::Unknown )
    {
      response.setHeader( "Content-Type", contentType );
      if ( saveFormat == "JPEG"_L1 || saveFormat == "WEBP"_L1 )
      {
        result.save( response.io(), qPrintable( saveFormat ), imageQuality );
      }
      else
      {
        result.save( response.io(), qPrintable( saveFormat ) );
      }
    }
    else
    {
      QgsWmsParameter parameter( QgsWmsParameter::FORMAT );
      parameter.mValue = formatStr;
      throw QgsBadRequestException( QgsServiceException::OGC_InvalidFormat, parameter );
    }
  }

  /**
   * Collects the \a acceptableLayersAndRequestNames recursively, a hash of all the layers that can be rendered and for each a list of the layer names requesting it.
   * It needs the \a project for properties and the \a group to analyze the current layer tree. Also the \a requestedLayerNames. If no \a requestedLayerNames are passed,
   * you will receive back all the layers except the ones hidden in an opaque group.
   * When an opaque group is in the \a requestedLayerNames, the children of this opaque group are passed back as well.
   * The \a requestedParentNames are used for the recursive collecting of the list of requested layers and groups.
   * When the \a groupIsAnOpaqueChild, it should continue to allow the layers to be rendered but not add the following group and layer names to the list of requested names.
  */
  void _collectAcceptableLayersAndRequestNames(
    QHash<const QgsMapLayer *, QStringList> &acceptableLayersAndRequestNames,
    const QgsProject &project,
    const QStringList &requestedLayerNames,
    const QgsLayerTreeGroup *group,
    QStringList requestedParentNames = QStringList(),
    bool groupIsAnOpaqueChild = false
  )
  {
    //get group nickname
    QString groupName = group->serverProperties()->shortName();
    if ( groupName.isEmpty() )
      groupName = group->name();

    bool projectIsRequested = ( requestedLayerNames.contains( QgsServerProjectUtils::wmsRootName( project ) ) || requestedLayerNames.contains( project.title() ) );
    bool groupIsRequested = requestedLayerNames.contains( groupName );

    // append the group to the list, when it's explicitly requested and it's not already a child of an opaque group
    if ( groupIsRequested && !groupIsAnOpaqueChild )
      requestedParentNames << groupName;

    // the group should not be opaque or explicitly requested (by the groupname or by the project name)
    if ( ( group->wmsGroupRequestMode() != Qgis::WmsGroupRequestMode::Opaque ) || groupIsRequested || projectIsRequested )
    {
      // when the current group is opaque or the previous groups have been opaque it is an opaque child and should not be requestable
      bool isOpaqueChild = ( group->wmsGroupRequestMode() == Qgis::WmsGroupRequestMode::Opaque ) || groupIsAnOpaqueChild;

      for ( QgsLayerTreeNode *child : group->children() )
      {
        if ( QgsLayerTree::isGroup( child ) )
        {
          auto subgroup = static_cast<const QgsLayerTreeGroup *>( child );
          _collectAcceptableLayersAndRequestNames( acceptableLayersAndRequestNames, project, requestedLayerNames, subgroup, requestedParentNames, isOpaqueChild );
        }
        else if ( QgsLayerTree::isLayer( child ) )
        {
          auto layernode = static_cast<const QgsLayerTreeLayer *>( child );
          const QgsMapLayer *layer = layernode->layer();
          if ( !layer )
            continue;

          //get layer nickname
          QString name = layer->serverProperties()->shortName();
          if ( QgsServerProjectUtils::wmsUseLayerIds( project ) )
          {
            name = layer->id();
          }
          else if ( name.isEmpty() )
          {
            name = layer->name();
          }

          QStringList requestedNames = requestedParentNames;
          // when the layer is explicitly requested and it's not an opaque child, then add it to the requested names
          if ( requestedLayerNames.contains( name ) && !isOpaqueChild )
          {
            requestedNames << name;
          }
          // we add the layer to the map when it's requested (or no requestedLayerNames are passed)
          if ( !requestedNames.isEmpty() || requestedLayerNames.isEmpty() || projectIsRequested )
            acceptableLayersAndRequestNames.insert( layer, requestedNames );
        }
      }
    }
  }

  void collectAcceptableLayersAndRequestNames( QHash<const QgsMapLayer *, QStringList> &acceptableLayersAndRequestNames, const QgsProject &project, const QStringList &requestedLayerNames )
  {
    //Call function used for recursive collect based on the layer tree root
    _collectAcceptableLayersAndRequestNames( acceptableLayersAndRequestNames, project, requestedLayerNames, project.layerTreeRoot() );
  }

} // namespace QgsWms
