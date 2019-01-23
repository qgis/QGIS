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

#include <QRegularExpression>

#include "qgsmodule.h"
#include "qgswmsutils.h"
#include "qgsmediancut.h"
#include "qgsserverprojectutils.h"
#include "qgswmsserviceexception.h"

namespace QgsWms
{
  QString ImplementationVersion()
  {
    return QStringLiteral( "1.3.0" );
  }

  QUrl serviceUrl( const QgsServerRequest &request, const QgsProject *project )
  {
    QUrl href;
    if ( project )
    {
      href.setUrl( QgsServerProjectUtils::wmsServiceUrl( *project ) );
    }

    // Build default url
    if ( href.isEmpty() )
    {
      static QSet<QString> sFilter
      {
        QStringLiteral( "REQUEST" ),
        QStringLiteral( "VERSION" ),
        QStringLiteral( "SERVICE" ),
        QStringLiteral( "LAYERS" ),
        QStringLiteral( "STYLES" ),
        QStringLiteral( "SLD_VERSION" ),
        QStringLiteral( "_DC" )
      };

      href = request.originalUrl();
      QUrlQuery q( href );

      for ( auto param : q.queryItems() )
      {
        if ( sFilter.contains( param.first.toUpper() ) )
          q.removeAllQueryItems( param.first );
      }

      href.setQuery( q );
    }

    return  href;
  }


  ImageOutputFormat parseImageFormat( const QString &format )
  {
    if ( format.compare( QLatin1String( "png" ), Qt::CaseInsensitive ) == 0 ||
         format.compare( QLatin1String( "image/png" ), Qt::CaseInsensitive ) == 0 )
    {
      return PNG;
    }
    else if ( format.compare( QLatin1String( "jpg " ), Qt::CaseInsensitive ) == 0  ||
              format.compare( QLatin1String( "image/jpeg" ), Qt::CaseInsensitive ) == 0 )
    {
      return JPEG;
    }
    else
    {
      // lookup for png with mode
      QRegularExpression modeExpr = QRegularExpression( QStringLiteral( "image/png\\s*;\\s*mode=([^;]+)" ),
                                    QRegularExpression::CaseInsensitiveOption );

      QRegularExpressionMatch match = modeExpr.match( format );
      QString mode = match.captured( 1 );
      if ( mode.compare( QLatin1String( "16bit" ), Qt::CaseInsensitive ) == 0 )
        return PNG16;
      if ( mode.compare( QLatin1String( "8bit" ), Qt::CaseInsensitive ) == 0 )
        return PNG8;
      if ( mode.compare( QLatin1String( "1bit" ), Qt::CaseInsensitive ) == 0 )
        return PNG1;
    }

    return UNKN;
  }

  void readLayersAndStyles( const QgsServerRequest::Parameters &parameters, QStringList &layersList, QStringList &stylesList )
  {
    //get layer and style lists from the parameters trying LAYERS and LAYER as well as STYLE and STYLES for GetLegendGraphic compatibility
    layersList = parameters.value( QStringLiteral( "LAYER" ) ).split( ',', QString::SkipEmptyParts );
    layersList = layersList + parameters.value( QStringLiteral( "LAYERS" ) ).split( ',', QString::SkipEmptyParts );
    stylesList = parameters.value( QStringLiteral( "STYLE" ) ).split( ',', QString::SkipEmptyParts );
    stylesList = stylesList + parameters.value( QStringLiteral( "STYLES" ) ).split( ',', QString::SkipEmptyParts );
  }


  // Write image response
  void writeImage( QgsServerResponse &response, QImage &img, const QString &formatStr,
                   int imageQuality )
  {
    ImageOutputFormat outputFormat = parseImageFormat( formatStr );
    QImage  result;
    QString saveFormat;
    QString contentType;
    switch ( outputFormat )
    {
      case PNG:
        result = img;
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case PNG8:
      {
        QVector<QRgb> colorTable;
        medianCut( colorTable, 256, img );
        result = img.convertToFormat( QImage::Format_Indexed8, colorTable,
                                      Qt::ColorOnly | Qt::ThresholdDither |
                                      Qt::ThresholdAlphaDither | Qt::NoOpaqueDetection );
      }
      contentType = "image/png";
      saveFormat = "PNG";
      break;
      case PNG16:
        result = img.convertToFormat( QImage::Format_ARGB4444_Premultiplied );
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case PNG1:
        result = img.convertToFormat( QImage::Format_Mono,
                                      Qt::MonoOnly | Qt::ThresholdDither |
                                      Qt::ThresholdAlphaDither | Qt::NoOpaqueDetection );
        contentType = "image/png";
        saveFormat = "PNG";
        break;
      case JPEG:
        result = img;
        contentType = "image/jpeg";
        saveFormat = "JPEG";
        break;
      default:
        QgsMessageLog::logMessage( QString( "Unsupported format string %1" ).arg( formatStr ) );
        saveFormat = UNKN;
        break;
    }

    if ( outputFormat != UNKN )
    {
      response.setHeader( "Content-Type", contentType );
      if ( saveFormat == "JPEG" )
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
      throw QgsServiceException( "InvalidFormat",
                                 QString( "Output format '%1' is not supported in the GetMap request" ).arg( formatStr ) );
    }
  }

  QgsRectangle parseBbox( const QString &bboxStr )
  {
    QStringList lst = bboxStr.split( ',' );
    if ( lst.count() != 4 )
      return QgsRectangle();

    double d[4];
    bool ok;
    for ( int i = 0; i < 4; i++ )
    {
      lst[i].replace( ' ', '+' );
      d[i] = lst[i].toDouble( &ok );
      if ( !ok )
        return QgsRectangle();
    }
    return QgsRectangle( d[0], d[1], d[2], d[3] );
  }

} // namespace QgsWms


