/***************************************************************************
    qgsarrowiterator.cpp
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
#include <nlohmann/json.hpp>

#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgsjsonutils.h"

#define QGIS_NANOARROW_THROW_NOT_OK_ERR( expr, err )                                                                             \
  do                                                                                                                             \
  {                                                                                                                              \
    const int ec = ( expr );                                                                                                     \
    if ( ec != NANOARROW_OK )                                                                                                    \
    {                                                                                                                            \
      throw QgsException( QStringLiteral( "nanoarrow error (%1): %2" ).arg( ec ).arg( QString::fromUtf8( ( err )->message ) ) ); \
    }                                                                                                                            \
  } while ( 0 )

#define QGIS_NANOARROW_THROW_NOT_OK( expr )                                     \
  do                                                                            \
  {                                                                             \
    const int ec = ( expr );                                                    \
    if ( ec != NANOARROW_OK )                                                   \
    {                                                                           \
      throw QgsException( QStringLiteral( "nanoarrow error (%1)" ).arg( ec ) ); \
    }                                                                           \
  } while ( 0 )

QgsArrowSchema::QgsArrowSchema( const QgsArrowSchema &other )
{
  QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaDeepCopy( &other.mSchema, &mSchema ) );
}

QgsArrowSchema &QgsArrowSchema::operator=( const QgsArrowSchema &other )
{
  if ( mSchema.release )
  {
    ArrowSchemaRelease( &mSchema );
  }
  QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaDeepCopy( &other.mSchema, &mSchema ) );
  return *this;
}

QgsArrowSchema::~QgsArrowSchema()
{
  if ( !mSchema.release )
  {
    ArrowSchemaRelease( &mSchema );
  }
}

struct ArrowSchema *QgsArrowSchema::schema()
{
  return &mSchema;
}

const struct ArrowSchema *QgsArrowSchema::schema() const
{
  return &mSchema;
}

uintptr_t QgsArrowSchema::cSchemaAddress()
{
  return reinterpret_cast<uintptr_t>( &mSchema );
}

void QgsArrowSchema::exportToAddress( uintptr_t otherAddress )
{
  struct ArrowSchema *otherArrowSchema = reinterpret_cast<struct ArrowSchema *>( otherAddress );
  ArrowSchemaMove( &mSchema, otherArrowSchema );
}

bool QgsArrowSchema::isValid() const
{
  return mSchema.release;
}

namespace
{


  void inferGeometry( const QgsVectorLayer &layer, struct ArrowSchema *col )
  {
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetName( col, "geometry" ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_BINARY ) );

    // Should be PROJJSON
    QString crsString = layer.crs().toWkt( Qgis::CrsWktVariant::Wkt2_2019 );
    QJsonObject crsMetadata;
    crsMetadata["crs"] = crsString;
    const std::string metadataJson = QgsJsonUtils::jsonFromVariant( crsMetadata ).dump();

    nanoarrow::UniqueBuffer metadataKv;
    QGIS_NANOARROW_THROW_NOT_OK( ArrowMetadataBuilderInit( metadataKv.get(), nullptr ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowMetadataBuilderAppend( metadataKv.get(), ArrowCharView( "ARROW:extension:name" ), ArrowCharView( "geoarrow.wkb" ) ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowMetadataBuilderAppend( metadataKv.get(), ArrowCharView( "ARROW:extension:metadata" ), ArrowCharView( metadataJson.c_str() ) ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetMetadata( col, reinterpret_cast<char *>( metadataKv->data ) ) );
  }

  void appendGeometry( const QgsFeature &feature, struct ArrowArray *col )
  {
    if ( !feature.hasGeometry() )
    {
      QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendNull( col, 1 ) );
      return;
    }

    const QByteArray wkb = feature.geometry().asWkb( QgsAbstractGeometry::FlagExportTrianglesAsPolygons );
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
        throw QgsException( QStringLiteral( "QgsArrowIterator can't infer field type '%1' for field '%2'" ).arg( QMetaType::typeName( field.type() ) ).arg( field.name() ) );
    }
  }

  void appendVariant( const QVariant &v, struct ArrowArray *col, struct ArrowSchemaView &columnTypeView )
  {
    if ( v.isNull() )
    {
      QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendNull( col, 1 ) );
      return;
    }

    switch ( columnTypeView.type )
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
        const QString string = v.toString();
        struct ArrowBufferView bytesView { { string.toUtf8().constData() }, static_cast<int64_t>( string.size() ) };
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendBytes( col, bytesView ) );
        break;
      }

      case NANOARROW_TYPE_BINARY:
      case NANOARROW_TYPE_LARGE_BINARY:
      case NANOARROW_TYPE_BINARY_VIEW:
      case NANOARROW_TYPE_FIXED_SIZE_BINARY:
      {
        const QByteArray bytes = v.toByteArray();
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
        int64_t msSinceEpoch = daysSinceEpoch * 24 * 60 * 60 * 1000;
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, msSinceEpoch ) );
        break;
      }

      case NANOARROW_TYPE_TIMESTAMP:
      {
        const QDateTime dateTime = v.toDateTime();
        switch ( columnTypeView.time_unit )
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
        const QTime time = v.toTime();
        switch ( columnTypeView.time_unit )
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
        const QStringList stringList = v.toStringList();
        for ( const QString &string : stringList )
        {
          struct ArrowBufferView bytesView { { string.toUtf8().constData() }, static_cast<int64_t>( string.size() ) };
          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendBytes( col->children[0], bytesView ) );
        }

        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayFinishElement( col ) );
        break;
      }
      default:
        throw QgsException( QStringLiteral( "Can't convert variant of type '%1' to Arrow type '%2'" ).arg( QMetaType::typeName( v.metaType().id() ) ).arg( ArrowTypeString( columnTypeView.type ) ) );
    }
  }

} //namespace

QgsArrowIterator::QgsArrowIterator( QgsFeatureIterator featureIterator )
  : mFeatureIterator( featureIterator )
{
}

void QgsArrowIterator::setSchema( const QgsArrowSchema &schema )
{
  if ( schema.isValid() )
  {
    throw QgsException( QStringLiteral( "Invalid or null ArrowSchema provided" ) );
  }

  mSchema = schema;
}


void QgsArrowIterator::nextFeatures( int64_t n, struct ArrowArray *out )
{
  if ( !out )
  {
    throw QgsException( "null output ArrowSchema provided" );
  }

  if ( n < 1 )
  {
    throw QgsException( "QgsArrowIterator can't iterate over less than one feature" );
  }

  if ( !mSchema.isValid() )
  {
    throw QgsException( "QgsArrowIterator schema not set" );
  }

  // Check the schema and cache a few things about it before we loop over features.
  // This could also be done when setting the schema (although the struct ArrowSchemaView
  // would have to be opaque in the header if this were cached as a class member).
  const struct ArrowSchema *schema = mSchema.schema();

  struct ArrowError error {};
  std::vector<QString> columnNames( schema->n_children );
  std::vector<struct ArrowSchemaView> colTypeViews( schema->n_children );
  for ( int64_t i = 0; i < schema->n_children; i++ )
  {
    columnNames[i] = QString( schema->children[i]->name != nullptr ? schema->children[i]->name : "" );
    QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowSchemaViewInit( &colTypeViews[i], schema->children[i], &error ), &error );

    // Check that any columns with a list type are lists of strings, as that's the
    // only list type supported here.
    switch ( colTypeViews[i].type )
    {
      case NANOARROW_TYPE_LIST:
      case NANOARROW_TYPE_FIXED_SIZE_LIST:
      case NANOARROW_TYPE_LARGE_LIST:
      case NANOARROW_TYPE_LIST_VIEW:
      case NANOARROW_TYPE_LARGE_LIST_VIEW:
      {
        struct ArrowSchemaView childView;
        QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowSchemaViewInit( &childView, schema->children[i]->children[0], &error ), &error );
        switch ( childView.type )
        {
          case NANOARROW_TYPE_STRING:
          case NANOARROW_TYPE_LARGE_STRING:
          case NANOARROW_TYPE_STRING_VIEW:
            break;
          default:
            throw QgsException( QStringLiteral( "Can't convert to list of Arrow '%1' (only lists of strings are supported)" ).arg( ArrowTypeString( colTypeViews[i].type ) ) );
        }

        break;
      }
      default:
        // No checking needed for other destination types
        break;
    }
  }

  // Create the output array
  nanoarrow::UniqueArray tmp;
  QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowArrayInitFromSchema( tmp.get(), schema, &error ), &error );
  QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayReserve( tmp.get(), n ) );
  QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayStartAppending( tmp.get() ) );

  // Loop features
  std::vector<int> featureAttributeIndex;
  QgsFeature feature;
  while ( n > 0 && mFeatureIterator.nextFeature( feature ) )
  {
    --n;

    // Cache the attribute index per output schema index on the first feature
    if ( featureAttributeIndex.empty() )
    {
      for ( int64_t i = 0; i < schema->n_children; i++ )
      {
        featureAttributeIndex.push_back( feature.fieldNameIndex( columnNames[i] ) );
      }
    }

    // Loop over the output schema fields and append the appropriate attribute from the
    // feature (or geometry, or null if the feature does not contain that field).
    for ( int64_t i = 0; i < schema->n_children; i++ )
    {
      int attributeIndex = featureAttributeIndex[i];
      struct ArrowArray *columnArray = tmp->children[i];

      if ( i == mSchemaGeometryColumnIndex )
      {
        appendGeometry( feature, columnArray );
      }
      else if ( attributeIndex > 0 && attributeIndex < feature.attributeCount() )
      {
        appendVariant( feature.attribute( attributeIndex ), columnArray, colTypeViews[i] );
      }
      else
      {
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendNull( columnArray, 1 ) );
      }
    }
  }

  QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowArrayFinishBuildingDefault( tmp.get(), &error ), &error );
}


QgsArrowSchema QgsArrowIterator::inferSchema( const QgsVectorLayer &layer )
{
  bool layerHasGeometry = layer.geometryType() != Qgis::GeometryType::Unknown && layer.geometryType() != Qgis::GeometryType::Null;

  QgsFields fields = layer.fields();
  QgsArrowSchema out;
  QgisPrivateArrowSchemaInit( out.schema() );
  QGIS_NANOARROW_THROW_NOT_OK( QgisPrivateArrowSchemaSetTypeStruct( out.schema(), fields.count() + layerHasGeometry ) );
  for ( int i = 0; i < fields.count(); i++ )
  {
    inferField( fields.field( i ), out.schema()->children[i] );
  }

  if ( layerHasGeometry )
  {
    inferGeometry( layer, out.schema()->children[fields.count()] );
  }

  return out;
}
