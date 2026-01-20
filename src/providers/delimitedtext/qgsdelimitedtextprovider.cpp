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

#include <memory>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgscoordinateutils.h"
#include "qgsdataprovider.h"
#include "qgsdelimitedtextfeatureiterator.h"
#include "qgsdelimitedtextfile.h"
#include "qgsexpression.h"
#include "qgsexpressioncontextutils.h"
#include "qgsfeature.h"
#include "qgsfeedback.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmessageoutput.h"
#include "qgsrectangle.h"
#include "qgsspatialindex.h"
#include "qgsvariantutils.h"

#include <QDataStream>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QStringList>
#include <QTextStream>
#include <QUrl>
#include <QUrlQuery>
#include <QtGlobal>

#include "moc_qgsdelimitedtextprovider.cpp"

const QString QgsDelimitedTextProvider::TEXT_PROVIDER_KEY = u"delimitedtext"_s;
const QString QgsDelimitedTextProvider::TEXT_PROVIDER_DESCRIPTION = u"Delimited text data provider"_s;

// If more than this fraction of records are not in a subset then use an index to
// iterate over records rather than simple iterator and filter.

static const int SUBSET_ID_THRESHOLD_FACTOR = 10;

QRegularExpression QgsDelimitedTextProvider::sWktPrefixRegexp( u"^\\s*(?:\\d+\\s+|SRID\\=\\d+\\;)"_s, QRegularExpression::CaseInsensitiveOption );
QRegularExpression QgsDelimitedTextProvider::sCrdDmsRegexp( u"^\\s*(?:([-+nsew])\\s*)?(\\d{1,3})(?:[^0-9.]+([0-5]?\\d))?[^0-9.]+([0-5]?\\d(?:\\.\\d+)?)[^0-9.]*([-+nsew])?\\s*$"_s, QRegularExpression::CaseInsensitiveOption );

QgsDelimitedTextProvider::QgsDelimitedTextProvider( const QString &uri, const ProviderOptions &options, Qgis::DataProviderReadFlags flags )
  : QgsVectorDataProvider( uri, options, flags )
{
  // Add supported types to enable creating expression fields in field calculator
  setNativeTypes( QList<NativeType>() << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Int ), u"integer"_s, QMetaType::Type::Int, 0, 10 ) << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::LongLong ), u"longlong"_s, QMetaType::Type::LongLong ) << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Double ), u"double"_s, QMetaType::Type::Double, -1, -1, -1, -1 ) << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::Bool ), u"bool"_s, QMetaType::Type::Bool, -1, -1, -1, -1 ) << QgsVectorDataProvider::NativeType( tr( "Text, unlimited length (text)" ), u"text"_s, QMetaType::Type::QString, -1, -1, -1, -1 )

                                      // date type
                                      << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), u"date"_s, QMetaType::Type::QDate, -1, -1, -1, -1 ) << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QTime ), u"time"_s, QMetaType::Type::QTime, -1, -1, -1, -1 ) << QgsVectorDataProvider::NativeType( QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), u"datetime"_s, QMetaType::Type::QDateTime, -1, -1, -1, -1 ) );

  QgsDebugMsgLevel( "Delimited text file uri is " + uri, 2 );

  const QUrl url = QUrl::fromEncoded( uri.toUtf8() );
  mFile = std::make_unique<QgsDelimitedTextFile>();
  mFile->setFromUrl( url );

  QString subset;

  const QUrlQuery query( url );
  if ( query.hasQueryItem( u"geomType"_s ) )
  {
    const QString gtype = query.queryItemValue( u"geomType"_s ).toLower();
    if ( gtype == "point"_L1 )
      mGeometryType = Qgis::GeometryType::Point;
    else if ( gtype == "line"_L1 )
      mGeometryType = Qgis::GeometryType::Line;
    else if ( gtype == "polygon"_L1 )
      mGeometryType = Qgis::GeometryType::Polygon;
    else if ( gtype == "none"_L1 )
      mGeometryType = Qgis::GeometryType::Null;
  }

  if ( mGeometryType != Qgis::GeometryType::Null )
  {
    if ( query.hasQueryItem( u"wktField"_s ) )
    {
      mWktFieldName = query.queryItemValue( u"wktField"_s );
      mGeomRep = GeomAsWkt;
      QgsDebugMsgLevel( "wktField is: " + mWktFieldName, 2 );
    }
    else if ( query.hasQueryItem( u"xField"_s ) && query.hasQueryItem( u"yField"_s ) )
    {
      mGeomRep = GeomAsXy;
      mGeometryType = Qgis::GeometryType::Point;
      mXFieldName = query.queryItemValue( u"xField"_s );
      mYFieldName = query.queryItemValue( u"yField"_s );
      if ( query.hasQueryItem( u"zField"_s ) )
        mZFieldName = query.queryItemValue( u"zField"_s );
      if ( query.hasQueryItem( u"mField"_s ) )
        mMFieldName = query.queryItemValue( u"mField"_s );
      QgsDebugMsgLevel( "xField is: " + mXFieldName, 2 );
      QgsDebugMsgLevel( "yField is: " + mYFieldName, 2 );
      QgsDebugMsgLevel( "zField is: " + mZFieldName, 2 );
      QgsDebugMsgLevel( "mField is: " + mMFieldName, 2 );

      if ( query.hasQueryItem( u"xyDms"_s ) )
      {
        mXyDms = !query.queryItemValue( u"xyDms"_s ).toLower().startsWith( 'n' );
      }
    }
    else
    {
      mGeometryType = Qgis::GeometryType::Null;
    }
  }

  mDetectTypes = true;
  if ( query.hasQueryItem( u"detectTypes"_s ) )
    mDetectTypes = !query.queryItemValue( u"detectTypes"_s ).toLower().startsWith( 'n' );

  if ( query.hasQueryItem( u"decimalPoint"_s ) )
    mDecimalPoint = query.queryItemValue( u"decimalPoint"_s );

  if ( query.hasQueryItem( u"crs"_s ) )
    mCrs.createFromString( query.queryItemValue( u"crs"_s ) );

  if ( query.hasQueryItem( u"subsetIndex"_s ) )
  {
    mBuildSubsetIndex = !query.queryItemValue( u"subsetIndex"_s ).toLower().startsWith( 'n' );
  }

  if ( query.hasQueryItem( u"spatialIndex"_s ) )
  {
    mBuildSpatialIndex = !query.queryItemValue( u"spatialIndex"_s ).toLower().startsWith( 'n' );
  }

  if ( query.hasQueryItem( u"subset"_s ) )
  {
    // We need to specify FullyDecoded so that %25 is decoded as %
    subset = query.queryItemValue( u"subset"_s, QUrl::FullyDecoded );
    QgsDebugMsgLevel( "subset is: " + subset, 2 );
  }

  if ( query.hasQueryItem( u"quiet"_s ) )
    mShowInvalidLines = false;

  // Parse and store user-defined field types and boolean literals
  const QList<QPair<QString, QString>> queryItems { query.queryItems( QUrl::ComponentFormattingOption::FullyDecoded ) };
  for ( const QPair<QString, QString> &queryItem : std::as_const( queryItems ) )
  {
    if ( queryItem.first.compare( u"field"_s, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
    {
      const QStringList parts { queryItem.second.split( ':' ) };
      if ( parts.size() == 2 )
      {
        // cppcheck-suppress containerOutOfBounds
        mUserDefinedFieldTypes.insert( parts[0], parts[1] );
      }
    }
  }

  // Parse and store custom boolean literals
  if ( query.hasQueryItem( u"booleanTrue"_s ) && query.hasQueryItem( u"booleanFalse"_s ) )
  {
    mUserDefinedBooleanLiterals = qMakePair<QString, QString>(
      query.queryItemValue( u"booleanTrue"_s, QUrl::ComponentFormattingOption::FullyDecoded ),
      query.queryItemValue( u"booleanFalse"_s, QUrl::ComponentFormattingOption::FullyDecoded )
    );
  }

  // Do an initial scan of the file to determine field names, types,
  // geometry type (for Wkt), extents, etc.  Parameter value subset.isEmpty()
  // avoid redundant building indexes if we will be building a subset string,
  // in which case indexes will be rebuilt.
  scanFile( subset.isEmpty() && !flags.testFlag( Qgis::DataProviderReadFlag::SkipGetExtent ), /* force full scan */ false );

  if ( !subset.isEmpty() )
  {
    setSubsetString( subset );
  }
}

QgsDelimitedTextProvider::~QgsDelimitedTextProvider() = default;

QgsAbstractFeatureSource *QgsDelimitedTextProvider::featureSource() const
{
  // If the file has become invalid, rescan to check that it is still invalid.
  //
  if ( ( mLayerValid && !mValid ) || mRescanRequired )
    const_cast<QgsDelimitedTextProvider *>( this )->rescanFile();

  return new QgsDelimitedTextFeatureSource( this );
}

QStringList QgsDelimitedTextProvider::readCsvtFieldTypes( const QString &filename, QString *message )
{
  // Look for a file with the same name as the data file, but an extra 't' or 'T' at the end
  QStringList types;
  QFileInfo csvtInfo( filename + 't' );
  if ( !csvtInfo.exists() )
    csvtInfo.setFile( filename + 'T' );
  if ( !csvtInfo.exists() )
    return types;
  QFile csvtFile( csvtInfo.filePath() );
  if ( !csvtFile.open( QIODevice::ReadOnly ) )
    return types;


  // If anything goes wrong here, just ignore it, as the file
  // is not valid, so just ignore any exceptions.

  // For it to be valid, there must be just one non blank line at the beginning of the
  // file.

  QString strTypeList;
  try
  {
    QTextStream csvtStream( &csvtFile );
    strTypeList = csvtStream.readLine();
    if ( strTypeList.isEmpty() )
      return types;
    QString extra = csvtStream.readLine();
    while ( !extra.isNull() )
    {
      if ( !extra.isEmpty() )
        return types;
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
  // https://regex101.com/r/BcVPcF/1
  const thread_local QRegularExpression reTypeList( QRegularExpression::anchoredPattern( QStringLiteral( R"re(^(?:\s*("?)(?:coord[xyz]|point\([xyz]\)|wkt|integer64|integer|integer\((?:boolean|int16)\)|real(?:\(float32\))?|double|longlong|long|int8|string|date|datetime|time)(?:\(\d+(?:\.\d+)?\))?\1\s*(?:,|$))+)re" ) ) );
  const QRegularExpressionMatch match = reTypeList.match( strTypeList );
  if ( !match.hasMatch() )
  {
    // Looks like this was supposed to be a CSVT file, so report bad formatted string
    if ( message )
    {
      *message = tr( "File type string in %1 is not correctly formatted" ).arg( csvtInfo.fileName() );
    }
    return types;
  }

  // All good, so pull out the types from the string.  Currently only returning integer, real, and string types
  QgsDebugMsgLevel( u"Reading field types from %1"_s.arg( csvtInfo.fileName() ), 2 );
  QgsDebugMsgLevel( u"Field type string: %1"_s.arg( strTypeList ), 2 );

  int pos = 0;
  // https://regex101.com/r/QwxaSe/1/
  const thread_local QRegularExpression reType( QStringLiteral( R"re((coord[xyz]|point\([xyz]\)|wkt|int8|\binteger\b(?=[^\(])|(?<=integer\()bool(?=ean)|integer64|\binteger\b(?=\((?:\d+|int16)\))|integer64|longlong|\blong\b|real|double|string|\bdate\b|datetime|\btime\b))re" ) );
  QRegularExpressionMatch typeMatch = reType.match( strTypeList, pos );
  while ( typeMatch.hasMatch() )
  {
    types << typeMatch.captured( 1 );
    pos = typeMatch.capturedEnd();
    QgsDebugMsgLevel( u"Found type: %1 at pos %2"_s.arg( typeMatch.captured( 1 ) ).arg( pos ), 2 );

    typeMatch = reType.match( strTypeList, pos );
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
    mSpatialIndex = std::make_unique<QgsSpatialIndex>();
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
  setUriParameter( u"spatialIndex"_s, u"yes"_s );
  rescanFile();
  return true;
}

Qgis::SpatialIndexPresence QgsDelimitedTextProvider::hasSpatialIndex() const
{
  return mSpatialIndex ? Qgis::SpatialIndexPresence::Present : Qgis::SpatialIndexPresence::NotPresent;
}

// Really want to merge scanFile and rescan into single code.  Currently the reason
// this is not done is that scanFile is done initially to create field names and, rescan
// file includes building subset expression and assumes field names/types are already
// defined.  Merging would not only make code a lot cleaner, but would also avoid
// double scan when loading a file with a subset expression.

// buildIndexes parameter of scanFile is set to false when we know we will be
// immediately rescanning (when the file is loaded and then the subset expression is
// set)

void QgsDelimitedTextProvider::scanFile( bool buildIndexes, bool forceFullScan, QgsFeedback *feedback )
{
  QStringList messages;

  // assume the layer is invalid until proven otherwise

  mLayerValid = false;
  mValid = false;
  mRescanRequired = false;

  clearInvalidLines();

  // Initiallize indexes

  resetIndexes();
  const bool buildSpatialIndex = buildIndexes && nullptr != mSpatialIndex;

  // No point building a subset index if there is no geometry, as all
  // records will be included.

  const bool buildSubsetIndex = buildIndexes && mBuildSubsetIndex && mGeomRep != GeomNone;

  if ( !mFile->isValid() )
  {
    // uri is invalid so the layer must be too...

    messages.append( tr( "File cannot be opened or delimiter parameters are not valid" ) );
    reportErrors( messages );
    QgsDebugMsgLevel( u"Delimited text source invalid - filename or delimiter parameters"_s, 2 );
    return;
  }

  // Open the file and get number of rows, etc. We assume that the
  // file has a header row and process accordingly. Caller should make
  // sure that the delimited file is properly formed.  const QUrl url { mFile->url() };

  // Reset is required because the quick scan might already have read the whole file
  if ( forceFullScan )
  {
    const QUrl url { mFile->url() };
    mFile = std::make_unique<QgsDelimitedTextFile>();
    mFile->setFromUrl( url );
  }

  if ( mGeomRep == GeomAsWkt )
  {
    mWktFieldIndex = mFile->fieldIndex( mWktFieldName );
    if ( mWktFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( u"Wkt"_s, mWktFieldName ) );
    }
  }
  else if ( mGeomRep == GeomAsXy )
  {
    mXFieldIndex = mFile->fieldIndex( mXFieldName );
    mYFieldIndex = mFile->fieldIndex( mYFieldName );
    if ( mXFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( u"X"_s, mXFieldName ) );
    }
    if ( mYFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( u"Y"_s, mYFieldName ) );
    }
    if ( !mZFieldName.isEmpty() )
    {
      mZFieldIndex = mFile->fieldIndex( mZFieldName );
      if ( mZFieldIndex < 0 )
      {
        messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( u"Z"_s, mZFieldName ) );
      }
    }
    if ( !mMFieldName.isEmpty() )
    {
      mMFieldIndex = mFile->fieldIndex( mMFieldName );
      if ( mMFieldIndex < 0 )
      {
        messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( u"M"_s, mMFieldName ) );
      }
    }
  }
  if ( !messages.isEmpty() )
  {
    reportErrors( messages );
    QgsDebugMsgLevel( u"Delimited text source invalid - missing geometry fields"_s, 2 );
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
  long nBadFormatRecords = 0;
  long nIncompatibleGeometry = 0;
  long nInvalidGeometry = 0;
  long nEmptyGeometry = 0;
  mNumberFeatures = 0;
  mExtent = QgsBox3D();

  struct FieldTypeInformation
  {
      bool isEmpty = true;
      bool couldBeInt = false;
      bool couldBeLongLong = false;
      bool couldBeDouble = false;
      bool couldBeDateTime = false;
      bool couldBeDate = false;
      bool couldBeTime = false;
      bool couldBeBool = false;
  };

  QVector<FieldTypeInformation> fieldTypeInformation;

  bool foundFirstGeometry = false;
  QMap<int, QPair<QString, QString>> boolCandidates;
  const QList<QPair<QString, QString>> boolLiterals { booleanLiterals() };

  while ( true )
  {
    if ( feedback && feedback->isCanceled() )
    {
      break;
    }
    const QgsDelimitedTextFile::Status status = mFile->nextRecord( parts );
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
      continue;
    }

    // Check geometries are valid
    bool geomValid = true;

    if ( mGeomRep == GeomAsWkt )
    {
      if ( mWktFieldIndex >= parts.size() || parts.value( mWktFieldIndex ).isEmpty() )
      {
        nEmptyGeometry++;
        mNumberFeatures++;
      }
      else
      {
        // Get the wkt - confirm it is valid, get the type, and
        // if compatible with the rest of file, add to the extents

        QString sWkt = parts.value( mWktFieldIndex );
        QgsGeometry geom;
        if ( !mWktHasPrefix && sWkt.indexOf( sWktPrefixRegexp ) >= 0 )
          mWktHasPrefix = true;
        geom = geomFromWkt( sWkt, mWktHasPrefix );

        if ( !geom.isNull() )
        {
          const Qgis::WkbType type = geom.wkbType();
          if ( type != Qgis::WkbType::NoGeometry )
          {
            if ( mGeometryType == Qgis::GeometryType::Unknown || geom.type() == mGeometryType )
            {
              mGeometryType = geom.type();
              if ( !foundFirstGeometry )
              {
                mNumberFeatures++;
                mWkbType = type;
                mExtent = geom.boundingBox3D();
                foundFirstGeometry = true;
              }
              else
              {
                mNumberFeatures++;
                if ( geom.isMultipart() )
                  mWkbType = type;
                const QgsBox3D bbox( geom.boundingBox3D() );
                mExtent.combineWith( bbox );
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

      QString sX = parts.value( mXFieldIndex );
      QString sY = parts.value( mYFieldIndex );
      QString sZ, sM;
      if ( mZFieldIndex > -1 )
        sZ = parts.value( mZFieldIndex );
      if ( mMFieldIndex > -1 )
        sM = parts.value( mMFieldIndex );
      if ( sX.isEmpty() && sY.isEmpty() )
      {
        nEmptyGeometry++;
        mNumberFeatures++;
      }
      else
      {
        QgsPoint pt;
        const bool ok = pointFromXY( sX, sY, pt, mDecimalPoint, mXyDms );

        if ( ok )
        {
          if ( !sZ.isEmpty() || sM.isEmpty() )
            appendZM( sZ, sM, pt, mDecimalPoint );

          if ( foundFirstGeometry )
          {
            mExtent.combineWith( pt.x(), pt.y(), pt.z() );
          }
          else
          {
            // Extent for the first point is just the first point
            mExtent = QgsBox3D( pt.x(), pt.y(), pt.z(), pt.x(), pt.y(), pt.z() );
            mWkbType = Qgis::WkbType::Point;
            if ( mZFieldIndex > -1 )
              mWkbType = QgsWkbTypes::addZ( mWkbType );
            if ( mMFieldIndex > -1 )
              mWkbType = QgsWkbTypes::addM( mWkbType );
            mGeometryType = Qgis::GeometryType::Point;
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

      if ( !QgsWkbTypes::hasZ( mWkbType ) )
      {
        mExtent.setZMinimum( std::numeric_limits<double>::quiet_NaN() );
        mExtent.setZMinimum( std::numeric_limits<double>::quiet_NaN() );
      }
    }
    else
    {
      mWkbType = Qgis::WkbType::NoGeometry;
      mNumberFeatures++;
    }

    elevationProperties()->setContainsElevationData( QgsWkbTypes::hasZ( mWkbType ) );

    // Progress changed every 100 features
    if ( feedback && mNumberFeatures % 100 == 0 )
    {
      feedback->setProcessedCount( mNumberFeatures );
    }

    if ( !geomValid )
      continue;

    if ( buildSubsetIndex )
      mSubsetIndex.append( mFile->recordId() );


    // If we are going to use this record, then assess the potential types of each column
    const int partsSize = parts.size();

    if ( fieldTypeInformation.size() < partsSize )
    {
      fieldTypeInformation.resize( partsSize );
    }

    FieldTypeInformation *typeInformation = fieldTypeInformation.data();

    for ( int i = 0; i < partsSize; i++, typeInformation++ )
    {
      QString value = parts.value( i );
      // Ignore empty fields - spreadsheet generated CSV files often
      // have random empty fields at the end of a row
      if ( value.isEmpty() )
        continue;

      // If this column has been empty so far then initialize it
      // for possible types

      if ( typeInformation->isEmpty )
      {
        typeInformation->isEmpty = false;
        typeInformation->couldBeInt = true;
        typeInformation->couldBeLongLong = true;
        typeInformation->couldBeDouble = true;
        typeInformation->couldBeDateTime = true;
        typeInformation->couldBeDate = true;
        typeInformation->couldBeTime = true;
        typeInformation->couldBeBool = true;
      }

      if ( !mDetectTypes )
      {
        continue;
      }

      // Now test for still valid possible types for the field
      // Types are possible until first record which cannot be parsed

      if ( typeInformation->couldBeBool )
      {
        typeInformation->couldBeBool = false;
        if ( !boolCandidates.contains( i ) )
        {
          boolCandidates[i] = QPair<QString, QString>();
        }
        if ( !boolCandidates[i].first.isEmpty() )
        {
          typeInformation->couldBeBool = value.compare( boolCandidates[i].first, Qt::CaseSensitivity::CaseInsensitive ) == 0 || value.compare( boolCandidates[i].second, Qt::CaseSensitivity::CaseInsensitive ) == 0;
        }
        else
        {
          for ( const auto &bc : std::as_const( boolLiterals ) )
          {
            if ( value.compare( bc.first, Qt::CaseSensitivity::CaseInsensitive ) == 0 || value.compare( bc.second, Qt::CaseSensitivity::CaseInsensitive ) == 0 )
            {
              boolCandidates[i] = bc;
              typeInformation->couldBeBool = true;
              break;
            }
          }
        }
      }

      if ( typeInformation->couldBeInt )
      {
        ( void ) value.toInt( &typeInformation->couldBeInt );
      }

      if ( typeInformation->couldBeLongLong && !typeInformation->couldBeInt )
      {
        ( void ) value.toLongLong( &typeInformation->couldBeLongLong );
      }

      if ( typeInformation->couldBeDouble && !typeInformation->couldBeLongLong )
      {
        if ( !mDecimalPoint.isEmpty() )
        {
          value.replace( mDecimalPoint, "."_L1 );
        }
        ( void ) value.toDouble( &typeInformation->couldBeDouble );
      }

      if ( typeInformation->couldBeDateTime )
      {
        QDateTime dt;
        if ( value.length() > 10 )
        {
          dt = QDateTime::fromString( value, Qt::ISODate );
        }
        typeInformation->couldBeDateTime = ( dt.isValid() );
      }

      if ( typeInformation->couldBeDate && !typeInformation->couldBeDateTime )
      {
        const QDate d = QDate::fromString( value, Qt::ISODate );
        typeInformation->couldBeDate = d.isValid();
      }

      if ( typeInformation->couldBeTime && !typeInformation->couldBeDateTime )
      {
        const QTime t = QTime::fromString( value );
        typeInformation->couldBeTime = t.isValid();
      }
    }

    // In case of fast scan we exit after the third record (to avoid detecting booleans)
    if ( !forceFullScan && mReadFlags.testFlag( Qgis::DataProviderReadFlag::SkipFullScan ) && mNumberFeatures > 2 )
    {
      break;
    }
  }

  // Final progress changed
  if ( feedback )
  {
    feedback->setProgress( mNumberFeatures );
  }

  // Now create the attribute fields.  Field types are determined by prioritizing
  // integer, failing that double, datetime, date, time, and finally text.
  QStringList fieldNames = mFile->fieldNames();
  mFieldCount = fieldNames.size();
  attributeColumns.clear();
  attributeFields.clear();

  QString csvtMessage;
  QgsDebugMsgLevel( u"Reading CSVT: %1"_s.arg( mFile->fileName() ), 2 );
  QStringList csvtTypes = readCsvtFieldTypes( mFile->fileName(), &csvtMessage );
  int fieldIdxOffset { 0 };

  for ( int fieldIdx = 0; fieldIdx < fieldNames.size(); fieldIdx++ )
  {
    // Skip over WKT field ... don't want to display in attribute table
    if ( fieldIdx == mWktFieldIndex )
    {
      fieldIdxOffset++;
      continue;
    }

    // Add the field index lookup for the column
    attributeColumns.append( fieldIdx );
    QMetaType::Type fieldType = QMetaType::Type::QString;
    QString typeName = u"text"_s;

    // User-defined types take precedence over all
    if ( !mUserDefinedFieldTypes.value( fieldNames[fieldIdx] ).isEmpty() )
    {
      typeName = mUserDefinedFieldTypes.value( fieldNames[fieldIdx] );
    }
    else
    {
      if ( fieldIdx < csvtTypes.size() )
      {
        typeName = csvtTypes[fieldIdx];
        // Map CSVT types to provider types
        if ( typeName.startsWith( "coord"_L1 ) || typeName.startsWith( "point("_L1 ) )
        {
          typeName = u"double"_s;
        }
        else if ( typeName == "long"_L1 || typeName == "integer64"_L1 )
        {
          typeName = u"longlong"_s;
        }
        else if ( typeName == "int8"_L1 )
        {
          typeName = u"integer"_s;
        }
        else if ( typeName == "real"_L1 )
        {
          typeName = u"double"_s;
        }
      }
      else if ( mDetectTypes && fieldIdx < fieldTypeInformation.size() )
      {
        const FieldTypeInformation &typeInformation = fieldTypeInformation[fieldIdx];
        if ( typeInformation.couldBeBool )
        {
          typeName = u"bool"_s;
        }
        else if ( typeInformation.couldBeInt )
        {
          typeName = u"integer"_s;
        }
        else if ( typeInformation.couldBeLongLong )
        {
          typeName = u"longlong"_s;
        }
        else if ( typeInformation.couldBeDouble )
        {
          typeName = u"double"_s;
        }
        else if ( typeInformation.couldBeDateTime )
        {
          typeName = u"datetime"_s;
        }
        else if ( typeInformation.couldBeDate )
        {
          typeName = u"date"_s;
        }
        else if ( typeInformation.couldBeTime )
        {
          typeName = u"time"_s;
        }
      }
    }

    if ( typeName == "bool"_L1 )
    {
      fieldType = QMetaType::Type::Bool;
      mFieldBooleanLiterals.insert( fieldIdx - fieldIdxOffset, boolCandidates[fieldIdx] );
    }
    else if ( typeName == "integer"_L1 )
    {
      fieldType = QMetaType::Type::Int;
    }
    else if ( typeName == "longlong"_L1 )
    {
      fieldType = QMetaType::Type::LongLong;
    }
    else if ( typeName == "double"_L1 )
    {
      fieldType = QMetaType::Type::Double;
    }
    else if ( typeName == "datetime"_L1 )
    {
      fieldType = QMetaType::Type::QDateTime;
    }
    else if ( typeName == "date"_L1 )
    {
      fieldType = QMetaType::Type::QDate;
    }
    else if ( typeName == "time"_L1 )
    {
      fieldType = QMetaType::Type::QTime;
    }
    else
    {
      typeName = u"text"_s;
    }

    attributeFields.append( QgsField( fieldNames[fieldIdx], fieldType, typeName ) );
  }

  QgsDebugMsgLevel( "Field count for the delimited text file is " + QString::number( attributeFields.size() ), 2 );
  QgsDebugMsgLevel( "geometry type is: " + QString::number( static_cast<quint32>( mWkbType ) ), 2 );
  QgsDebugMsgLevel( "feature count is: " + QString::number( mNumberFeatures ), 2 );

  QStringList warnings;
  if ( !csvtMessage.isEmpty() )
    warnings.append( csvtMessage );
  if ( nBadFormatRecords > 0 )
    warnings.append( tr( "%n record(s) discarded due to invalid format", nullptr, nBadFormatRecords ) );
  if ( nEmptyGeometry > 0 )
    warnings.append( tr( "%n record(s) have missing geometry definitions", nullptr, nEmptyGeometry ) );
  if ( nInvalidGeometry > 0 )
    warnings.append( tr( "%n record(s) discarded due to invalid geometry definitions", nullptr, nInvalidGeometry ) );
  if ( nIncompatibleGeometry > 0 )
    warnings.append( tr( "%n record(s) discarded due to incompatible geometry types", nullptr, nIncompatibleGeometry ) );

  reportErrors( warnings );

  // Decide whether to use subset ids to index records rather than simple iteration through all
  // If more than 10% of records are being skipped, then use index.  (Not based on any experimentation,
  // could do with some analysis?)

  if ( buildSubsetIndex )
  {
    long recordCount = mFile->recordCount();
    recordCount -= recordCount / SUBSET_ID_THRESHOLD_FACTOR;
    mUseSubsetIndex = mSubsetIndex.size() < recordCount;
    if ( !mUseSubsetIndex )
      mSubsetIndex = QList<quintptr>();
  }

  mUseSpatialIndex = buildSpatialIndex;

  mValid = mGeometryType != Qgis::GeometryType::Unknown;
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

  const bool buildSpatialIndex = nullptr != mSpatialIndex;
  const bool buildSubsetIndex = mBuildSubsetIndex && ( mSubsetExpression || mGeomRep != GeomNone );

  // In case file has been rewritten check that it is still valid

  mValid = mLayerValid && mFile->isValid();
  if ( !mValid )
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
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( u"Wkt"_s, mWktFieldName ) );
    }
  }
  else if ( mGeomRep == GeomAsXy )
  {
    mXFieldIndex = mFile->fieldIndex( mXFieldName );
    mYFieldIndex = mFile->fieldIndex( mYFieldName );
    if ( mXFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( u"X"_s, mWktFieldName ) );
    }
    if ( mYFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( u"Y"_s, mWktFieldName ) );
    }
  }
  if ( !messages.isEmpty() )
  {
    reportErrors( messages );
    QgsDebugMsgLevel( u"Delimited text source invalid on rescan - missing geometry fields"_s, 2 );
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
  mExtent = QgsBox3D();
  QgsFeature f;
  bool foundFirstGeometry = false;
  while ( fi.nextFeature( f ) )
  {
    if ( mGeometryType != Qgis::GeometryType::Null && f.hasGeometry() )
    {
      if ( !foundFirstGeometry )
      {
        mExtent = f.geometry().boundingBox3D();
        foundFirstGeometry = true;
      }
      else
      {
        const QgsBox3D bbox( f.geometry().boundingBox3D() );
        mExtent.combineWith( bbox );
      }
      if ( buildSpatialIndex )
        mSpatialIndex->addFeature( f );
    }
    if ( buildSubsetIndex )
      mSubsetIndex.append( ( quintptr ) f.id() );
    mNumberFeatures++;
  }

  if ( !QgsWkbTypes::hasZ( mWkbType ) )
  {
    mExtent.setZMinimum( std::numeric_limits<double>::quiet_NaN() );
    mExtent.setZMinimum( std::numeric_limits<double>::quiet_NaN() );
  }

  if ( buildSubsetIndex )
  {
    long recordCount = mFile->recordCount();
    recordCount -= recordCount / SUBSET_ID_THRESHOLD_FACTOR;
    mUseSubsetIndex = recordCount < mSubsetIndex.size();
    if ( !mUseSubsetIndex )
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

void QgsDelimitedTextProvider::appendZM( QString &sZ, QString &sM, QgsPoint &point, const QString &decimalPoint )
{
  if ( !decimalPoint.isEmpty() )
  {
    sZ.replace( decimalPoint, "."_L1 );
    sM.replace( decimalPoint, "."_L1 );
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

QList<QPair<QString, QString>> QgsDelimitedTextProvider::booleanLiterals() const
{
  QList<QPair<QString, QString>> booleans {
    { u"true"_s, u"false"_s },
    { u"t"_s, u"f"_s },
    { u"yes"_s, u"no"_s },
    { u"1"_s, u"0"_s },
  };
  if ( !mUserDefinedBooleanLiterals.first.isEmpty() )
  {
    booleans.append( mUserDefinedBooleanLiterals );
  }
  return booleans;
}

bool QgsDelimitedTextProvider::pointFromXY( QString &sX, QString &sY, QgsPoint &pt, const QString &decimalPoint, bool xyDms )
{
  if ( !decimalPoint.isEmpty() )
  {
    sX.replace( decimalPoint, "."_L1 );
    sY.replace( decimalPoint, "."_L1 );
  }

  bool xOk, yOk;
  double x, y;
  if ( xyDms )
  {
    x = QgsCoordinateUtils::dmsToDecimal( sX, &xOk );
    y = QgsCoordinateUtils::dmsToDecimal( sY, &yOk );
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
  return u"Delimited text file"_s;
}

QgsFeatureIterator QgsDelimitedTextProvider::getFeatures( const QgsFeatureRequest &request ) const
{
  // If the file has become invalid, rescan to check that it is still invalid.
  //
  if ( ( mLayerValid && !mValid ) || mRescanRequired )
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
    if ( !s.isEmpty() )
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
  if ( !mInvalidLines.isEmpty() || !messages.isEmpty() )
  {
    const QString tag( u"DelimitedText"_s );
    QgsMessageLog::logMessage( tr( "Errors in file %1" ).arg( mFile->fileName() ), tag );
    const auto constMessages = messages;
    for ( const QString &message : constMessages )
    {
      QgsMessageLog::logMessage( message, tag );
    }
    if ( !mInvalidLines.isEmpty() )
    {
      QgsMessageLog::logMessage( tr( "The following lines were not loaded into QGIS due to errors:" ), tag );
      for ( int i = 0; i < mInvalidLines.size(); ++i )
        QgsMessageLog::logMessage( mInvalidLines.at( i ), tag );
      if ( mNExtraInvalidLines > 0 )
        QgsMessageLog::logMessage( tr( "There are %n additional error(s) in the file", nullptr, mNExtraInvalidLines ), tag );
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
      if ( !mInvalidLines.isEmpty() )
      {
        output->appendMessage( tr( "The following lines were not loaded into QGIS due to errors:" ) );
        for ( int i = 0; i < mInvalidLines.size(); ++i )
          output->appendMessage( mInvalidLines.at( i ) );
        if ( mNExtraInvalidLines > 0 )
          output->appendMessage( tr( "There are %n additional error(s) in the file", nullptr, mNExtraInvalidLines ) );
      }
      output->showMessage();
    }

    // We no longer need these lines.
    clearInvalidLines();
  }
}

bool QgsDelimitedTextProvider::setSubsetString( const QString &subset, bool updateFeatureCount )
{
  const QString nonNullSubset = subset.isNull() ? QString() : subset;

  // If not changing string, then all OK, nothing to do
  if ( nonNullSubset == mSubsetString )
    return true;

  bool valid = true;

  // If there is a new subset string then encode it..

  std::unique_ptr<QgsExpression> expression;
  if ( !nonNullSubset.isEmpty() )
  {
    expression = std::make_unique<QgsExpression>( nonNullSubset );
    QString error;
    if ( expression->hasParserError() )
    {
      error = expression->parserErrorString();
    }
    else
    {
      const QgsExpressionContext context = QgsExpressionContextUtils::createFeatureBasedContext( QgsFeature(), fields() );
      expression->prepare( &context );
      if ( expression->hasEvalError() )
      {
        error = expression->evalErrorString();
      }
    }
    if ( !error.isEmpty() )
    {
      valid = false;
      expression.reset();
      const QString tag( u"DelimitedText"_s );
      QgsMessageLog::logMessage( tr( "Invalid subset string %1 for %2" ).arg( nonNullSubset, mFile->fileName() ), tag );
    }
  }

  // if the expression is valid, then reset the subset string and data source Uri

  if ( valid )
  {
    const QString previousSubset = mSubsetString;
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
      if ( !mCachedSubsetString.isNull() && mSubsetString == mCachedSubsetString )
      {
        QgsDebugMsgLevel( u"DelimitedText: Resetting cached subset string %1"_s.arg( mSubsetString ), 3 );
        mUseSpatialIndex = mCachedUseSpatialIndex;
        mUseSubsetIndex = mCachedUseSubsetIndex;
        resetCachedSubset();
      }
      else
      {
        QgsDebugMsgLevel( u"DelimitedText: Setting new subset string %1"_s.arg( mSubsetString ), 3 );
        // Reset the subset index
        rescanFile();
        // Encode the subset string into the data source URI.
        setUriParameter( u"subset"_s, nonNullSubset );
      }
    }
    else
    {
      // If not already using temporary subset, then cache the current subset
      QgsDebugMsgLevel( u"DelimitedText: Setting temporary subset string %1"_s.arg( mSubsetString ), 3 );
      if ( mCachedSubsetString.isNull() )
      {
        QgsDebugMsgLevel( u"DelimitedText: Caching previous subset %1"_s.arg( previousSubset ), 3 );
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

bool QgsDelimitedTextProvider::supportsSubsetString() const
{
  return true;
}

QString QgsDelimitedTextProvider::subsetStringDialect() const
{
  return tr( "QGIS expression" );
}

QString QgsDelimitedTextProvider::subsetStringHelpUrl() const
{
  // unfortunately we can't access QgsHelp here, that's a GUI class!
  return QString();
}

void QgsDelimitedTextProvider::setUriParameter( const QString &parameter, const QString &value )
{
  QUrl url = QUrl::fromEncoded( dataSourceUri().toUtf8() );
  QUrlQuery query( url );
  if ( query.hasQueryItem( parameter ) )
    query.removeAllQueryItems( parameter );
  if ( !value.isEmpty() )
    query.addQueryItem( parameter, value );
  url.setQuery( query );
  setDataSourceUri( QString::fromUtf8( url.toEncoded() ) );
}

void QgsDelimitedTextProvider::onFileUpdated()
{
  if ( !mRescanRequired )
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
  return mExtent.toRectangle();
}

QgsBox3D QgsDelimitedTextProvider::extent3D() const
{
  if ( mRescanRequired )
    rescanFile();
  return mExtent;
}

Qgis::WkbType QgsDelimitedTextProvider::wkbType() const
{
  return mWkbType;
}

long long QgsDelimitedTextProvider::featureCount() const
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

Qgis::VectorProviderCapabilities QgsDelimitedTextProvider::capabilities() const
{
  return Qgis::VectorProviderCapability::SelectAtId | Qgis::VectorProviderCapability::CreateSpatialIndex | Qgis::VectorProviderCapability::CircularGeometries;
}

QgsCoordinateReferenceSystem QgsDelimitedTextProvider::crs() const
{
  return mCrs;
}

QString QgsDelimitedTextProvider::name() const
{
  return TEXT_PROVIDER_KEY;
}

QString QgsDelimitedTextProvider::providerKey()
{
  return TEXT_PROVIDER_KEY;
}

QString QgsDelimitedTextProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
}

QVariantMap QgsDelimitedTextProviderMetadata::decodeUri( const QString &uri ) const
{
  const QUrl url = QUrl::fromEncoded( uri.toUtf8() );
  const QUrlQuery queryItems( url.query() );

  QString subset;
  QStringList openOptions;
  for ( const auto &item : queryItems.queryItems() )
  {
    if ( item.first == "subset"_L1 )
    {
      subset = item.second;
    }
    else
    {
      openOptions << u"%1=%2"_s.arg( item.first, item.second );
    }
  }

  QVariantMap components;
  components.insert( u"path"_s, url.toLocalFile() );
  if ( !subset.isEmpty() )
    components.insert( u"subset"_s, subset );
  components.insert( u"openOptions"_s, openOptions );
  return components;
}

QString QgsDelimitedTextProviderMetadata::encodeUri( const QVariantMap &parts ) const
{
  QUrl url = QUrl::fromLocalFile( parts.value( u"path"_s ).toString() );
  const QStringList openOptions = parts.value( u"openOptions"_s ).toStringList();

  QUrlQuery queryItems;
  for ( const auto &option : openOptions )
  {
    const int separator = option.indexOf( '=' );
    if ( separator >= 0 )
    {
      queryItems.addQueryItem( option.mid( 0, separator ), option.mid( separator + 1 ) );
    }
    else
    {
      queryItems.addQueryItem( option, QString() );
    }
  }
  if ( parts.contains( u"subset"_s ) )
    queryItems.addQueryItem( u"subset"_s, parts.value( u"subset"_s ).toString() );
  url.setQuery( queryItems );

  return QString::fromUtf8( url.toEncoded() );
}

QString QgsDelimitedTextProviderMetadata::absoluteToRelativeUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QUrl urlSource = QUrl::fromEncoded( uri.toUtf8() );
  QUrl urlDest = QUrl::fromLocalFile( context.pathResolver().writePath( urlSource.toLocalFile() ) );
  urlDest.setQuery( urlSource.query() );
  return QString::fromUtf8( urlDest.toEncoded() );
}

QString QgsDelimitedTextProviderMetadata::relativeToAbsoluteUri( const QString &uri, const QgsReadWriteContext &context ) const
{
  QUrl urlSource = QUrl::fromEncoded( uri.toUtf8() );

  if ( !uri.startsWith( "file:"_L1 ) )
  {
    QUrl file = QUrl::fromLocalFile( uri.left( uri.indexOf( '?' ) ) );
    urlSource.setScheme( u"file"_s );
    urlSource.setPath( file.path() );
  }

  QUrl urlDest = QUrl::fromLocalFile( context.pathResolver().readPath( urlSource.toLocalFile() ) );
  urlDest.setQuery( urlSource.query() );
  return QString::fromUtf8( urlDest.toEncoded() );
}

QgsProviderMetadata::ProviderCapabilities QgsDelimitedTextProviderMetadata::providerCapabilities() const
{
  return FileBasedUris;
}

QList<Qgis::LayerType> QgsDelimitedTextProviderMetadata::supportedLayerTypes() const
{
  return { Qgis::LayerType::Vector };
}

QgsDataProvider *QgsDelimitedTextProviderMetadata::createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, Qgis::DataProviderReadFlags flags )
{
  return new QgsDelimitedTextProvider( uri, options, flags );
}


QgsDelimitedTextProviderMetadata::QgsDelimitedTextProviderMetadata()
  : QgsProviderMetadata( QgsDelimitedTextProvider::TEXT_PROVIDER_KEY, QgsDelimitedTextProvider::TEXT_PROVIDER_DESCRIPTION )
{
}

QIcon QgsDelimitedTextProviderMetadata::icon() const
{
  return QgsApplication::getThemeIcon( u"mIconDelimitedText.svg"_s );
}

#ifndef HAVE_STATIC_PROVIDERS
QGISEXTERN QgsProviderMetadata *providerMetadataFactory()
{
  return new QgsDelimitedTextProviderMetadata();
}
#endif
