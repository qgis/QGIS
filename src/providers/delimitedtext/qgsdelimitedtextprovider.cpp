/***************************************************************************
  qgsdelimitedtextprovider.cpp -  Data provider for delimted text
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
#include <QDataStream>
#include <QTextStream>
#include <QStringList>
#include <QMessageBox>
#include <QSettings>
#include <QRegExp>
#include <QUrl>

#include "qgsapplication.h"
#include "qgsdataprovider.h"
#include "qgsexpression.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsmessageoutput.h"
#include "qgsrectangle.h"
#include "qgsspatialindex.h"
#include "qgis.h"

#include "qgsdelimitedtextsourceselect.h"
#include "qgsdelimitedtextfeatureiterator.h"
#include "qgsdelimitedtextfile.h"

static const QString TEXT_PROVIDER_KEY = "delimitedtext";
static const QString TEXT_PROVIDER_DESCRIPTION = "Delimited text data provider";

// If more than this fraction of records are not in a subset then use an index to
// iterate over records rather than simple iterator and filter.

static const int SUBSET_ID_THRESHOLD_FACTOR = 10;

QRegExp QgsDelimitedTextProvider::WktPrefixRegexp( "^\\s*(?:\\d+\\s+|SRID\\=\\d+\\;)", Qt::CaseInsensitive );
QRegExp QgsDelimitedTextProvider::WktZMRegexp( "\\s*(?:z|m|zm)(?=\\s*\\()", Qt::CaseInsensitive );
QRegExp QgsDelimitedTextProvider::WktCrdRegexp( "(\\-?\\d+(?:\\.\\d*)?\\s+\\-?\\d+(?:\\.\\d*)?)\\s[\\s\\d\\.\\-]+" );
QRegExp QgsDelimitedTextProvider::CrdDmsRegexp( "^\\s*(?:([-+nsew])\\s*)?(\\d{1,3})(?:[^0-9.]+([0-5]?\\d))?[^0-9.]+([0-5]?\\d(?:\\.\\d+)?)[^0-9.]*([-+nsew])?\\s*$", Qt::CaseInsensitive );

QgsDelimitedTextProvider::QgsDelimitedTextProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , mFile( 0 )
    , mGeomRep( GeomNone )
    , mFieldCount( 0 )
    , mXFieldIndex( -1 )
    , mYFieldIndex( -1 )
    , mWktFieldIndex( -1 )
    , mWktHasZM( false )
    , mWktHasPrefix( false )
    , mXyDms( false )
    , mSubsetString( "" )
    , mSubsetExpression( 0 )
    , mBuildSubsetIndex( true )
    , mUseSubsetIndex( false )
    , mMaxInvalidLines( 50 )
    , mShowInvalidLines( true )
    , mCrs()
    , mWkbType( QGis::WKBNoGeometry )
    , mGeometryType( QGis::UnknownGeometry )
    , mBuildSpatialIndex( false )
    , mSpatialIndex( 0 )
    , mActiveIterator( 0 )
{

  QgsDebugMsg( "Delimited text file uri is " + uri );

  QUrl url = QUrl::fromEncoded( uri.toAscii() );
  mFile = new QgsDelimitedTextFile();
  mFile->setFromUrl( url );

  QString subset;

  if ( url.hasQueryItem( "geomType" ) )
  {
    QString gtype = url.queryItemValue( "geomType" ).toLower();
    if ( gtype == "point" ) mGeometryType = QGis::Point;
    else if ( gtype == "line" ) mGeometryType = QGis::Line;
    else if ( gtype == "polygon" ) mGeometryType = QGis::Polygon;
    else if ( gtype == "none " ) mGeometryType = QGis::NoGeometry;
  }

  if ( mGeometryType != QGis::NoGeometry )
  {
    if ( url.hasQueryItem( "wktField" ) )
    {
      mWktFieldName = url.queryItemValue( "wktField" );
      mGeomRep = GeomAsWkt;
      QgsDebugMsg( "wktField is: " + mWktFieldName );
    }
    else if ( url.hasQueryItem( "xField" ) && url.hasQueryItem( "yField" ) )
    {
      mGeomRep = GeomAsXy;
      mGeometryType = QGis::Point;
      mXFieldName = url.queryItemValue( "xField" );
      mYFieldName = url.queryItemValue( "yField" );
      QgsDebugMsg( "xField is: " + mXFieldName );
      QgsDebugMsg( "yField is: " + mYFieldName );

      if ( url.hasQueryItem( "xyDms" ) )
      {
        mXyDms = ! url.queryItemValue( "xyDms" ).toLower().startsWith( "n" );
      }
    }
    else
    {
      mGeometryType = QGis::NoGeometry;
    }
  }

  if ( url.hasQueryItem( "decimalPoint" ) )
    mDecimalPoint = url.queryItemValue( "decimalPoint" );

  if ( url.hasQueryItem( "crs" ) )
    mCrs.createFromString( url.queryItemValue( "crs" ) );

  if ( url.hasQueryItem( "subsetIndex" ) )
  {
    mBuildSubsetIndex = ! url.queryItemValue( "subsetIndex" ).toLower().startsWith( "n" );
  }

  if ( url.hasQueryItem( "spatialIndex" ) )
  {
    mBuildSpatialIndex = ! url.queryItemValue( "spatialIndex" ).toLower().startsWith( "n" );
  }

  if ( url.hasQueryItem( "subset" ) )
  {
    subset = url.queryItemValue( "subset" );
    QgsDebugMsg( "subset is: " + subset );
  }

  if ( url.hasQueryItem( "quiet" ) ) mShowInvalidLines = false;

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

void QgsDelimitedTextProvider::resetCachedSubset()
{
  mCachedSubsetString = QString();
  mCachedUseSubsetIndex = false;
  mCachedUseSpatialIndex = false;
}


void QgsDelimitedTextProvider::resetIndexes()
{
  resetCachedSubset();
  mUseSubsetIndex = false;
  mUseSpatialIndex = false;

  mSubsetIndex.clear();
  if ( mSpatialIndex ) delete mSpatialIndex;
  mSpatialIndex = 0;
  if ( mBuildSpatialIndex && mGeomRep != GeomNone ) mSpatialIndex = new QgsSpatialIndex();
}

bool QgsDelimitedTextProvider::createSpatialIndex()
{
  if ( mBuildSpatialIndex ) return true; // Already built
  if ( mGeomRep == GeomNone ) return false; // Cannot build index - no geometries

  // Ok, set the spatial index option, set the Uri parameter so that the index is
  // rebuilt when theproject is reloaded, and rescan the file to populate the index

  mBuildSpatialIndex = true;
  setUriParameter( "spatialIndex", "yes" );
  rescanFile();
  return true;
}

// buildIndexes parameter of scanFile is to allow for potential rescan - if using
// subset string then rescan follows this to determine subset extents etc.
// Done this way as subset requires fields to be defined, which they are not
// until initial file scan is complete.
//
// Although at this point the subset expression does not apply (if one is defined)
// we still consider a subset index, as this also applies for implicit subsets
// due to filtering on geometry validity.

void QgsDelimitedTextProvider::scanFile( bool buildIndexes )
{
  QStringList messages;

  // assume the layer is invalid until proven otherwise

  mValid = false;

  clearInvalidLines();

  // Initiallize indexes

  resetIndexes();
  bool buildSpatialIndex = buildIndexes && mSpatialIndex != 0;

  // No point building a subset index if there is no geometry, as all
  // records will be included.

  bool buildSubsetIndex = buildIndexes && mBuildSubsetIndex && mGeomRep != GeomNone;

  if ( ! mFile->isValid() )
  {
    // uri is invalid so the layer must be too...

    messages.append( tr( "File cannot be opened or delimiter parameters are not valid" ) );
    reportErrors( messages );
    QgsDebugMsg( "Delimited text source invalid - filename or delimiter parameters" );
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
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( "Wkt" ).arg( mWktFieldName ) );
    }
  }
  else if ( mGeomRep == GeomAsXy )
  {
    mXFieldIndex = mFile->fieldIndex( mXFieldName );
    mYFieldIndex = mFile->fieldIndex( mYFieldName );
    if ( mXFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( "X" ).arg( mWktFieldName ) );
    }
    if ( mYFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( "Y" ).arg( mWktFieldName ) );
    }
  }
  if ( messages.size() > 0 )
  {
    reportErrors( messages );
    QgsDebugMsg( "Delimited text source invalid - missing geometry fields" );
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
  QList<bool> couldBeDouble;

  while ( true )
  {
    QgsDelimitedTextFile::Status status = mFile->nextRecord( parts );
    if ( status == QgsDelimitedTextFile::RecordEOF ) break;
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
        geomValid = false;
      }
      else
      {
        // Get the wkt - confirm it is valid, get the type, and
        // if compatible with the rest of file, add to the extents

        QString sWkt = parts[mWktFieldIndex];
        QgsGeometry *geom = 0;
        if ( !mWktHasPrefix && sWkt.indexOf( WktPrefixRegexp ) >= 0 )
          mWktHasPrefix = true;
        if ( !mWktHasZM && sWkt.indexOf( WktZMRegexp ) >= 0 )
          mWktHasZM = true;
        geom = geomFromWkt( sWkt );

        if ( geom )
        {
          QGis::WkbType type = geom->wkbType();
          if ( type != QGis::WKBNoGeometry )
          {
            if ( mGeometryType == QGis::UnknownGeometry || geom->type() == mGeometryType )
            {
              mGeometryType = geom->type();
              if ( mNumberFeatures == 0 )
              {
                mNumberFeatures++;
                mWkbType = type;
                mExtent = geom->boundingBox();
              }
              else
              {
                mNumberFeatures++;
                if ( geom->isMultipart() ) mWkbType = type;
                QgsRectangle bbox( geom->boundingBox() );
                mExtent.combineExtentWith( &bbox );
              }
              if ( buildSpatialIndex )
              {
                QgsFeature f;
                f.setFeatureId( mFile->recordId() );
                f.setGeometry( geom );
                mSpatialIndex->insertFeature( f );
                // Feature now has ownership of geometry, so set to null
                // here to avoid deleting twice.
                geom = 0;
              }
            }
            else
            {
              nIncompatibleGeometry++;
              geomValid = false;
            }
          }
          if ( geom ) delete geom;
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

      QString sX = mXFieldIndex < parts.size() ? parts[mXFieldIndex] : "";
      QString sY = mYFieldIndex < parts.size() ? parts[mYFieldIndex] : "";
      if ( sX.isEmpty() && sY.isEmpty() )
      {
        geomValid = false;
        nEmptyGeometry++;
      }
      else
      {
        QgsPoint pt;
        bool ok = pointFromXY( sX, sY, pt );

        if ( ok )
        {
          if ( mNumberFeatures > 0 )
          {
            mExtent.combineExtentWith( pt.x(), pt.y() );
          }
          else
          {
            // Extent for the first point is just the first point
            mExtent.set( pt.x(), pt.y(), pt.x(), pt.y() );
            mWkbType = QGis::WKBPoint;
            mGeometryType = QGis::Point;
          }
          mNumberFeatures++;
          if ( buildSpatialIndex )
          {
            QgsFeature f;
            f.setFeatureId( mFile->recordId() );
            f.setGeometry( QgsGeometry::fromPoint( pt ) );
            mSpatialIndex->insertFeature( f );
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
      mWkbType = QGis::WKBNoGeometry;
      mNumberFeatures++;
    }

    if ( ! geomValid ) continue;

    if ( buildSubsetIndex ) mSubsetIndex.append( mFile->recordId() );


    // If we are going to use this record, then assess the potential types of each colum

    for ( int i = 0; i < parts.size(); i++ )
    {

      QString &value = parts[i];
      if ( value.isEmpty() )
        continue;

      // try to convert attribute values to integer and double

      while ( couldBeInt.size() <= i )
      {
        isEmpty.append( true );
        couldBeInt.append( false );
        couldBeDouble.append( false );
      }
      if ( isEmpty[i] )
      {
        isEmpty[i] = false;
        couldBeInt[i] = true;
        couldBeDouble[i] = true;
      }
      if ( couldBeInt[i] )
      {
        value.toInt( &couldBeInt[i] );
      }
      if ( couldBeDouble[i] )
      {
        if ( ! mDecimalPoint.isEmpty() )
        {
          value.replace( mDecimalPoint, "." );
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
  for ( int i = 0; i < fieldNames.size(); i++ )
  {
    // Skip over WKT field ... don't want to display in attribute table
    if ( i == mWktFieldIndex ) continue;

    // Add the field index lookup for the column
    attributeColumns.append( i );
    QVariant::Type fieldType = QVariant::String;
    QString typeName = "Text";
    if ( i < couldBeInt.size() )
    {
      if ( couldBeInt[i] )
      {
        fieldType = QVariant::Int;
        typeName = "integer";
      }
      else if ( couldBeDouble[i] )
      {
        fieldType = QVariant::Double;
        typeName = "double";
      }
    }
    attributeFields.append( QgsField( fieldNames[i], fieldType, typeName ) );
  }


  QgsDebugMsg( "Field count for the delimited text file is " + QString::number( attributeFields.size() ) );
  QgsDebugMsg( "geometry type is: " + QString::number( mWkbType ) );
  QgsDebugMsg( "feature count is: " + QString::number( mNumberFeatures ) );

  QStringList warnings;
  if ( nBadFormatRecords > 0 )
    warnings.append( tr( "%1 records discarded due to invalid format" ).arg( nBadFormatRecords ) );
  if ( nEmptyGeometry > 0 )
    warnings.append( tr( "%1 records discarded due to missing geometry definitions" ).arg( nEmptyGeometry ) );
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
    if ( ! mUseSubsetIndex ) mSubsetIndex = QList<quintptr>();
  }

  mUseSpatialIndex = buildSpatialIndex;

  mValid = mGeometryType != QGis::UnknownGeometry;

  // If it is valid, then watch for changes to the file
  connect( mFile, SIGNAL( fileUpdated() ), this, SLOT( onFileUpdated() ) );


}

QgsDelimitedTextProvider::~QgsDelimitedTextProvider()
{
  if ( mActiveIterator )
    mActiveIterator->close();

  if ( mFile )
  {
    delete mFile;
    mFile = 0;
  }

  if ( mSubsetExpression )
  {
    delete mSubsetExpression;
    mSubsetExpression = 0;
  }
  if ( mSpatialIndex )
  {
    delete mSpatialIndex;
    mSpatialIndex = 0;
  }
}

QgsGeometry *QgsDelimitedTextProvider::geomFromWkt( QString &sWkt )
{
  QgsGeometry *geom = 0;
  try
  {
    if ( mWktHasPrefix )
    {
      sWkt.remove( WktPrefixRegexp );
    }

    if ( mWktHasZM )
    {
      sWkt.remove( WktZMRegexp ).replace( WktCrdRegexp, "\\1" );
    }
    geom = QgsGeometry::fromWkt( sWkt );
  }
  catch ( ... )
  {
    geom = 0;
  }
  return geom;
}


double QgsDelimitedTextProvider::dmsStringToDouble( const QString &sX, bool *xOk )
{
  static QString negative( "swSW-" );
  QRegExp re( CrdDmsRegexp );
  double x = 0.0;

  *xOk = re.indexIn( sX ) == 0;
  if ( ! *xOk ) return 0.0;
  QString dms1 = re.capturedTexts()[2];
  QString dms2 = re.capturedTexts()[3];
  QString dms3 = re.capturedTexts()[4];
  x = dms3.toDouble( xOk );
  // Allow for Degrees/minutes format as well as DMS
  if ( ! dms2.isEmpty() )
  {
    x = dms2.toInt( xOk ) + x / 60.0;
  }
  x = dms1.toInt( xOk ) + x / 60.0;
  QString sign1 = re.capturedTexts()[1];
  QString sign2 = re.capturedTexts()[5];

  if ( sign1.isEmpty() )
  {
    if ( ! sign2.isEmpty() && negative.contains( sign2 ) ) x = -x;
  }
  else if ( sign2.isEmpty() )
  {
    if ( ! sign1.isEmpty() && negative.contains( sign1 ) ) x = -x;
  }
  else
  {
    *xOk = false;
  }
  return x;
}


bool QgsDelimitedTextProvider::pointFromXY( QString &sX, QString &sY, QgsPoint &pt )
{
  if ( ! mDecimalPoint.isEmpty() )
  {
    sX.replace( mDecimalPoint, "." );
    sY.replace( mDecimalPoint, "." );
  }

  bool xOk, yOk;
  double x, y;
  if ( mXyDms )
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
  return "Delimited text file";
}

void QgsDelimitedTextProvider::resetStream()
{
  mFile->reset();
}

QgsFeatureIterator QgsDelimitedTextProvider::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsDelimitedTextFeatureIterator( this, request ) );
}

void QgsDelimitedTextProvider::clearInvalidLines()
{
  mInvalidLines.clear();
  mNExtraInvalidLines = 0;
}

bool QgsDelimitedTextProvider::recordIsEmpty( QStringList &record )
{
  foreach ( QString s, record )
  {
    if ( ! s.isEmpty() ) return false;
  }
  return true;
}

void QgsDelimitedTextProvider::recordInvalidLine( QString message )
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

void QgsDelimitedTextProvider::reportErrors( QStringList messages , bool showDialog )
{
  if ( !mInvalidLines.isEmpty() || ! messages.isEmpty() )
  {
    QString tag( "DelimitedText" );
    QgsMessageLog::logMessage( tr( "Errors in file %1" ).arg( mFile->fileName() ), tag );
    foreach ( QString message, messages )
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
      QgsMessageOutput* output = QgsMessageOutput::createMessageOutput();
      output->setTitle( tr( "Delimited text file errors" ) );
      output->setMessage( tr( "Errors in file %1" ).arg( mFile->fileName() ), QgsMessageOutput::MessageText );
      foreach ( QString message, messages )
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

//

bool QgsDelimitedTextProvider::setSubsetString( QString subset, bool updateFeatureCount )
{
  // If not changing string, then oll ok, nothing to do

  if ( subset.isNull() ) subset = "";
  if ( subset == mSubsetString ) return true;

  bool valid = true;

  // If there is a new subset string then encode it..

  QgsExpression *expression = 0;
  if ( ! subset.isEmpty() )
  {

    expression = new QgsExpression( subset );
    QString error;
    if ( expression->hasParserError() )
    {
      error = expression->parserErrorString();
    }
    else
    {
      expression->prepare( fields() );
      if ( expression->hasEvalError() )
      {
        error = expression->evalErrorString();
      }
    }
    if ( ! error.isEmpty() )
    {
      valid = false;
      delete expression;
      expression = 0;
      QString tag( "DelimitedText" );
      QgsMessageLog::logMessage( tr( "Invalid subset string %1 for %2" ).arg( subset ).arg( mFile->fileName() ), tag );
    }
  }

  // if the expression is valid, then reset the subset string and data source Uri

  if ( valid )
  {

    if ( mSubsetExpression ) delete mSubsetExpression;
    QString previousSubset = mSubsetString;
    mSubsetString = subset;
    mSubsetExpression = expression;

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
        QgsDebugMsg( QString( "DelimitedText: Resetting cached subset string %1" ).arg( mSubsetString ) );
        mUseSpatialIndex = mCachedUseSpatialIndex;
        mUseSubsetIndex = mCachedUseSubsetIndex;
        resetCachedSubset();
      }
      else
      {
        QgsDebugMsg( QString( "DelimitedText: Setting new subset string %1" ).arg( mSubsetString ) );
        // Reset the subset index
        rescanFile();
        // Encode the subset string into the data source URI.
        setUriParameter( "subset", subset );
      }
    }
    else
    {
      // If not already using temporary subset, then cache the current subset
      QgsDebugMsg( QString( "DelimitedText: Setting temporary subset string %1" ).arg( mSubsetString ) );
      if ( mCachedSubsetString.isNull() )
      {
        QgsDebugMsg( QString( "DelimitedText: Caching previous subset %1" ).arg( previousSubset ) );
        mCachedSubsetString = previousSubset;
        mCachedUseSpatialIndex = mUseSpatialIndex;
        mCachedUseSubsetIndex = mUseSubsetIndex;
      }
      mUseSubsetIndex = false;
      mUseSpatialIndex = false;
    }
  }

  return valid;
}

void QgsDelimitedTextProvider::setUriParameter( QString parameter, QString value )
{
  QUrl url = QUrl::fromEncoded( dataSourceUri().toAscii() );
  if ( url.hasQueryItem( parameter ) ) url.removeAllQueryItems( parameter );
  if ( ! value.isEmpty() ) url.addQueryItem( parameter, value );
  setDataSourceUri( QString::fromAscii( url.toEncoded() ) );
}

// rescanFile.  Called if something has changed file definition, such as
// selecting a subset, the file has been changed by another program, etc

void QgsDelimitedTextProvider::rescanFile()
{
  resetIndexes();

  bool buildSpatialIndex = mSpatialIndex != 0;
  bool buildSubsetIndex = mBuildSubsetIndex && ( mSubsetExpression || mGeomRep != GeomNone );

  // In case file has been rewritten, check that required fields are still
  // valid

  mValid = mFile->isValid();
  if ( ! mValid ) return;

  // Open the file and get number of rows, etc. We assume that the
  // file has a header row and process accordingly. Caller should make
  // sure that the delimited file is properly formed.

  QStringList messages;

  if ( mGeomRep == GeomAsWkt )
  {
    mWktFieldIndex = mFile->fieldIndex( mWktFieldName );
    if ( mWktFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( "Wkt" ).arg( mWktFieldName ) );
    }
  }
  else if ( mGeomRep == GeomAsXy )
  {
    mXFieldIndex = mFile->fieldIndex( mXFieldName );
    mYFieldIndex = mFile->fieldIndex( mYFieldName );
    if ( mXFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( "X" ).arg( mWktFieldName ) );
    }
    if ( mYFieldIndex < 0 )
    {
      messages.append( tr( "%0 field %1 is not defined in delimited text file" ).arg( "Y" ).arg( mWktFieldName ) );
    }
  }
  if ( messages.size() > 0 )
  {
    reportErrors( messages, false );
    QgsDebugMsg( "Delimited text source invalid on rescan - missing geometry fields" );
    mValid = false;
  }

  // Reset the field columns

  for ( int i = 0; i < attributeFields.size(); i++ )
  {
    attributeColumns[i] = mFile->fieldIndex( attributeFields[i].name() );
  }

  // Scan through the features in the file

  mSubsetIndex.clear();
  mUseSubsetIndex = false;
  QgsFeatureIterator fi = getFeatures( QgsFeatureRequest() );
  mNumberFeatures = 0;
  mExtent = QgsRectangle();
  QgsFeature f;
  while ( fi.nextFeature( f ) )
  {
    if ( mGeometryType != QGis::NoGeometry )
    {
      if ( mNumberFeatures == 0 )
      {
        mExtent = f.geometry()->boundingBox();
      }
      else
      {
        QgsRectangle bbox( f.geometry()->boundingBox() );
        mExtent.combineExtentWith( &bbox );
      }
      if ( buildSpatialIndex ) mSpatialIndex->insertFeature( f );
    }
    if ( buildSubsetIndex ) mSubsetIndex.append(( quintptr ) f.id() );
    mNumberFeatures++;
  }
  if ( buildSubsetIndex )
  {
    long recordCount = mFile->recordCount();
    recordCount -= recordCount / SUBSET_ID_THRESHOLD_FACTOR;
    mUseSubsetIndex = recordCount < mSubsetIndex.size();
    if ( ! mUseSubsetIndex ) mSubsetIndex.clear();
  }

  mUseSpatialIndex = buildSpatialIndex;
}

void QgsDelimitedTextProvider::onFileUpdated()
{
  QStringList messages;
  messages.append( tr( "The file has been updated by another application - reloading" ) );
  reportErrors( messages, false );

  if ( mActiveIterator ) mActiveIterator->close();
  rescanFile();
}

bool QgsDelimitedTextProvider::nextFeature( QgsFeature& feature, QgsDelimitedTextFile *file, QgsDelimitedTextFeatureIterator *iterator )
{
  QStringList tokens;

  // If the iterator is not scanning the file, then it will have requested a specific
  // record, so only need to load that one.

  bool first = true;
  bool scanning = iterator->scanningFile();

  while ( scanning || first )
  {
    first = false;

    // before we do anything else, assume that there's something wrong with
    // the feature

    feature.setValid( false );
    QgsDelimitedTextFile::Status status = file->nextRecord( tokens );
    if ( status == QgsDelimitedTextFile::RecordEOF ) break;
    if ( status != QgsDelimitedTextFile::RecordOk ) continue;
    // We ignore empty records, such as added randomly by spreadsheets

    if ( recordIsEmpty( tokens ) ) continue;

    QgsFeatureId fid = file->recordId();

    while ( tokens.size() < mFieldCount )
      tokens.append( QString::null );

    QgsGeometry *geom = 0;

    // Load the geometry if required

    if ( iterator->loadGeometry() )
    {
      if ( mGeomRep == GeomAsWkt )
      {
        geom = loadGeometryWkt( tokens, iterator );
      }
      else if ( mGeomRep == GeomAsXy )
      {
        geom = loadGeometryXY( tokens, iterator );
      }

      if ( ! geom )
      {
        continue;
      }
    }

    // At this point the current feature values are valid

    feature.setValid( true );
    feature.setFields( &attributeFields ); // allow name-based attribute lookups
    feature.setFeatureId( fid );
    feature.initAttributes( attributeFields.count() );

    if ( geom )
      feature.setGeometry( geom );

    // If we are testing subset expression, then need all attributes just in case.
    // Could be more sophisticated, but probably not worth it!

    if ( iterator->loadSubsetOfAttributes() )
    {
      const QgsAttributeList& attrs = iterator->subsetOfAttributes();
      for ( QgsAttributeList::const_iterator i = attrs.begin(); i != attrs.end(); ++i )
      {
        int fieldIdx = *i;
        fetchAttribute( feature, fieldIdx, tokens );
      }
    }
    else
    {
      for ( int idx = 0; idx < attributeFields.count(); ++idx )
        fetchAttribute( feature, idx, tokens );
    }

    // If the iterator hasn't already filtered out the subset, then do it now

    if ( iterator->testSubset() )
    {
      QVariant isOk = mSubsetExpression->evaluate( &feature );
      if ( mSubsetExpression->hasEvalError() ) continue;
      if ( ! isOk.toBool() ) continue;
    }

    // We have a good record, so return
    return true;

  }

  return false;
}


QgsGeometry* QgsDelimitedTextProvider::loadGeometryWkt( const QStringList& tokens, QgsDelimitedTextFeatureIterator *iterator )
{
  QgsGeometry* geom = 0;
  QString sWkt = tokens[mWktFieldIndex];

  geom = geomFromWkt( sWkt );

  if ( geom && geom->type() != mGeometryType )
  {
    delete geom;
    geom = 0;
  }
  if ( geom && ! iterator->wantGeometry( geom ) )
  {
    delete geom;
    geom = 0;
  }
  return geom;
}


QgsGeometry* QgsDelimitedTextProvider::loadGeometryXY( const QStringList& tokens, QgsDelimitedTextFeatureIterator *iterator )
{
  QString sX = tokens[mXFieldIndex];
  QString sY = tokens[mYFieldIndex];
  QgsPoint pt;
  bool ok = pointFromXY( sX, sY, pt );

  if ( ok && iterator->wantGeometry( pt ) )
  {
    return QgsGeometry::fromPoint( pt );
  }
  return 0;
}


void QgsDelimitedTextProvider::fetchAttribute( QgsFeature& feature, int fieldIdx, const QStringList& tokens )
{
  if ( fieldIdx < 0 || fieldIdx >= attributeColumns.count() ) return;
  int column = attributeColumns[fieldIdx];
  if ( column < 0 || column >= tokens.count() ) return;
  const QString &value = tokens[column];
  QVariant val;
  switch ( attributeFields[fieldIdx].type() )
  {
    case QVariant::Int:
      if ( value.isEmpty() )
        val = QVariant( attributeFields[fieldIdx].type() );
      else
        val = QVariant( value );
      break;
    case QVariant::Double:
      if ( value.isEmpty() )
      {
        val = QVariant( attributeFields[fieldIdx].type() );
      }
      else if ( mDecimalPoint.isEmpty() )
      {
        val = QVariant( value.toDouble() );
      }
      else
      {
        val = QVariant( QString( value ).replace( mDecimalPoint, "." ).toDouble() );
      }
      break;
    default:
      val = QVariant( value );
      break;
  }
  feature.setAttribute( fieldIdx, val );
}

// Return the extent of the layer
QgsRectangle QgsDelimitedTextProvider::extent()
{
  return mExtent;
}

/**
 * Return the feature type
 */
QGis::WkbType QgsDelimitedTextProvider::geometryType() const
{
  return mWkbType;
}

/**
 * Return the feature type
 */
long QgsDelimitedTextProvider::featureCount() const
{
  return mNumberFeatures;
}


const QgsFields & QgsDelimitedTextProvider::fields() const
{
  return attributeFields;
}

bool QgsDelimitedTextProvider::isValid()
{
  return mValid;
}


int QgsDelimitedTextProvider::capabilities() const
{
  return SelectAtId | CreateSpatialIndex;
}


QgsCoordinateReferenceSystem QgsDelimitedTextProvider::crs()
{
  return mCrs;
}


QString  QgsDelimitedTextProvider::name() const
{
  return TEXT_PROVIDER_KEY;
} // ::name()



QString  QgsDelimitedTextProvider::description() const
{
  return TEXT_PROVIDER_DESCRIPTION;
} //  QgsDelimitedTextProvider::name()


/**
 * Class factory to return a pointer to a newly created
 * QgsDelimitedTextProvider object
 */
QGISEXTERN QgsDelimitedTextProvider *classFactory( const QString *uri )
{
  return new QgsDelimitedTextProvider( *uri );
}

/** Required key function (used to map the plugin to a data store type)
*/
QGISEXTERN QString providerKey()
{
  return TEXT_PROVIDER_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return TEXT_PROVIDER_DESCRIPTION;
}

/**
 * Required isProvider function. Used to determine if this shared library
 * is a data provider plugin
 */
QGISEXTERN bool isProvider()
{
  return true;
}

QGISEXTERN QgsDelimitedTextSourceSelect *selectWidget( QWidget *parent, Qt::WFlags fl )
{
  return new QgsDelimitedTextSourceSelect( parent, fl );
}
