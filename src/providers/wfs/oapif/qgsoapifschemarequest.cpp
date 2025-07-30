/***************************************************************************
    qgsoapifschemarequest.cpp
    -------------------------
    begin                : March 2025
    copyright            : (C) 2025 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <nlohmann/json.hpp>
using namespace nlohmann;

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsoapifschemarequest.h"
#include "moc_qgsoapifschemarequest.cpp"
#include "qgsoapifutils.h"
#include "qgswfsconstants.h"

#include <algorithm>
#include <map>

#include <QTextCodec>

QgsOapifSchemaRequest::QgsOapifSchemaRequest( const QgsDataSourceUri &uri )
  : QgsBaseNetworkRequest( QgsAuthorizationSettings( uri.username(), uri.password(), QgsHttpHeaders(), uri.authConfigId() ), "OAPIF" )
{
  // Using Qt::DirectConnection since the download might be running on a different thread.
  // In this case, the request was sent from the main thread and is executed with the main
  // thread being blocked in future.waitForFinished() so we can run code on this object which
  // lives in the main thread without risking havoc.
  connect( this, &QgsBaseNetworkRequest::downloadFinished, this, &QgsOapifSchemaRequest::processReply, Qt::DirectConnection );
}

const QgsOapifSchemaRequest::Schema &QgsOapifSchemaRequest::schema( const QUrl &schemaUrl )
{
  mUrl = schemaUrl;
  sendGET( schemaUrl, QString( "application/schema+json" ), /*synchronous=*/true, /*forceRefresh=*/false );
  return mSchema;
}

QString QgsOapifSchemaRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of schema failed: %1" ).arg( reason );
}

void QgsOapifSchemaRequest::processReply()
{
  if ( mErrorCode != QgsBaseNetworkRequest::NoError )
  {
    return;
  }
  const QByteArray &buffer = mResponse;
  if ( buffer.isEmpty() )
  {
    mErrorMessage = tr( "empty response" );
    mErrorCode = QgsBaseNetworkRequest::ServerExceptionError;
    return;
  }

  QgsDebugMsgLevel( QStringLiteral( "parsing Schema response: " ) + buffer, 4 );

  QTextCodec::ConverterState state;
  QTextCodec *codec = QTextCodec::codecForName( "UTF-8" );
  Q_ASSERT( codec );

  const QString utf8Text = codec->toUnicode( buffer.constData(), buffer.size(), &state );
  if ( state.invalidChars != 0 )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mErrorMessage = errorMessageWithReason( tr( "Invalid UTF-8 content" ) );
    return;
  }

  // Key is the explicit x-ogc-propertySeq or a synthetized one
  std::map<int, QgsField> fields;
  int seqNumber = 1;
  int largestSeqNumber = 1;
  bool seqNumbersAreOk = true;
  try
  {
    const json j = json::parse( utf8Text.toStdString() );

    if ( j.is_object() && j.contains( "properties" ) )
    {
      const json jProperties = j["properties"];
      if ( jProperties.is_object() )
      {
        for ( const auto &[key, val] : jProperties.items() )
        {
          if ( val.is_object() && val.contains( "type" ) )
          {
            QgsField field( QString::fromStdString( key ), QMetaType::Type::QString );

            const json jType = val["type"];
            if ( jType.is_string() )
            {
              const QString type = QString::fromStdString( jType.get<std::string>() );
              if ( type == QLatin1String( "integer" ) )
              {
                field.setType( QMetaType::Type::LongLong );
              }
              else if ( type == QLatin1String( "number" ) )
              {
                field.setType( QMetaType::Type::Double );
              }
              else if ( type == QLatin1String( "boolean" ) )
              {
                field.setType( QMetaType::Type::Bool );
              }
              else if ( type == QLatin1String( "object" ) )
              {
                field.setType( QMetaType::Type::QVariantMap );
              }
              else if ( type == QLatin1String( "array" ) )
              {
                field.setType( QMetaType::Type::QVariantList );
              }
              else if ( type == QLatin1String( "string" ) )
              {
                if ( val.contains( "format" ) )
                {
                  const json jFormat = val["format"];
                  if ( jFormat.is_string() )
                  {
                    const QString format = QString::fromStdString( jFormat.get<std::string>() );
                    if ( format == QLatin1String( "date-time" ) )
                    {
                      field.setType( QMetaType::Type::QDateTime );
                    }
                    else if ( format == QLatin1String( "date" ) )
                    {
                      field.setType( QMetaType::Type::QDate );
                    }
                  }
                }
              }
              else
              {
                QgsDebugMsgLevel( QStringLiteral( "Unhandled OGC API Features schema type: " ) + type, 3 );
              }
            }

            if ( val.contains( "title" ) )
            {
              const json jTitle = val["title"];
              if ( jTitle.is_string() )
              {
                field.setAlias( QString::fromStdString( jTitle.get<std::string>() ) );
              }
            }

            if ( val.contains( "description" ) )
            {
              const json jDescription = val["description"];
              if ( jDescription.is_string() )
              {
                field.setComment( QString::fromStdString( jDescription.get<std::string>() ) );
              }
            }

            if ( seqNumbersAreOk && val.contains( "x-ogc-propertySeq" ) )
            {
              const json jOgcPropertySeq = val["x-ogc-propertySeq"];
              if ( jOgcPropertySeq.is_number_integer() )
              {
                seqNumber = jOgcPropertySeq.get<int>();
              }
            }

            if ( val.contains( "readOnly" ) )
            {
              const json jReadOnly = val["readOnly"];
              if ( jReadOnly.is_boolean() )
              {
                field.setReadOnly( jReadOnly.get<bool>() );
              }
            }

            // Make sure that there are no colliding sequence numbers. If so,
            // give up honouring them.
            if ( seqNumbersAreOk && fields.find( seqNumber ) != fields.end() )
            {
              QgsMessageLog::logMessage( tr( "Schema at %1 has inconsistent x-ogc-propertySeq" ).arg( mUrl.toString() ), tr( "OAPIF" ) );

              seqNumbersAreOk = false;
              seqNumber = largestSeqNumber + 1;
            }

            fields[seqNumber] = std::move( field );

            largestSeqNumber = std::max( largestSeqNumber, seqNumber );
            ++seqNumber;
          }

          if ( val.is_object() && val.contains( "x-ogc-role" ) )
          {
            const json jOgcRole = val["x-ogc-role"];
            if ( jOgcRole.is_string() )
            {
              if ( QString::fromStdString( jOgcRole.get<std::string>() ) == QLatin1String( "primary-geometry" ) )
              {
                mSchema.mGeometryColumnName = QString::fromStdString( key );
                mSchema.mWKBType = Qgis::WkbType::Unknown;
                if ( val.contains( "format" ) )
                {
                  const json jFormat = val["format"];
                  if ( jFormat.is_string() )
                  {
                    const QString format = QString::fromStdString( jFormat.get<std::string>() );
                    const QMap<QString, Qgis::WkbType> mapFormatToWkbType = {
                      { QStringLiteral( "geometry-point" ), Qgis::WkbType::Point },
                      { QStringLiteral( "geometry-multipoint" ), Qgis::WkbType::MultiPoint },
                      { QStringLiteral( "geometry-linestring" ), Qgis::WkbType::LineString },
                      { QStringLiteral( "geometry-multilinestring" ), Qgis::WkbType::MultiLineString },
                      { QStringLiteral( "geometry-polygon" ), Qgis::WkbType::Polygon },
                      { QStringLiteral( "geometry-multipolygon" ), Qgis::WkbType::MultiPolygon },
                      { QStringLiteral( "geometry-geometrycollection" ), Qgis::WkbType::GeometryCollection },
                      { QStringLiteral( "geometry-any" ), Qgis::WkbType::Unknown },
                      { QStringLiteral( "geometry-point-or-multipoint" ), Qgis::WkbType::MultiPoint },
                      { QStringLiteral( "geometry-linestring-or-multilinestring" ), Qgis::WkbType::MultiLineString },
                      { QStringLiteral( "geometry-polygon-or-multipolygon" ), Qgis::WkbType::MultiPolygon },
                    };
                    const auto it = mapFormatToWkbType.constFind( format );
                    if ( it != mapFormatToWkbType.end() )
                    {
                      mSchema.mWKBType = it.value();
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    // Order fields by ascending sequence number
    for ( const auto &[seqNumber, f] : fields )
    {
      mSchema.mFields.append( f );
    }
  }
  catch ( const json::parse_error &ex )
  {
    mErrorCode = QgsBaseNetworkRequest::ApplicationLevelError;
    mErrorMessage = errorMessageWithReason( tr( "Cannot decode JSON document: %1" ).arg( QString::fromStdString( ex.what() ) ) );
    return;
  }
}
