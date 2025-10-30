/***************************************************************************
    qgsarrowreader.h
    ---------------------
    begin                : November 2025
    copyright            : (C) 2025 by Dewey Dunnington
    email                : dewey at dunnington dot ca
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarrowiterator.h"

#include <nanoarrow/nanoarrow.hpp>

#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"

#define QGIS_NANOARROW_THROW_NOT_OK_ERR( expr, err )                                                                      \
  do                                                                                                                      \
  {                                                                                                                       \
    const int ec = ( expr );                                                                                              \
    if ( ec != NANOARROW_OK )                                                                                             \
    {                                                                                                                     \
      throw QgsException( QString( "nanoarrow error (%1): %2" ).arg( ec ).arg( QString::fromUtf8( ( err )->message ) ) ); \
    }                                                                                                                     \
  } while ( 0 )

#define QGIS_NANOARROW_THROW_NOT_OK( expr )                              \
  do                                                                     \
  {                                                                      \
    const int ec = ( expr );                                             \
    if ( ec != NANOARROW_OK )                                            \
    {                                                                    \
      throw QgsException( QString( "nanoarrow error (%1)" ).arg( ec ) ); \
    }                                                                    \
  } while ( 0 )

namespace
{


  void inferGeometry( const QgsVectorLayer &layer, struct ArrowSchema *col )
  {
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetName( col, "geometry" ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_BINARY ) );

    QString crsString = layer.crs().toWkt( Qgis::CrsWktVariant::Wkt2_2019 );
    QJsonObject crsMetadata;
    crsMetadata["crs"] = crsString;
    QString metadataJson = QJsonDocument( crsMetadata ).toJson( QJsonDocument::Compact );

    nanoarrow::UniqueBuffer metadataKv;
    QGIS_NANOARROW_THROW_NOT_OK( ArrowMetadataBuilderInit( metadataKv.get(), nullptr ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowMetadataBuilderAppend( metadataKv.get(), ArrowCharView( "ARROW:extension:name" ), ArrowCharView( "geoarrow.wkb" ) ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowMetadataBuilderAppend( metadataKv.get(), ArrowCharView( "ARROW:extension:metadata" ), ArrowCharView( metadataJson.toUtf8().constData() ) ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetMetadata( col, reinterpret_cast<char *>( metadataKv->data ) ) );
  }

  void appendGeometry( const QgsFeature feat, struct ArrowArray *col )
  {
    if ( !feat.hasGeometry() )
    {
      QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendNull( col, 1 ) );
      return;
    }

    QByteArray wkb = feat.geometry().asWkb( QgsAbstractGeometry::FlagExportTrianglesAsPolygons );
    struct ArrowBufferView v { { wkb.data() }, static_cast<int64_t>( wkb.size() ) };
    QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendBytes( col, v ) );
  }

  void inferField( const QgsField &field, struct ArrowSchema *col )
  {
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetName( col, field.name().toUtf8().constData() ) );

    switch ( field.type() )
    {
      case QMetaType::Bool:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_BOOL ) );
        break;
      case QMetaType::QChar:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_INT8 ) );
        break;
      case QMetaType::Short:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_INT16 ) );
        break;
      case QMetaType::UShort:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_UINT16 ) );
        break;
      case QMetaType::Int:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_INT32 ) );
        break;
      case QMetaType::UInt:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_UINT32 ) );
        break;
      case QMetaType::Long:
      case QMetaType::LongLong:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_INT64 ) );
        break;
      case QMetaType::ULong:
      case QMetaType::ULongLong:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_UINT64 ) );
        break;
      case QMetaType::Float:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_FLOAT ) );
        break;
      case QMetaType::Double:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_DOUBLE ) );
        break;
      case QMetaType::QString:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_STRING ) );
        break;
      case QMetaType::QByteArray:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_BINARY ) );
        break;
      case QMetaType::QDate:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_DATE32 ) );
        break;
      case QMetaType::QTime:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetTypeDateTime( col, NANOARROW_TYPE_TIME32, NANOARROW_TIME_UNIT_MILLI, nullptr ) );
        break;
      case QMetaType::QDateTime:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetTypeDateTime( col, NANOARROW_TYPE_TIMESTAMP, NANOARROW_TIME_UNIT_MILLI, nullptr ) );
        break;
      case QMetaType::QStringList:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_LIST ) );
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col->children[0], NANOARROW_TYPE_STRING ) );
        break;

      default:
        throw QgsException( QString( "QgsArrowIterator can't infer field type '%1' for field '%2'" ).arg( QMetaType::typeName( field.type() ) ).arg( field.name() ) );
    }
  }

  void appendVariant( const QVariant &v, struct ArrowArray *col, const struct ArrowSchema *colType, struct ArrowError *error )
  {
    struct ArrowSchemaView colTypeView;
    QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowSchemaViewInit( &colTypeView, colType, error ), error );

    switch ( colTypeView.type )
    {
      case NANOARROW_TYPE_BOOL:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, v.toBool() ) );
        return;
      case NANOARROW_TYPE_UINT8:
      case NANOARROW_TYPE_UINT16:
      case NANOARROW_TYPE_UINT32:
      case NANOARROW_TYPE_UINT64:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendUInt( col, v.toULongLong() ) );
        break;
      case NANOARROW_TYPE_INT8:
      case NANOARROW_TYPE_INT16:
      case NANOARROW_TYPE_INT32:
      case NANOARROW_TYPE_INT64:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, v.toLongLong() ) );
        break;
      case NANOARROW_TYPE_HALF_FLOAT:
      case NANOARROW_TYPE_FLOAT:
      case NANOARROW_TYPE_DOUBLE:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendDouble( col, v.toDouble() ) );
        break;
      case NANOARROW_TYPE_STRING:
      case NANOARROW_TYPE_LARGE_STRING:
      case NANOARROW_TYPE_STRING_VIEW:
      {
        QString string = v.toString().toUtf8();
        struct ArrowBufferView bytesView { { string.constData() }, static_cast<int64_t>( string.size() ) };
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendBytes( col, bytesView ) );
        break;
      }

      case NANOARROW_TYPE_BINARY:
      case NANOARROW_TYPE_LARGE_BINARY:
      case NANOARROW_TYPE_BINARY_VIEW:
      case NANOARROW_TYPE_FIXED_SIZE_BINARY:
      {
        QByteArray bytes = v.toByteArray();
        struct ArrowBufferView bytesView { { bytes.data() }, static_cast<int64_t>( bytes.size() ) };
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendBytes( col, bytesView ) );
        break;
      }

      case NANOARROW_TYPE_DATE32:
      {
        static QDate epoch = QDate( 1970, 1, 1 );
        int64_t daysSinceEpoch = epoch.daysTo( v.toDate() );
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, daysSinceEpoch ) );
        break;
      }

      case NANOARROW_TYPE_DATE64:
      {
        static QDate epoch = QDate( 1970, 1, 1 );
        int64_t daysSinceEpoch = epoch.daysTo( v.toDate() );
        int64_t msSinceEpoch = daysSinceEpoch * 24 * 60 * 80 * 1000;
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, msSinceEpoch ) );
        break;
      }

      case NANOARROW_TYPE_TIMESTAMP:
      {
        QDateTime dateTime = v.toDateTime();
        switch ( colTypeView.time_unit )
        {
          case NANOARROW_TIME_UNIT_SECOND:
            QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, dateTime.toSecsSinceEpoch() ) );
            break;
          case NANOARROW_TIME_UNIT_MILLI:
            QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, dateTime.toMSecsSinceEpoch() ) );
            break;
          case NANOARROW_TIME_UNIT_MICRO:
            QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, dateTime.toSecsSinceEpoch() * 1000 ) );
            break;
          case NANOARROW_TIME_UNIT_NANO:
            QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, dateTime.toSecsSinceEpoch() * 1000 * 1000 ) );
            break;
        }

        break;
      }
      case NANOARROW_TYPE_TIME32:
      case NANOARROW_TYPE_TIME64:
      {
        QTime time = v.toTime();
        switch ( colTypeView.time_unit )
        {
          case NANOARROW_TIME_UNIT_SECOND:
            QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, time.msec() / 1000 ) );
            break;
          case NANOARROW_TIME_UNIT_MILLI:
            QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, time.msec() ) );
            break;
          case NANOARROW_TIME_UNIT_MICRO:
            QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, static_cast<int64_t>( time.msec() ) * 1000 ) );
            break;
          case NANOARROW_TIME_UNIT_NANO:
            QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, static_cast<int64_t>( time.msec() ) * 1000 * 1000 ) );
            break;
        }

        break;
      }

      case NANOARROW_TYPE_LIST:
      case NANOARROW_TYPE_FIXED_SIZE_LIST:
      case NANOARROW_TYPE_LARGE_LIST:
      case NANOARROW_TYPE_LIST_VIEW:
      case NANOARROW_TYPE_LARGE_LIST_VIEW:
      {
        struct ArrowSchemaView childView;
        QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowSchemaViewInit( &childView, colType->children[0], error ), error );
        switch ( childView.type )
        {
          case NANOARROW_TYPE_STRING:
          case NANOARROW_TYPE_LARGE_STRING:
          case NANOARROW_TYPE_STRING_VIEW:
          {
            QStringList stringList = v.toStringList();
            for ( const auto &item : stringList )
            {
              QString string = item.toUtf8();
              struct ArrowBufferView bytesView { { string.constData() }, static_cast<int64_t>( string.size() ) };
              QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendBytes( col->children[0], bytesView ) );
            }

            QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayFinishElement( col ) );
            break;
          }
          default:
            throw QgsException( QString( "Can't convert variant of type '%1' to list of Arrow '%2'" ).arg( QMetaType::typeName( v.metaType().id() ) ).arg( ArrowTypeString( colTypeView.type ) ) );
        }

        break;
      }
      default:
        throw QgsException( QString( "Can't convert variant of type '%1' to Arrow type '%2'" ).arg( QMetaType::typeName( v.metaType().id() ) ).arg( ArrowTypeString( colTypeView.type ) ) );
    }
  }

} //namespace

QgsArrowIterator::QgsArrowIterator( QgsFeatureIterator featureIterator )
  : mFeatureIterator( featureIterator )
{
}

QgsArrowIterator::~QgsArrowIterator()
{
  if ( mSchema.release != nullptr )
  {
    ArrowSchemaRelease( &mSchema );
  }
}

void QgsArrowIterator::setSchema( const struct ArrowSchema *requestedSchema )
{
  if ( requestedSchema == nullptr || requestedSchema->release == nullptr )
  {
    throw QgsException( "Invalid or null ArrowSchema provided" );
  }

  if ( mSchema.release != nullptr )
  {
    ArrowSchemaRelease( &mSchema );
  }

  ArrowSchemaDeepCopy( requestedSchema, &mSchema );
}


void QgsArrowIterator::nextFeatures( int64_t n, struct ArrowArray *out )
{
  if ( out == nullptr )
  {
    throw QgsException( "null output ArrowSchema provided" );
  }

  if ( n < 1 )
  {
    throw QgsException( "QgsArrowIterator can't iterate over less than one feature" );
  }

  if ( mSchema.release == nullptr )
  {
    throw QgsException( "QgsArrowIterator schema not set" );
  }

  nanoarrow::UniqueArray tmp;
  struct ArrowError error {};
  QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowArrayInitFromSchema( tmp.get(), &mSchema, &error ), &error );
  QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayReserve( tmp.get(), n ) );

  std::vector<QString> colNames( mSchema.n_children );
  for ( int64_t i = 0; i < mSchema.n_children; i++ )
  {
    colNames[i] = QString( mSchema.name != nullptr ? mSchema.name : "" );
  }

  QgsFeature feat;
  while ( n > 0 && mFeatureIterator.nextFeature( feat ) )
  {
    --n;
    QVariantMap attrs = feat.attributeMap();

    for ( int64_t i = 0; i < mSchema.n_children; i++ )
    {
      struct ArrowArray *col = tmp->children[i];

      if ( i == mSchemaGeometryColumnIndex )
      {
        appendGeometry( feat, col );
      }
      else if ( attrs.contains( colNames[i] ) )
      {
        appendVariant( attrs.value( colNames[i] ), col, mSchema.children[i], &error );
      }
      else
      {
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendNull( col, 1 ) );
      }
    }
  }
}


int64_t QgsArrowIterator::inferSchema( const QgsVectorLayer &layer, struct ArrowSchema *out )
{
  if ( out == nullptr )
  {
    throw QgsException( "null output ArrowSchema provided" );
  }

  bool layerHasGeometry = layer.geometryType() != Qgis::GeometryType::Unknown && layer.geometryType() != Qgis::GeometryType::Null;

  QgsFields fields = layer.fields();
  nanoarrow::UniqueSchema tmp;
  QgisPrivateArrowSchemaInit( tmp.get() );
  QGIS_NANOARROW_THROW_NOT_OK( QgisPrivateArrowSchemaSetTypeStruct( tmp.get(), fields.count() + layerHasGeometry ) );
  for ( int i = 0; i < fields.count(); i++ )
  {
    inferField( fields.field( i ), tmp->children[i] );
  }

  if ( layerHasGeometry )
  {
    inferGeometry( layer, tmp->children[fields.count()] );
  }

  ArrowSchemaMove( tmp.get(), out );
  if ( layerHasGeometry )
  {
    return fields.count();
  }
  else
  {
    return -1;
  }
}
