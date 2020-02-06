/***************************************************************************
  qgsdelimitedtextprovider.cpp -  Data provider for delimited text
  -------------------
          begin                : 2004-02-27
          copyright            : (C) 2004 by Gary E.Sherman
          email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdelimitedtextprovider.h"

#include <QtGlobal>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QTextStream>
#include <QStringList>
#include <QSettings>
#include <QRegExp>
#include <QUrl>
#include <QUrlQuery>

#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmessageoutput.h"
#include "qgsrectangle.h"
#include "qgsspatialindex.h"
#include "qgis.h"
#include "qgsexpressioncontextutils.h"
#include "qgsproviderregistry.h"

#include "qgsdelimitedtextfeatureiterator.h"
#include "qgsdelimitedtextfile.h"


const QString QgsDelimitedTextProvider::TEXT_PROVIDER_KEY = QStringLiteral( "delimitedtext" );
const QString QgsDelimitedTextProvider::TEXT_PROVIDER_DESCRIPTION = QStringLiteral( "Delimited text data provider" );

// If more than this fraction of records are not in a subset then use an index to
// iterate over records rather than simple iterator and filter.

static const int SUBSET_ID_THRESHOLD_FACTOR = 10;

QRegExp QgsDelimitedTextProvider::sWktPrefixRegexp( "^\\s*(?:\\d+\\s+|SRID\\=\\d+\\;)", Qt::CaseInsensitive );
QRegExp QgsDelimitedTextProvider::sCrdDmsRegexp( "^\\s*(?:([-+nsew])\\s*)?(\\d{1,3})(?:[^0-9.]+([0-5]?\\d))?[^0-9.]+([0-5]?\\d(?:\\.\\d+)?)[^0-9.]*([-+nsew])?\\s*$", Qt::CaseInsensitive );

QgsDelimitedTextProvider::QgsDelimitedTextProvider( const QString &uri, const ProviderOptions &options )
  : QgsVectorDataProvider( uri, options )
{

  // Add supported types to enable creating expression fields in field calculator
  setNativeTypes( QList< NativeType >()
                  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer)" ), QStringLiteral( "integer" ), QVariant::Int, 0, 10 )
                  << QgsVectorDataProvider::NativeType( tr( "Whole number (integer - 64 bit)" ), QStringLiteral( "int8" ), QVariant::LongLong )
                  << QgsVectorDataProvider::NativeType( tr( "Decimal number (double)" ), QStringLiteral( "double precision" ), QVariant::Double, -1, -1, -1, -1 )
                  << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), QStringLiteral( "text" ), QVariant::String, -1, -1, -1, -1 )
                );

  QgsDebugMsg( "Delimited text file uri is " + uri );

  const QUrl url = QUrl::fromEncoded( uri.toLatin1() );
  mFile = qgis::make_unique< QgsDelimitedTextFile >();
  mFile->setFromUrl( url );

  QString subset;

  const QUrlQuery query( url );
  if ( query.hasQueryItem( QStringLiteral( "geomType" ) ) )
  {
    QString gtype = query.queryItemValue( QStringLiteral( "geomType" ) ).toLower();
    if ( gtype == QLatin1String( "point" ) ) mGeometryType = QgsWkbTypes::PointGeometry;
    else if ( gtype == QLatin1String( "line" ) ) mGeometryType = QgsWkbTypes::LineGeometry;
    else if ( gtype == QLatin1String( "polygon" ) ) mGeometryType = QgsWkbTypes::PolygonGeometry;
    else if ( gtype == QLatin1String( "none " ) ) mGeometryType = QgsWkbTypes::NullGeometry;
  }

  if ( mGeometryType != QgsWkbTypes::NullGeometry )
  {
    if ( query.hasQueryItem( QStringLiteral( "wktField" ) ) )
    {
      mWktFieldName = query.queryItemValue( QStringLiteral( "wktField" ) );
      mGeomRep = GeomAsWkt;
      QgsDebugMsg( "wktField is: " + mWktFieldName );
    }
    else if ( query.hasQueryItem( QStringLiteral( "xField" ) ) && query.hasQueryItem( QStringLiteral( "yField" ) ) )
    {
      mGeomRep = GeomAsXy;
      mGeometryType = QgsWkbTypes::PointGeometry;
      mXFieldName = query.queryItemValue( QStringLiteral( "xField" ) );
      mYFieldName = query.queryItemValue( QStringLiteral( "yField" ) );
      if ( query.hasQueryItem( QStringLiteral( "zField" ) ) )
        mZFieldName = query.queryItemValue( QStringLiteral( "zField" ) );
      if ( query.hasQueryItem( QStringLiteral( "mField" ) ) )
        mMFieldName = query.queryItemValue( QStringLiteral( "mField" ) );
      QgsDebugMsg( "xField is: " + mXFieldName );
      QgsDebugMsg( "yField is: " + mYFieldName );
      QgsDebugMsg( "zField is: " + mZFieldName );
      QgsDebugMsg( "mField is: " + mMFieldName );

      if ( query.hasQueryItem( QStringLiteral( "xyDms" ) ) )
      {
        mXyDms = ! query.queryItemValue( QStringLiteral( "xyDms" ) ).toLower().startsWith( 'n' );
      }
    }
    else
    {
      mGeometryType = QgsWkbTypes::NullGeometry;
    }
  }

  mDetectTypes = true;
  if ( query.hasQueryItem( QStringLiteral( "detectTypes" ) ) )
    mDetectTypes = ! query.queryItemValue( QStringLiteral( "detectTypes" ) ).toLower().startsWith( 'n' );

  if ( query.hasQueryItem( QStringLiteral( "decimalPoint" ) ) )
    mDecimalPoint = query.queryItemValue( QStringLiteral( "decimalPoint" ) );

  if ( query.hasQueryItem( QStringLiteral( "crs" ) ) )
    mCrs.createFromString( query.queryItemValue( QStringLiteral( "crs" ) ) );

  if ( query.hasQueryItem( QStringLiteral( "subsetIndex" ) ) )
  {
    mBuildSubsetIndex = ! query.queryItemValue( QStringLiteral( "subsetIndex" ) ).toLower().startsWith( 'n' );
  }

  if ( query.hasQueryItem( QStringLiteral( "spatialIndex" ) ) )
  {
    mBuildSpatialIndex = ! query.queryItemValue( QStringLiteral( "spatialIndex" ) ).toLower().startsWith( 'n' );
  }

  if ( query.hasQueryItem( QStringLiteral( "subset" ) ) )
  {
    // We need to specify FullyDecoded so that %25 is decoded as %
    subset = query.queryItemValue( QStringLiteral( "subset" ), QUrl::FullyDecoded );
    QgsDebugMsg( "subset is: " + subset );
  }

  if ( query.hasQueryItem( QStringLiteral( "quiet" ) ) ) mShowInvalidLines = false;

  // Do an initial scan of the file to determine field names, types,
  // geometry type (for Wkt), extents, etc.  Parameter value subset.isEmpty()
  // avoid redundant building indexes if we will be building a subset string,
  // in which case indexes will be rebuilt.

  scanFile( subset.isEmpty() );

  if ( ! subset.isEmpty() )
  {
    setSubsetString( subset );
  }
}

QgsDelimitedTextProvider::~QgsDelimitedTextProvider() = default;

QgsAbstractFeatureSource *QgsDelimitedTextProvider::featureSource() const
{
  // If the file has become invalid, rescan to check that it is still invalid.
  //
  if ( ( mLayerValid && ! mValid ) || mRescanRequired )
    const_cast<QgsDelimitedTextProvider *>( this )->rescanFile();

  return new QgsDelimitedTextFeatureSource( this );
}

QStringList QgsDelimitedTextProvider::readCsvtFieldTypes( const QString &filename, QString *message )
{
  // Look for a file with the same name as the data file, but an extra 't' or 'T' at the end
  QStringList types;
  QFileInfo csvtInfo( filename + 't' );
  if ( ! csvtInfo.exists() ) csvtInfo.setFile( filename + 'T' );
  if ( ! csvtInfo.exists() ) return types;
  QFile csvtFile( csvtInfo.filePath() );
  if ( ! csvtFile.open( QIODevice::ReadOnly ) ) return types;


  // If anything goes wrong here, just ignore it, as the file
  // is not valid, so just ignore any exceptions.

  // For it to be valid, there must be just one non blank line at the beginning of the
  // file.

  QString strTypeList;
  try
  {
    QTextStream csvtStream( &csvtFile );
    strTypeList = csvtStream.readLine();
    if ( strTypeList.isEmpty() ) return types;
    QString extra = csvtStream.readLine();
    while ( ! extra.isNull() )
    {
      if ( ! extra.isEmpty() ) return types;
      extra = csvtStream.readLine();
    }
  }
  catch ( ... )
  {
    return types;
  }
  csvtFile.close();

  // Is the type string valid?
  // This is a slightly generous regular expression in that it allows spaces and unquoted field types
  // not allowed in OGR CSVT files.  Also doesn't care if int and string fields have

  strTypeList = strTypeList.toLower();
  QRegExp reTypeList( "^(?:\\s*(\\\"?)(?:integer|real|double|long|longlong|int8|string|date|datetime|time)(?:\\(\\d+(?:\\.\\d+)?\\))?\\1\\s*(?:,|$))+" );
  if ( ! reTypeList.exactMatch( strTypeList ) )
  {
    // Looks like this was supposed to be a CSVT file, so report bad formatted string
    if ( message ) { *message = tr( "File type string in %1 is not correctly formatted" ).arg( csvtInfo.fileName() ); }
    return types;
  }

  // All good, so pull out the types from the string.  Currently only returning integer, real, and string types

  QgsDebugMsg( QStringLiteral( "Reading field types from %1" ).arg( csvtInfo.fileName() ) );
  QgsDebugMsg( QStringLiteral( "Field type string: %1" ).arg( strTypeList ) );

  int pos = 0;
  QRegExp reType( "(integer|real|double|string|date|datetime|time)" );

  while ( ( pos = reType.indexIn( strTypeList, pos ) ) != -1 )
  {
    QgsDebugMsg( QStringLiteral( "Found type: %1" ).arg( reType.cap( 1 ) ) );
    types << reType.cap( 1 );
    pos += reType.matchedLength();
  }

  if ( message )
  {
    // Would be a useful info message, but don't want dialog to pop up every time...
    // *message=tr("Reading field types from %1").arg(csvtInfo.fileName());
  }

  return types;
}

void QgsDelimitedTextProvider::resetCachedSubset() const
{
  mCachedSubsetString = QString();
  mCachedUseSubsetIndex = false;
  mCachedUseSpatialIndex = false;
}

void QgsDelimitedTextProvider::resetIndexes() const
{
  resetCachedSubset();
  mUseSubsetIndex = false;
  mUseSpatialIndex = false;

  mSubsetIndex.clear();
  if ( mBuildSpatialIndex && mGeomRep != GeomNone )
    mSpatialIndex = qgis::make_unique< QgsSpatialIndex >();
}

bool QgsDelimitedTextProvider::createSpatialIndex()
{
  if ( mBuildSpatialIndex )
    return true; // Already built
  if ( mGeomRep == GeomNone )
    return false; // Cannot build index - no geometries

  // OK, set the spatial index option, set the Uri parameter so that the index is
  // rebuilt when theproject is reloaded, and rescan the file to populate the index

  mBuildSpatialIndex = true;
  setUriParameter( QStringLiteral( "spatialIndex" ), QStringLiteral( "yes" ) );
  rescanFile();
  return true;
}

QgsFeatureSource::SpatialIndexPresence QgsDelimitedTextProvider::hasSpatialIndex() const
{
  return mSpatialIndex ? QgsFeatureSource::SpatialIndexPresent : QgsFeatureSource::SpatialIndexNotPresent;
}

// Really want to merge scanFile and rescan into single code.  Currently the reason
// this is not done is that scanFile is done initially to create field names and, rescan
// file includes building subset expression and assumes field names/types are already
// defined.  Merging would not only make code a lot cleaner, but would also avoid
// double scan when loading a file with a subset expression.

// buildIndexes parameter of scanFile is set to false when we know we will be
// immediately rescanning (when the file is loaded and then the subset expression is
// set)

void QgsDelimitedTextProvider::scanFile( bool buildIndexes )
{
  QStringList messages;

  // assume the layer is invalid until proven otherwise

  mLayerValid = false;
  mValid = false;
  mRescanRequired = false;

  clearInvalidLines();

  // Initiallize indexes

  resetIndexes();
  bool buildSpatialIndex = buildIndexes && nullptr != mSpatialIndex;

  // No point building a subset index if there is no geometry, as all
  // records will be included.

  bool buildSubsetIndex = buildIndexes && mBuildSubsetIndex && mGeomRep != GeomNone;

  if ( ! mFile->isValid() )
  {
    // uri is invalid so the layer must be too...

    messages.append( tr( "File cannot be opened or delimiter parameters are not valid" ) );
    reportErrors( messages );
    QgsDebugMsg( QStringLiteral( "Delimited text source invalid - filename or delimiter parameters" ) );
    return;
  }

  // Open the file and get number of rows, etc. We assume that the
  // file has a header row and process accordingly. Caller should make
  // sure that the delimited file is properly formed.

  if ( mGeomRep == GeomAsWkt )
  {
    mWktFieldIndex = mFile->fieldIndex( mWktFieldName );
    if ( mWktFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( QStringLiteral( "Wkt" ), mWktFieldName ) );
    }
  }
  else if ( mGeomRep == GeomAsXy )
  {
    mXFieldIndex = mFile->fieldIndex( mXFieldName );
    mYFieldIndex = mFile->fieldIndex( mYFieldName );
    if ( mXFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( QStringLiteral( "X" ), mXFieldName ) );
    }
    if ( mYFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( QStringLiteral( "Y" ), mYFieldName ) );
    }
    if ( !mZFieldName.isEmpty() )
    {
      mZFieldIndex = mFile->fieldIndex( mZFieldName );
      if ( mZFieldIndex < 0 )
      {
        messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( QStringLiteral( "Z" ), mZFieldName ) );
      }
    }
    if ( !mMFieldName.isEmpty() )
    {
      mMFieldIndex = mFile->fieldIndex( mMFieldName );
      if ( mMFieldIndex < 0 )
      {
        messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( QStringLiteral( "M" ), mMFieldName ) );
      }
    }
  }
  if ( !messages.isEmpty() )
  {
    reportErrors( messages );
    QgsDebugMsg( QStringLiteral( "Delimited text source invalid - missing geometry fields" ) );
    return;
  }

  // Scan the entire file to determine
  // 1) the number of fields (this is handled by QgsDelimitedTextFile mFile
  // 2) the number of valid features.  Note that the selection of valid features
  //    should match the code in QgsDelimitedTextFeatureIterator
  // 3) the geometric extents of the layer
  // 4) the type of each field
  //
  // Also build subset and spatial indexes.

  QStringList parts;
  long nEmptyRecords = 0;
  long nBadFormatRecords = 0;
  long nIncompatibleGeometry = 0;
  long nInvalidGeometry = 0;
  long nEmptyGeometry = 0;
  mNumberFeatures = 0;
  mExtent = QgsRectangle();

  QList<bool> isEmpty;
  QList<bool> couldBeInt;
  QList<bool> couldBeLongLong;
  QList<bool> couldBeDouble;
  bool foundFirstGeometry = false;

  while ( true )
  {
    QgsDelimitedTextFile::Status status = mFile->nextRecord( parts );
    if ( status == QgsDelimitedTextFile::RecordEOF )
      break;
    if ( status != QgsDelimitedTextFile::RecordOk )
    {
      nBadFormatRecords++;
      recordInvalidLine( tr( "Invalid record format at line %1" ) );
      continue;
    }
    // Skip over empty records
    if ( recordIsEmpty( parts ) )
    {
      nEmptyRecords++;
      continue;
    }

    // Check geometries are valid
    bool geomValid = true;

    if ( mGeomRep == GeomAsWkt )
    {
      if ( mWktFieldIndex >= parts.size() || parts[mWktFieldIndex].isEmpty() )
      {
        nEmptyGeometry++;
        mNumberFeatures++;
      }
      else
      {
        // Get the wkt - confirm it is valid, get the type, and
        // if compatible with the rest of file, add to the extents

        QString sWkt = parts[mWktFieldIndex];
        QgsGeometry geom;
        if ( !mWktHasPrefix && sWkt.indexOf( sWktPrefixRegexp ) >= 0 )
          mWktHasPrefix = true;
        geom = geomFromWkt( sWkt, mWktHasPrefix );

        if ( !geom.isNull() )
        {
          QgsWkbTypes::Type type = geom.wkbType();
          if ( type != QgsWkbTypes::NoGeometry )
          {
            if ( mGeometryType == QgsWkbTypes::UnknownGeometry || geom.type() == mGeometryType )
            {
              mGeometryType = geom.type();
              if ( !foundFirstGeometry )
              {
                mNumberFeatures++;
                mWkbType = type;
                mExtent = geom.boundingBox();
                foundFirstGeometry = true;
              }
              else
              {
                mNumberFeatures++;
                if ( geom.isMultipart() )
                  mWkbType = type;
                QgsRectangle bbox( geom.boundingBox() );
                mExtent.combineExtentWith( bbox );
              }
              if ( buildSpatialIndex )
              {
                QgsFeature f;
                f.setId( mFile->recordId() );
                f.setGeometry( geom );
                mSpatialIndex->addFeature( f );
              }
            }
            else
            {
              nIncompatibleGeometry++;
              geomValid = false;
            }
          }
        }
        else
        {
          geomValid = false;
          nInvalidGeometry++;
          recordInvalidLine( tr( "Invalid WKT at line %1" ) );
        }
      }
    }
    else if ( mGeomRep == GeomAsXy )
    {
      // Get the x and y values, first checking to make sure they
      // aren't null.

      QString sX = mXFieldIndex < parts.size() ? parts[mXFieldIndex] : QString();
      QString sY = mYFieldIndex < parts.size() ? parts[mYFieldIndex] : QString();
      QString sZ, sM;
      if ( mZFieldIndex > -1 )
        sZ = mZFieldIndex < parts.size() ? parts[mZFieldIndex] : QString();
      if ( mMFieldIndex > -1 )
        sM = mMFieldIndex < parts.size() ? parts[mMFieldIndex] : QString();
      if ( sX.isEmpty() && sY.isEmpty() )
      {
        nEmptyGeometry++;
        mNumberFeatures++;
      }
      else
      {
        QgsPoint pt;
        bool ok = pointFromXY( sX, sY, pt, mDecimalPoint, mXyDms );

        if ( ok )
        {
          if ( !sZ.isEmpty() || sM.isEmpty() )
            appendZM( sZ, sM, pt, mDecimalPoint );

          if ( foundFirstGeometry )
          {
            mExtent.combineExtentWith( pt.x(), pt.y() );
          }
          else
          {
            // Extent for the first point is just the first point
            mExtent.set( pt.x(), pt.y(), pt.x(), pt.y() );
            mWkbType = QgsWkbTypes::Point;
            if ( mZFieldIndex > -1 )
              mWkbType = QgsWkbTypes::addZ( mWkbType );
            if ( mMFieldIndex > -1 )
              mWkbType = QgsWkbTypes::addM( mWkbType );
            mGeometryType = QgsWkbTypes::PointGeometry;
            foundFirstGeometry = true;
          }
          mNumberFeatures++;
          if ( buildSpatialIndex && std::isfinite( pt.x() ) && std::isfinite( pt.y() ) )
          {
            QgsFeature f;
            f.setId( mFile->recordId() );
            f.setGeometry( QgsGeometry::fromPointXY( pt ) );
            mSpatialIndex->addFeature( f );
          }
        }
        else
        {
          geomValid = false;
          nInvalidGeometry++;
          recordInvalidLine( tr( "Invalid X or Y fields at line %1" ) );
        }
      }
    }
    else
    {
      mWkbType = QgsWkbTypes::NoGeometry;
      mNumberFeatures++;
    }

    if ( !geomValid )
      continue;

    if ( buildSubsetIndex )
      mSubsetIndex.append( mFile->recordId() );


    // If we are going to use this record, then assess the potential types of each column

    for ( int i = 0; i < parts.size(); i++ )
    {

      QString &value = parts[i];
      // Ignore empty fields - spreadsheet generated CSV files often
      // have random empty fields at the end of a row
      if ( value.isEmpty() )
        continue;

      // Expand the columns to include this non empty field if necessary

      while ( couldBeInt.size() <= i )
      {
        isEmpty.append( true );
        couldBeInt.append( false );
        couldBeLongLong.append( false );
        couldBeDouble.append( false );
      }

      // If this column has been empty so far then initiallize it
      // for possible types

      if ( isEmpty[i] )
      {
        isEmpty[i] = false;
        couldBeInt[i] = true;
        couldBeLongLong[i] = true;
        couldBeDouble[i] = true;
      }

      if ( ! mDetectTypes )
      {
        continue;
      }

      // Now test for still valid possible types for the field
      // Types are possible until first record which cannot be parsed

      if ( couldBeInt[i] )
      {
        value.toInt( &couldBeInt[i] );
      }

      if ( couldBeLongLong[i] && ! couldBeInt[i] )
      {
        value.toLongLong( &couldBeLongLong[i] );
      }

      if ( couldBeDouble[i] && ! couldBeLongLong[i] )
      {
        if ( ! mDecimalPoint.isEmpty() )
        {
          value.replace( mDecimalPoint, QLatin1String( "." ) );
        }
        value.toDouble( &couldBeDouble[i] );
      }
    }
  }

  // Now create the attribute fields.  Field types are integer by preference,
  // failing that double, failing that text.

  QStringList fieldNames = mFile->fieldNames();
  mFieldCount = fieldNames.size();
  attributeColumns.clear();
  attributeFields.clear();

  QString csvtMessage;
  QStringList csvtTypes = readCsvtFieldTypes( mFile->fileName(), &csvtMessage );

  for ( int i = 0; i < fieldNames.size(); i++ )
  {
    // Skip over WKT field ... don't want to display in attribute table
    if ( i == mWktFieldIndex )
      continue;

    // Add the field index lookup for the column
    attributeColumns.append( i );
    QVariant::Type fieldType = QVariant::String;
    QString typeName = QStringLiteral( "text" );
    if ( i < csvtTypes.size() )
    {
      typeName = csvtTypes[i];
    }
    else if ( mDetectTypes && i < couldBeInt.size() )
    {
      if ( couldBeInt[i] )
      {
        typeName = QStringLiteral( "integer" );
      }
      else if ( couldBeLongLong[i] )
      {
        typeName = QStringLiteral( "longlong" );
      }
      else if ( couldBeDouble[i] )
      {
        typeName = QStringLiteral( "double" );
      }
    }
    if ( typeName == QStringLiteral( "integer" ) )
    {
      fieldType = QVariant::Int;
    }
    else if ( typeName == QStringLiteral( "longlong" ) )
    {
      fieldType = QVariant::LongLong;
    }
    else if ( typeName == QStringLiteral( "real" ) || typeName == QStringLiteral( "double" ) )
    {
      typeName = QStringLiteral( "double" );
      fieldType = QVariant::Double;
    }
    else
    {
      typeName = QStringLiteral( "text" );
    }
    attributeFields.append( QgsField( fieldNames[i], fieldType, typeName ) );
  }


  QgsDebugMsg( "Field count for the delimited text file is " + QString::number( attributeFields.size() ) );
  QgsDebugMsg( "geometry type is: " + QString::number( mWkbType ) );
  QgsDebugMsg( "feature count is: " + QString::number( mNumberFeatures ) );

  QStringList warnings;
  if ( ! csvtMessage.isEmpty() )
    warnings.append( csvtMessage );
  if ( nBadFormatRecords > 0 )
    warnings.append( tr( "%1 records discarded due to invalid format" ).arg( nBadFormatRecords ) );
  if ( nEmptyGeometry > 0 )
    warnings.append( tr( "%1 records have missing geometry definitions" ).arg( nEmptyGeometry ) );
  if ( nInvalidGeometry > 0 )
    warnings.append( tr( "%1 records discarded due to invalid geometry definitions" ).arg( nInvalidGeometry ) );
  if ( nIncompatibleGeometry > 0 )
    warnings.append( tr( "%1 records discarded due to incompatible geometry types" ).arg( nIncompatibleGeometry ) );

  reportErrors( warnings );

  // Decide whether to use subset ids to index records rather than simple iteration through all
  // If more than 10% of records are being skipped, then use index.  (Not based on any experimentation,
  // could do with some analysis?)

  if ( buildSubsetIndex )
  {
    long recordCount = mFile->recordCount();
    recordCount -= recordCount / SUBSET_ID_THRESHOLD_FACTOR;
    mUseSubsetIndex = mSubsetIndex.size() < recordCount;
    if ( ! mUseSubsetIndex )
      mSubsetIndex = QList<quintptr>();
  }

  mUseSpatialIndex = buildSpatialIndex;

  mValid = mGeometryType != QgsWkbTypes::UnknownGeometry;
  mLayerValid = mValid;

  // If it is valid, then watch for changes to the file
  connect( mFile.get(), &QgsDelimitedTextFile::fileUpdated, this, &QgsDelimitedTextProvider::onFileUpdated );
}

// rescanFile.  Called if something has changed file definition, such as
// selecting a subset, the file has been changed by another program, etc

void QgsDelimitedTextProvider::rescanFile() const
{
  mRescanRequired = false;
  resetIndexes();

  bool buildSpatialIndex = nullptr != mSpatialIndex;
  bool buildSubsetIndex = mBuildSubsetIndex && ( mSubsetExpression || mGeomRep != GeomNone );

  // In case file has been rewritten check that it is still valid

  mValid = mLayerValid && mFile->isValid();
  if ( ! mValid )
    return;

  // Open the file and get number of rows, etc. We assume that the
  // file has a header row and process accordingly. Caller should make
  // sure that the delimited file is properly formed.

  QStringList messages;

  if ( mGeomRep == GeomAsWkt )
  {
    mWktFieldIndex = mFile->fieldIndex( mWktFieldName );
    if ( mWktFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( QStringLiteral( "Wkt" ), mWktFieldName ) );
    }
  }
  else if ( mGeomRep == GeomAsXy )
  {
    mXFieldIndex = mFile->fieldIndex( mXFieldName );
    mYFieldIndex = mFile->fieldIndex( mYFieldName );
    if ( mXFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( QStringLiteral( "X" ), mWktFieldName ) );
    }
    if ( mYFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( QStringLiteral( "Y" ), mWktFieldName ) );
    }
  }
  if ( !messages.isEmpty() )
  {
    reportErrors( messages );
    QgsDebugMsg( QStringLiteral( "Delimited text source invalid on rescan - missing geometry fields" ) );
    mValid = false;
    return;
  }

  // Reset the field columns

  for ( int i = 0; i < attributeFields.size(); i++ )
  {
    attributeColumns[i] = mFile->fieldIndex( attributeFields.at( i ).name() );
  }

  // Scan through the features in the file

  mSubsetIndex.clear();
  mUseSubsetIndex = false;
  QgsFeatureIterator fi = getFeatures( QgsFeatureRequest() );
  mNumberFeatures = 0;
  mExtent = QgsRectangle();
  QgsFeature f;
  bool foundFirstGeometry = false;
  while ( fi.nextFeature( f ) )
  {
    if ( mGeometryType != QgsWkbTypes::NullGeometry && f.hasGeometry() )
    {
      if ( !foundFirstGeometry )
      {
        mExtent = f.geometry().boundingBox();
        foundFirstGeometry = true;
      }
      else
      {
        QgsRectangle bbox( f.geometry().boundingBox() );
        mExtent.combineExtentWith( bbox );
      }
      if ( buildSpatialIndex )
        mSpatialIndex->addFeature( f );
    }
    if ( buildSubsetIndex )
      mSubsetIndex.append( ( quintptr ) f.id() );
    mNumberFeatures++;
  }
  if ( buildSubsetIndex )
  {
    long recordCount = mFile->recordCount();
    recordCount -= recordCount / SUBSET_ID_THRESHOLD_FACTOR;
    mUseSubsetIndex = recordCount < mSubsetIndex.size();
    if ( ! mUseSubsetIndex )
      mSubsetIndex.clear();
  }

  mUseSpatialIndex = buildSpatialIndex;
}

QgsGeometry QgsDelimitedTextProvider::geomFromWkt( QString &sWkt, bool wktHasPrefixRegexp )
{
  QgsGeometry geom;
  try
  {
    if ( wktHasPrefixRegexp )
    {
      sWkt.remove( sWktPrefixRegexp );
    }

    geom = QgsGeometry::fromWkt( sWkt );
  }
  catch ( ... )
  {
    geom = QgsGeometry();
  }
  return geom;
}

double QgsDelimitedTextProvider::dmsStringToDouble( const QString &sX, bool *xOk )
{
  static QString negative( QStringLiteral( "swSW-" ) );
  QRegExp re( sCrdDmsRegexp );
  double x = 0.0;

  *xOk = re.indexIn( sX ) == 0;
  if ( ! *xOk )
    return 0.0;
  QString dms1 = re.capturedTexts().at( 2 );
  QString dms2 = re.capturedTexts().at( 3 );
  QString dms3 = re.capturedTexts().at( 4 );
  x = dms3.toDouble( xOk );
  // Allow for Degrees/minutes format as well as DMS
  if ( ! dms2.isEmpty() )
  {
    x = dms2.toInt( xOk ) + x / 60.0;
  }
  x = dms1.toInt( xOk ) + x / 60.0;
  QString sign1 = re.capturedTexts().at( 1 );
  QString sign2 = re.capturedTexts().at( 5 );

  if ( sign1.isEmpty() )
  {
    if ( ! sign2.isEmpty() && negative.contains( sign2 ) )
      x = -x;
  }
  else if ( sign2.isEmpty() )
  {
    if ( ! sign1.isEmpty() && negative.contains( sign1 ) )
      x = -x;
  }
  else
  {
    *xOk = false;
  }
  return x;
}

void QgsDelimitedTextProvider::appendZM( QString &sZ, QString &sM, QgsPoint &point, const QString &decimalPoint )
{
  if ( ! decimalPoint.isEmpty() )
  {
    sZ.replace( decimalPoint, QLatin1String( "." ) );
    sM.replace( decimalPoint, QLatin1String( "." ) );
  }

  bool zOk, mOk;
  double z, m;
  if ( !sZ.isEmpty() )
  {
    z = sZ.toDouble( &zOk );
    if ( zOk )
      point.addZValue( z );
  }
  if ( !sM.isEmpty() )
  {
    m = sM.toDouble( &mOk );
    if ( mOk )
      point.addMValue( m );
  }
}

bool QgsDelimitedTextProvider::pointFromXY( QString &sX, QString &sY, QgsPoint &pt, const QString &decimalPoint, bool xyDms )
{
  if ( ! decimalPoint.isEmpty() )
  {
    sX.replace( decimalPoint, QLatin1String( "." ) );
    sY.replace( decimalPoint, QLatin1String( "." ) );
  }

  bool xOk, yOk;
  double x, y;
  if ( xyDms )
  {
    x = dmsStringToDouble( sX, &xOk );
    y = dmsStringToDouble( sY, &yOk );
  }
  else
  {
    x = sX.toDouble( &xOk );
    y = sY.toDouble( &yOk );
  }

  if ( xOk && yOk )
  {
    pt.setX( x );
    pt.setY( y );
    return true;
  }

  return false;
}

QString QgsDelimitedTextProvider::storageType() const
{
  return QStringLiteral( "Delimited text file" );
}

QgsFeatureIterator QgsDelimitedTextProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  // If the file has become invalid, rescan to check that it is still invalid.
  //
  if ( ( mLayerValid && ! mValid ) || mRescanRequired )
    rescanFile();

  return QgsFeatureIterator( new QgsDelimitedTextFeatureIterator( new QgsDelimitedTextFeatureSource( this ), true, request ) );
}

void QgsDelimitedTextProvider::clearInvalidLines() const
{
  mInvalidLines.clear();
  mNExtraInvalidLines = 0;
}

bool QgsDelimitedTextProvider::recordIsEmpty( QStringList &record )
{
  const auto constRecord = record;
  for ( const QString &s : constRecord )
  {
    if ( ! s.isEmpty() )
      return false;
  }
  return true;
}

void QgsDelimitedTextProvider::recordInvalidLine( const QString &message )
{
  if ( mInvalidLines.size() < mMaxInvalidLines )
  {
    mInvalidLines.append( message.arg( mFile->recordId() ) );
  }
  else
  {
    mNExtraInvalidLines++;
  }
}

void QgsDelimitedTextProvider::reportErrors( const QStringList &messages, bool showDialog ) const
{
  if ( !mInvalidLines.isEmpty() || ! messages.isEmpty() )
  {
    QString tag( QStringLiteral( "DelimitedText" ) );
    QgsMessageLog::logMessage( tr( "Errors in file %1" ).arg( mFile->fileName() ), tag );
    const auto constMessages = messages;
    for ( const QString &message : constMessages )
    {
      QgsMessageLog::logMessage( message, tag );
    }
    if ( ! mInvalidLines.isEmpty() )
    {
      QgsMessageLog::logMessage( tr( "The following lines were not loaded into QGIS due to errors:" ), tag );
      for ( int i = 0; i < mInvalidLines.size(); ++i )
        QgsMessageLog::logMessage( mInvalidLines.at( i ), tag );
      if ( mNExtraInvalidLines > 0 )
        QgsMessageLog::logMessage( tr( "There are %1 additional errors in the file" ).arg( mNExtraInvalidLines ), tag );
    }

    // Display errors in a dialog...
    if ( mShowInvalidLines && showDialog )
    {
      QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
      output->setTitle( tr( "Delimited text file errors" ) );
      output->setMessage( tr( "Errors in file %1" ).arg( mFile->fileName() ), QgsMessageOutput::MessageText );
      const auto constMessages = messages;
      for ( const QString &message : constMessages )
      {
        output->appendMessage( message );
      }
      if ( ! mInvalidLines.isEmpty() )
      {
        output->appendMessage( tr( "The following lines were not loaded into QGIS due to errors:" ) );
        for ( int i = 0; i < mInvalidLines.size(); ++i )
          output->appendMessage( mInvalidLines.at( i ) );
        if ( mNExtraInvalidLines > 0 )
          output->appendMessage( tr( "There are %1 additional errors in the file" ).arg( mNExtraInvalidLines ) );
      }
      output->showMessage();
    }

    // We no longer need these lines.
    clearInvalidLines();
  }
}

bool QgsDelimitedTextProvider::setSubsetString( const QString &subset, bool updateFeatureCount )
{
  QString nonNullSubset = subset.isNull() ? QString() : subset;

  // If not changing string, then all OK, nothing to do
  if ( nonNullSubset == mSubsetString )
    return true;

  bool valid = true;

  // If there is a new subset string then encode it..

  std::unique_ptr< QgsExpression > expression;
  if ( ! nonNullSubset.isEmpty() )
  {

    expression = qgis::make_unique< QgsExpression >( nonNullSubset );
    QString error;
    if ( expression->hasParserError() )
    {
      error = expression->parserErrorString();
    }
    else
    {
      QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( QgsFeature(), fields() );
      expression->prepare( &context );
      if ( expression->hasEvalError() )
      {
        error = expression->evalErrorString();
      }
    }
    if ( ! error.isEmpty() )
    {
      valid = false;
      expression.reset();
      QString tag( QStringLiteral( "DelimitedText" ) );
      QgsMessageLog::logMessage( tr( "Invalid subset string %1 for %2" ).arg( nonNullSubset, mFile->fileName() ), tag );
    }
  }

  // if the expression is valid, then reset the subset string and data source Uri

  if ( valid )
  {
    QString previousSubset = mSubsetString;
    mSubsetString = nonNullSubset;
    mSubsetExpression = std::move( expression );

    // Update the feature count and extents if requested

    // Usage of updateFeatureCount is a bit painful, basically expect that it
    // will only be false for a temporary subset, and the original subset
    // will be replaced before an update is requeired.
    //
    // It appears to be false for a temporary subset string, which is used to
    // get some data, and then immediately reset.  No point scanning file and
    // resetting subset index for this.  On the other hand, we don't want to
    // lose indexes in this instance, or have to rescan file.  So we cache
    // the settings until a real subset is required.

    if ( updateFeatureCount )
    {
      if ( ! mCachedSubsetString.isNull() && mSubsetString == mCachedSubsetString )
      {
        QgsDebugMsg( QStringLiteral( "DelimitedText: Resetting cached subset string %1" ).arg( mSubsetString ) );
        mUseSpatialIndex = mCachedUseSpatialIndex;
        mUseSubsetIndex = mCachedUseSubsetIndex;
        resetCachedSubset();
      }
      else
      {
        QgsDebugMsg( QStringLiteral( "DelimitedText: Setting new subset string %1" ).arg( mSubsetString ) );
        // Reset the subset index
        rescanFile();
        // Encode the subset string into the data source URI.
        setUriParameter( QStringLiteral( "subset" ), nonNullSubset );
      }
    }
    else
    {
      // If not already using temporary subset, then cache the current subset
      QgsDebugMsg( QStringLiteral( "DelimitedText: Setting temporary subset string %1" ).arg( mSubsetString ) );
      if ( mCachedSubsetString.isNull() )
      {
        QgsDebugMsg( QStringLiteral( "DelimitedText: Caching previous subset %1" ).arg( previousSubset ) );
        mCachedSubsetString = previousSubset;
        mCachedUseSpatialIndex = mUseSpatialIndex;
        mCachedUseSubsetIndex = mUseSubsetIndex;
      }
      mUseSubsetIndex = false;
      mUseSpatialIndex = false;
    }
  }

  clearMinMaxCache();
  emit dataChanged();
  return valid;
}

void QgsDelimitedTextProvider::setUriParameter( const QString &parameter, const QString &value )
{
  QUrl url = QUrl::fromEncoded( dataSourceUri().toLatin1() );
  QUrlQuery query( url );
  if ( query.hasQueryItem( parameter ) )
    query.removeAllQueryItems( parameter );
  if ( ! value.isEmpty() )
    query.addQueryItem( parameter, value );
  url.setQuery( query );
  setDataSourceUri( QString::fromLatin1( url.toEncoded() ) );
}

void QgsDelimitedTextProvider::onFileUpdated()
{
  if ( ! mRescanRequired )
  {
    QStringList messages;
    messages.append( tr( "The file has been updated by another application - reloading" ) );
    reportErrors( messages );
    mRescanRequired = true;
    emit dataChanged();
  }
}

QgsRectangle QgsDelimitedTextProvider::extent() const
{
  if ( mRescanRequired )
    rescanFile();
  return mExtent;
}

QgsWkbTypes::Type QgsDelimitedTextProvider::wkbType() const
{
  return mWkbType;
}

long QgsDelimitedTextProvider::featureCount() const
{
  if ( mRescanRequired )
    const_cast<QgsDelimitedTextProvider *>( this )->rescanFile();
  return mNumberFeatures;
}


QgsFields QgsDelimitedTextProvider::fields() const
{
  return attributeFields;
}

bool QgsDelimitedTextProvider::isValid() const
{
  return mLayerValid;
}

QgsVectorDataProvider::Capabilities QgsDelimitedTextProvider::capabilities() const
{
  return SelectAtId | CreateSpatialIndex | CircularGeometries;
}

QgsCoordinateReferenceSystem QgsDelimitedTextProvider::crs() const
{
  return mCrs;
}

QString  QgsDelimitedTextProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString  QgsDelimitedTextProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QVariantMap QgsDelimitedTextProviderMetadata::decodeUri( const QString &uri )
{
  QVariantMap components;
  components.insert( QStringLiteral( "path" ), QUrl( uri ).toLocalFile() );
  return components;
}

QString QgsDelimitedTextProviderMetadata::encodeUri( const QVariantMap &parts )
{
  return QStringLiteral( "file://%1" ).arg( parts.value( QStringLiteral( "path" ) ).toString() );
}

QgsDataProvider *QgsDelimitedTextProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options )
{
  return new QgsDelimitedTextProvider( uri, options );
}


QgsDelimitedTextProviderMetadata::QgsDelimitedTextProviderMetadata():
  QgsProviderMetadata( QgsDelimitedTextProvider::TEXT_PROVIDER_KEY, QgsDelimitedTextProvider::TEXT_PROVIDER_DESCRIPTION )
{
}

QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsDelimitedTextProviderMetadata();
}
