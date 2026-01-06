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

#include <nlohmann/json.hpp>

#include "nanoarrow/nanoarrow.hpp"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"

#define QGIS_NANOARROW_THROW_NOT_OK_ERR( expr, err )                                                                             \
  do                                                                                                                             \
  {                                                                                                                              \
    const int ec = ( expr );                                                                                                     \
    if ( ec != NANOARROW_OK )                                                                                                    \
    {                                                                                                                            \
      throw QgsException( u"nanoarrow error (%1): %2"_s.arg( ec ).arg( QString::fromUtf8( ( err )->message ) ) ); \
    }                                                                                                                            \
  } while ( 0 )

#define QGIS_NANOARROW_THROW_NOT_OK( expr )                                     \
  do                                                                            \
  {                                                                             \
    const int ec = ( expr );                                                    \
    if ( ec != NANOARROW_OK )                                                   \
    {                                                                           \
      throw QgsException( u"nanoarrow error (%1)"_s.arg( ec ) ); \
    }                                                                           \
  } while ( 0 )


QgsArrowInferSchemaOptions::QgsArrowInferSchemaOptions()
{}

void QgsArrowInferSchemaOptions::setGeometryColumnName( const QString &geometryColumnName )
{
  mGeometryColumnName = geometryColumnName;
}

QString QgsArrowInferSchemaOptions::geometryColumnName() const
{
  return mGeometryColumnName;
}

QgsArrowSchema::QgsArrowSchema()
{}

QgsArrowSchema::QgsArrowSchema( const QgsArrowSchema &other )
{
  QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaDeepCopy( &other.mSchema, &mSchema ) );
  mGeometryColumnIndex = other.mGeometryColumnIndex;
}

QgsArrowSchema &QgsArrowSchema::operator=( const QgsArrowSchema &other )
{
  if ( mSchema.release )
  {
    ArrowSchemaRelease( &mSchema );
  }
  QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaDeepCopy( &other.mSchema, &mSchema ) );
  mGeometryColumnIndex = other.mGeometryColumnIndex;
  return *this;
}

QgsArrowSchema::~QgsArrowSchema()
{
  if ( mSchema.release )
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

unsigned long long QgsArrowSchema::cSchemaAddress() const
{
  // In the event QGIS is built on platform where unsigned long long is insufficient to
  // represent a uintptr_t, ensure compilation fails
  static_assert( sizeof( unsigned long long ) >= sizeof( uintptr_t ) );

  return reinterpret_cast<unsigned long long>( &mSchema );
}

void QgsArrowSchema::exportToAddress( unsigned long long otherAddress )
{
  static_assert( sizeof( unsigned long long ) >= sizeof( uintptr_t ) );

  struct ArrowSchema *otherArrowSchema = reinterpret_cast<struct ArrowSchema *>( otherAddress );
  QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaDeepCopy( &mSchema, otherArrowSchema ) );
}

bool QgsArrowSchema::isValid() const
{
  return mSchema.release;
}

int QgsArrowSchema::geometryColumnIndex() const { return mGeometryColumnIndex; }

void QgsArrowSchema::setGeometryColumnIndex( int geometryColumnIndex ) { mGeometryColumnIndex = geometryColumnIndex; }

QgsArrowArray::QgsArrowArray( QgsArrowArray &&other )
{
  if ( mArray.release )
  {
    ArrowArrayRelease( &mArray );
  }

  ArrowArrayMove( other.array(), &mArray );
}

QgsArrowArray &QgsArrowArray::operator=( QgsArrowArray &&other )
{
  if ( this != &other )
  {
    ArrowArrayMove( other.array(), &mArray );
  }

  return *this;
}

QgsArrowArray::~QgsArrowArray()
{
  if ( mArray.release )
  {
    ArrowArrayRelease( &mArray );
  }
}

struct ArrowArray *QgsArrowArray::array()
{
  return &mArray;
}

const struct ArrowArray *QgsArrowArray::array() const
{
  return &mArray;
}

unsigned long long QgsArrowArray::cArrayAddress() const
{
  // In the event QGIS is built on platform where unsigned long long is insufficient to
  // represent a uintptr_t, ensure compilation fails
  static_assert( sizeof( unsigned long long ) >= sizeof( uintptr_t ) );

  return reinterpret_cast<unsigned long long>( &mArray );
}

void QgsArrowArray::exportToAddress( unsigned long long otherAddress )
{
  static_assert( sizeof( unsigned long long ) >= sizeof( uintptr_t ) );

  struct ArrowArray *otherArrowArray = reinterpret_cast<struct ArrowArray *>( otherAddress );
  ArrowArrayMove( &mArray, otherArrowArray );
}

bool QgsArrowArray::isValid() const
{
  return mArray.release;
}

QgsArrowArrayStream::QgsArrowArrayStream( QgsArrowArrayStream &&other )
{
  if ( mArrayStream.release )
  {
    ArrowArrayStreamRelease( &mArrayStream );
  }

  ArrowArrayStreamMove( other.arrayStream(), &mArrayStream );
}

QgsArrowArrayStream &QgsArrowArrayStream::operator=( QgsArrowArrayStream &&other )
{
  if ( this != &other )
  {
    ArrowArrayStreamMove( other.arrayStream(), &mArrayStream );
  }

  return *this;
}

QgsArrowArrayStream::~QgsArrowArrayStream()
{
  if ( mArrayStream.release )
  {
    ArrowArrayStreamRelease( &mArrayStream );
  }
}

struct ArrowArrayStream *QgsArrowArrayStream::arrayStream()
{
  return &mArrayStream;
}

unsigned long long QgsArrowArrayStream::cArrayStreamAddress() const
{
  // In the event QGIS is built on platform where unsigned long long is insufficient to
  // represent a uintptr_t, ensure compilation fails
  static_assert( sizeof( unsigned long long ) >= sizeof( uintptr_t ) );

  return reinterpret_cast<unsigned long long>( &mArrayStream );
}

void QgsArrowArrayStream::exportToAddress( unsigned long long otherAddress )
{
  static_assert( sizeof( unsigned long long ) >= sizeof( uintptr_t ) );

  struct ArrowArrayStream *otherArrowArrayStream = reinterpret_cast<struct ArrowArrayStream *>( otherAddress );
  ArrowArrayStreamMove( &mArrayStream, otherArrowArrayStream );
}

bool QgsArrowArrayStream::isValid() const
{
  return mArrayStream.release;
}

namespace
{


  void inferGeometry( struct ArrowSchema *col, const QString &name, const QgsCoordinateReferenceSystem &crs )
  {
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetName( col, name.toUtf8().constData() ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_BINARY ) );

    std::string crsString = crs.toJsonString();
    std::string geoArrowMetadata;
    if ( crsString.empty() )
    {
      geoArrowMetadata = "{}";
    }
    else
    {
      geoArrowMetadata = R"({"crs":)" + crsString + R"(})";
    }

    nanoarrow::UniqueBuffer metadataKv;
    QGIS_NANOARROW_THROW_NOT_OK( ArrowMetadataBuilderInit( metadataKv.get(), nullptr ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowMetadataBuilderAppend( metadataKv.get(), ArrowCharView( "ARROW:extension:name" ), ArrowCharView( "geoarrow.wkb" ) ) );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowMetadataBuilderAppend( metadataKv.get(), ArrowCharView( "ARROW:extension:metadata" ), ArrowCharView( geoArrowMetadata.c_str() ) ) );
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
    struct ArrowBufferView v;
    v.data.data = wkb.data();
    v.size_bytes = static_cast<int64_t>( wkb.size() );
    QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendBytes( col, v ) );
  }

  void inferMetaType( const QMetaType::Type metaType, struct ArrowSchema *col, const QString &fieldName )
  {
    switch ( metaType )
    {
      case QMetaType::Bool:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_BOOL ) );
        return;
      case QMetaType::QChar:
      case QMetaType::SChar:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_INT8 ) );
        return;
      case QMetaType::UChar:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_UINT8 ) );
        return;
      case QMetaType::Short:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_INT16 ) );
        return;
      case QMetaType::UShort:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_UINT16 ) );
        return;
      case QMetaType::Int:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_INT32 ) );
        return;
      case QMetaType::UInt:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_UINT32 ) );
        return;
      case QMetaType::Long:
      case QMetaType::LongLong:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_INT64 ) );
        return;
      case QMetaType::ULong:
      case QMetaType::ULongLong:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_UINT64 ) );
        return;
      case QMetaType::Float:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_FLOAT ) );
        return;
      case QMetaType::Double:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_DOUBLE ) );
        return;
      case QMetaType::QString:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_STRING ) );
        return;
      case QMetaType::QByteArray:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_BINARY ) );
        return;
      case QMetaType::QDate:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_DATE32 ) );
        return;
      case QMetaType::QTime:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetTypeDateTime( col, NANOARROW_TYPE_TIME32, NANOARROW_TIME_UNIT_MILLI, nullptr ) );
        return;
      case QMetaType::QDateTime:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetTypeDateTime( col, NANOARROW_TYPE_TIMESTAMP, NANOARROW_TIME_UNIT_MILLI, "UTC" ) );
        return;
      case QMetaType::QStringList:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_LIST ) );
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col->children[0], NANOARROW_TYPE_STRING ) );
        return;
      default:
        throw QgsException( u"QgsArrowIterator can't infer field type '%1' for field '%2'"_s.arg( QMetaType::typeName( metaType ) ).arg( fieldName ) );
    }
  }

  void inferField( const QgsField &field, struct ArrowSchema *col )
  {
    QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetName( col, field.name().toUtf8().constData() ) );
    switch ( field.type() )
    {
      case QMetaType::QVariantList:
        QGIS_NANOARROW_THROW_NOT_OK( ArrowSchemaSetType( col, NANOARROW_TYPE_LIST ) );
        inferMetaType( field.subType(), col->children[0], field.name() );
        break;
      default:
        inferMetaType( field.type(), col, field.name() );
        break;
    }
  }

  void appendVariant( const QVariant &v, struct ArrowArray *col, struct ArrowSchemaView &columnTypeView, struct ArrowSchemaView &columnListTypeView )
  {
    if ( QgsVariantUtils::isNull( v ) )
    {
      QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendNull( col, 1 ) );
      return;
    }

    switch ( columnTypeView.type )
    {
      case NANOARROW_TYPE_BOOL:
        if ( v.canConvert( QMetaType::Bool ) )
        {
          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, v.toBool() ) );
          return;
        }
        break;
      case NANOARROW_TYPE_UINT8:
      case NANOARROW_TYPE_UINT16:
      case NANOARROW_TYPE_UINT32:
      case NANOARROW_TYPE_UINT64:
        if ( v.canConvert( QMetaType::ULongLong ) )
        {
          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendUInt( col, v.toULongLong() ) );
          return;
        }
        break;
      case NANOARROW_TYPE_INT8:
      case NANOARROW_TYPE_INT16:
      case NANOARROW_TYPE_INT32:
      case NANOARROW_TYPE_INT64:
        if ( v.canConvert( QMetaType::LongLong ) )
        {
          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, v.toLongLong() ) );
          return;
        }
        break;
      case NANOARROW_TYPE_HALF_FLOAT:
      case NANOARROW_TYPE_FLOAT:
      case NANOARROW_TYPE_DOUBLE:
        if ( v.canConvert( QMetaType::Double ) )
        {
          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendDouble( col, v.toDouble() ) );
          return;
        }
        break;
      case NANOARROW_TYPE_STRING:
      case NANOARROW_TYPE_LARGE_STRING:
      case NANOARROW_TYPE_STRING_VIEW:
      {
        if ( v.canConvert( QMetaType::QString ) )
        {
          const QByteArray string = v.toString().toUtf8();
          struct ArrowBufferView bytesView;
          bytesView.data.data = string.constData();
          bytesView.size_bytes = static_cast<int64_t>( string.size() );
          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendBytes( col, bytesView ) );
          return;
        }
        break;
      }

      case NANOARROW_TYPE_BINARY:
      case NANOARROW_TYPE_LARGE_BINARY:
      case NANOARROW_TYPE_BINARY_VIEW:
      case NANOARROW_TYPE_FIXED_SIZE_BINARY:
      {
        if ( v.canConvert( QMetaType::QByteArray ) )
        {
          const QByteArray bytes = v.toByteArray();
          struct ArrowBufferView bytesView;
          bytesView.data.data = bytes.data();
          bytesView.size_bytes = static_cast<int64_t>( bytes.size() );
          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendBytes( col, bytesView ) );
          return;
        }
        break;
      }

      case NANOARROW_TYPE_DATE32:
      {
        if ( v.canConvert( QMetaType::QDate ) )
        {
          static QDate epoch = QDate( 1970, 1, 1 );
          int64_t daysSinceEpoch = epoch.daysTo( v.toDate() );
          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, daysSinceEpoch ) );
          return;
        }
        break;
      }

      case NANOARROW_TYPE_DATE64:
      {
        if ( v.canConvert( QMetaType::QDate ) )
        {
          static QDate epoch = QDate( 1970, 1, 1 );
          int64_t daysSinceEpoch = epoch.daysTo( v.toDate() );
          int64_t msSinceEpoch = daysSinceEpoch * 24 * 60 * 60 * 1000;
          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, msSinceEpoch ) );
          return;
        }
        break;
      }

      case NANOARROW_TYPE_TIMESTAMP:
      {
        if ( v.canConvert( QMetaType::QDateTime ) )
        {
          const QDateTime dateTime = v.toDateTime().toUTC();
          switch ( columnTypeView.time_unit )
          {
            case NANOARROW_TIME_UNIT_SECOND:
              QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, dateTime.toSecsSinceEpoch() ) );
              return;
            case NANOARROW_TIME_UNIT_MILLI:
              QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, dateTime.toMSecsSinceEpoch() ) );
              return;
            case NANOARROW_TIME_UNIT_MICRO:
              QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, dateTime.toMSecsSinceEpoch() * 1000 ) );
              return;
            case NANOARROW_TIME_UNIT_NANO:
              QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, dateTime.toMSecsSinceEpoch() * 1000 * 1000 ) );
              return;
          }
        }

        break;
      }
      case NANOARROW_TYPE_TIME32:
      case NANOARROW_TYPE_TIME64:
      {
        if ( v.canConvert( QMetaType::QTime ) )
        {
          const QTime time = v.toTime();
          switch ( columnTypeView.time_unit )
          {
            case NANOARROW_TIME_UNIT_SECOND:
              QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, time.msecsSinceStartOfDay() / 1000 ) );
              return;
            case NANOARROW_TIME_UNIT_MILLI:
              QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, time.msecsSinceStartOfDay() ) );
              return;
            case NANOARROW_TIME_UNIT_MICRO:
              QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, static_cast<int64_t>( time.msecsSinceStartOfDay() ) * 1000 ) );
              return;
            case NANOARROW_TIME_UNIT_NANO:
              QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendInt( col, static_cast<int64_t>( time.msecsSinceStartOfDay() ) * 1000 * 1000 ) );
              return;
          }
        }

        break;
      }

      case NANOARROW_TYPE_LIST:
      case NANOARROW_TYPE_FIXED_SIZE_LIST:
      case NANOARROW_TYPE_LARGE_LIST:
      case NANOARROW_TYPE_LIST_VIEW:
      case NANOARROW_TYPE_LARGE_LIST_VIEW:
      {
        if ( v.canConvert( QMetaType::QVariantList ) )
        {
          const QVariantList variantList = v.toList();
          struct ArrowSchemaView dummyListType {};
          for ( const QVariant &item : variantList )
          {
            appendVariant( item, col->children[0], columnListTypeView, dummyListType );
          }

          QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayFinishElement( col ) );
          return;
        }
        break;
      }
      default:
        break;
    }

    throw QgsException( u"Can't convert variant of type '%1' to Arrow type '%2'"_s.arg( v.typeName() ).arg( ArrowTypeString( columnTypeView.type ) ) );
  }

  class ArrowIteratorArrayStreamImpl
  {
    public:
      ArrowIteratorArrayStreamImpl( QgsArrowIterator iterator, int batchSize )
        : mIterator( iterator ), mBatchSize( batchSize ) {}

      int GetSchema( struct ArrowSchema *schema )
      {
        NANOARROW_RETURN_NOT_OK( ArrowSchemaDeepCopy( mIterator.schema(), schema ) );
        return NANOARROW_OK;
      }

      int GetNext( struct ArrowArray *array )
      {
        try
        {
          QgsArrowArray batch = mIterator.nextFeatures( mBatchSize );
          ArrowArrayMove( batch.array(), array );
          return NANOARROW_OK;
        }
        catch ( QgsException &e )
        {
          mLastError = e.what().toStdString();
          return EINVAL;
        }
        catch ( std::exception &e )
        {
          mLastError = e.what();
          return EINVAL;
        }
        catch ( ... )
        {
          mLastError = "unknown error";
          return EINVAL;
        }
      }

      const char *GetLastError() const { return mLastError.c_str(); }

    private:
      QgsArrowIterator mIterator;
      int mBatchSize { 65536 };
      std::string mLastError {};
  };

} //namespace

QgsArrowIterator::QgsArrowIterator( QgsFeatureIterator featureIterator )
  : mFeatureIterator( featureIterator )
{
}

struct ArrowSchema *QgsArrowIterator::schema()
{
  return mSchema.schema();
}

void QgsArrowIterator::setSchema( const QgsArrowSchema &schema )
{
  if ( !schema.isValid() )
  {
    throw QgsException( u"Invalid or null ArrowSchema provided"_s );
  }

  mSchema = schema;
}

QgsArrowArrayStream QgsArrowIterator::toArrayStream( int batchSize ) const
{
  QgsArrowArrayStream out;
  nanoarrow::ArrayStreamFactory<ArrowIteratorArrayStreamImpl>::InitArrayStream( new ArrowIteratorArrayStreamImpl( *this, batchSize ), out.arrayStream() );
  return out;
}


QgsArrowArray QgsArrowIterator::nextFeatures( int n )
{
  if ( n < 1 )
  {
    throw QgsException( u"QgsArrowIterator can't iterate over less than one feature"_s );
  }

  if ( !mSchema.isValid() )
  {
    throw QgsException( u"QgsArrowIterator schema not set"_s );
  }

  // Check the schema and cache a few things about it before we loop over features.
  // This could also be done when setting the schema (although the struct ArrowSchemaView
  // would have to be opaque in the header if this were cached as a class member).
  const struct ArrowSchema *schema = mSchema.schema();

  struct ArrowError error {};

  // Check that the top-level schema is a struct
  struct ArrowSchemaView schemaView;
  QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowSchemaViewInit( &schemaView, schema, &error ), &error );
  if ( schemaView.type != NANOARROW_TYPE_STRUCT )
  {
    throw QgsException( u"QgsArrowIterator expected requested schema as struct but got '%1'"_s.arg( ArrowTypeString( schemaView.type ) ) );
  }

  std::vector<QString> columnNames( schema->n_children );
  std::vector<struct ArrowSchemaView> colTypeViews( schema->n_children );
  std::vector<struct ArrowSchemaView> colListTypeViews( schema->n_children );
  for ( int64_t i = 0; i < schema->n_children; i++ )
  {
    // Parse the column schema
    columnNames[i] = QString( schema->children[i]->name != nullptr ? schema->children[i]->name : QString() );
    QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowSchemaViewInit( &colTypeViews[i], schema->children[i], &error ), &error );

    // Parse the column list type if applicable
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
        colListTypeViews[i] = std::move( childView );
        break;
      }
      default:
        colListTypeViews[i] = ArrowSchemaView {};
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

      if ( i == mSchema.geometryColumnIndex() )
      {
        appendGeometry( feature, columnArray );
      }
      else if ( attributeIndex >= 0 && attributeIndex < feature.attributeCount() )
      {
        appendVariant( feature.attribute( attributeIndex ), columnArray, colTypeViews[i], colListTypeViews[i] );
      }
      else
      {
        QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayAppendNull( columnArray, 1 ) );
      }
    }

    QGIS_NANOARROW_THROW_NOT_OK( ArrowArrayFinishElement( tmp.get() ) );
  }

  QGIS_NANOARROW_THROW_NOT_OK_ERR( ArrowArrayFinishBuildingDefault( tmp.get(), &error ), &error );

  QgsArrowArray out;
  if ( tmp->length > 0 )
  {
    ArrowArrayMove( tmp.get(), out.array() );
  }
  return out;
}

QgsArrowSchema QgsArrowIterator::inferSchema( const QgsVectorLayer &layer, const QgsArrowInferSchemaOptions &options )
{
  bool layerHasGeometry = layer.isSpatial();
  if ( layerHasGeometry && options.geometryColumnName().isEmpty() )
  {
    QgsArrowInferSchemaOptions optionsClone( options );
    optionsClone.setGeometryColumnName( layer.dataProvider()->geometryColumnName() );
    return inferSchema( layer.fields(), layerHasGeometry, layer.crs(), optionsClone );
  }
  else
  {
    return inferSchema( layer.fields(), layerHasGeometry, layer.crs(), options );
  }
}


QgsArrowSchema QgsArrowIterator::inferSchema( const QgsFields &fields, bool hasGeometry, const QgsCoordinateReferenceSystem &crs, const QgsArrowInferSchemaOptions &options )
{
  QgsArrowSchema out;
  QgisPrivateArrowSchemaInit( out.schema() );
  QGIS_NANOARROW_THROW_NOT_OK( QgisPrivateArrowSchemaSetTypeStruct( out.schema(), fields.count() + hasGeometry ) );
  for ( int i = 0; i < fields.count(); i++ )
  {
    inferField( fields.field( i ), out.schema()->children[i] );
  }

  if ( hasGeometry )
  {
    QString geometryColumnName = options.geometryColumnName();
    if ( geometryColumnName.isEmpty() )
    {
      geometryColumnName = u"geometry"_s;
    }

    inferGeometry( out.schema()->children[fields.count()], geometryColumnName, crs );
    out.setGeometryColumnIndex( fields.count() );
  }

  return out;
}
