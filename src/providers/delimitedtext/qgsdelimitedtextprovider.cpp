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
#include "qgis.h"

#include "qgsdelimitedtextsourceselect.h"
#include "qgsdelimitedtextfeatureiterator.h"
#include "qgsdelimitedtextfile.h"

static const QString TEXT_PROVIDER_KEY = "delimitedtext";
static const QString TEXT_PROVIDER_DESCRIPTION = "Delimited text data provider";

QRegExp QgsDelimitedTextProvider::WktPrefixRegexp( "^\\s*(?:\\d+\\s+|SRID\\=\\d+\\;)", Qt::CaseInsensitive );
QRegExp QgsDelimitedTextProvider::WktZMRegexp( "\\s*(?:z|m|zm)(?=\\s*\\()", Qt::CaseInsensitive );
QRegExp QgsDelimitedTextProvider::WktCrdRegexp( "(\\-?\\d+(?:\\.\\d*)?\\s+\\-?\\d+(?:\\.\\d*)?)\\s[\\s\\d\\.\\-]+" );
QRegExp QgsDelimitedTextProvider::CrdDmsRegexp( "^\\s*(?:([-+nsew])\\s*)?(\\d{1,3})(?:[^0-9.]+([0-5]?\\d))?[^0-9.]+([0-5]?\\d(?:\\.\\d+)?)[^0-9.]*([-+nsew])?\\s*$", Qt::CaseInsensitive );

QgsDelimitedTextProvider::QgsDelimitedTextProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , mFile( 0 )
    , mFieldCount( 0 )
    , mXFieldIndex( -1 )
    , mYFieldIndex( -1 )
    , mWktFieldIndex( -1 )
    , mWktHasZM( false )
    , mWktHasPrefix( false )
    , mXyDms( false )
    , mSubsetString("")
    , mSubsetExpression( 0 )
    , mMaxInvalidLines( 50 )
    , mShowInvalidLines( true )
    , mCrs()
    , mWkbType( QGis::WKBNoGeometry )
    , mGeometryType( QGis::UnknownGeometry )
    , mActiveIterator( 0 )
{

  QgsDebugMsg( "Delimited text file uri is " + uri );

  QUrl url = QUrl::fromEncoded( uri.toAscii() );
  mFile = new QgsDelimitedTextFile();
  mFile->setFromUrl( url );

  QString wktField;
  QString xField;
  QString yField;
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
      wktField = url.queryItemValue( "wktField" );
    }
    else if ( url.hasQueryItem( "xField" ) && url.hasQueryItem( "yField" ) )
    {
      mGeometryType = QGis::Point;
      xField = url.queryItemValue( "xField" );
      yField = url.queryItemValue( "yField" );

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


  QgsDebugMsg( "wktField is: " + wktField );
  QgsDebugMsg( "xField is: " + xField );
  QgsDebugMsg( "yField is: " + yField );

  if( url.hasQueryItem( "subset" ))
  {
    subset = url.queryItemValue("subset");
    QgsDebugMsg( "subset is: " + subset );
  }

  if ( url.hasQueryItem( "quiet" ) ) mShowInvalidLines = false;

  // assume the layer is invalid until proven otherwise
  clearInvalidLines();
  mValid = false;
  if ( ! mFile->isValid() )
  {
    // uri is invalid so the layer must be too...
    QStringList messages;
    messages.append( "File cannot be opened or delimiter parameters are not valid" );
    reportErrors( messages );
    QgsDebugMsg( "Delimited text source invalid - filename or delimiter parameters" );
    return;
  }

  // Open the file and get number of rows, etc. We assume that the
  // file has a header row and process accordingly. Caller should make
  // sure that the delimited file is properly formed.

  mWktFieldIndex = mFile->fieldIndex( wktField );
  mXFieldIndex = mFile->fieldIndex( xField );
  mYFieldIndex = mFile->fieldIndex( yField );

  if ( mWktFieldIndex >= 0 )
  {
    QgsDebugMsg( "Found wkt field: " + QString::number( mWktFieldIndex + 1 ) );
  }
  if ( mXFieldIndex >= 0 )
  {
    QgsDebugMsg( "Found X field: " + QString::number( mXFieldIndex + 1 ) );
  }
  if ( mYFieldIndex >= 0 )
  {
    QgsDebugMsg( "Found Y field: " + QString::number( mYFieldIndex + 1 ) );
  }

  // Scan the entire file to determine
  // 1) the number of fields (this is handled by QgsDelimitedTextFile mFile
  // 2) the number of valid features.  Note that the selection of valid features
  //    should match the code in QgsDelimitedTextFeatureIterator
  // 3) the geometric extents of the layer
  // 4) the type of each field

  QStringList parts;
  int nEmptyRecords = 0;
  int nBadFormatRecords = 0;
  int nIncompatibleGeometry = 0;
  int nInvalidGeometry = 0;
  int nEmptyGeometry = 0;
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

    if ( mWktFieldIndex >= 0 )
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
            }
            else
            {
              nIncompatibleGeometry++;
              geomValid = false;
            }
          }
          delete geom;
        }
        else
        {
          geomValid = false;
          nInvalidGeometry++;
          recordInvalidLine( tr( "Invalid WKT at line %1" ) );
        }
      }
    }
    else if ( mXFieldIndex >= 0 && mYFieldIndex >= 0 )
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
  mValid = mGeometryType != QGis::UnknownGeometry;

  if( ! subset.isEmpty())
  {
    setSubsetString( subset );
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


QgsDelimitedTextProvider::~QgsDelimitedTextProvider()
{
  if ( mActiveIterator )
    mActiveIterator->close();

  if ( mFile )
  {
    delete mFile;
    mFile = 0;
  }

  if( mSubsetExpression )
  {
    delete mSubsetExpression;
    mSubsetExpression = 0;
  }
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
    mInvalidLines.append( message.arg( mFile->recordLineNumber() ) );
  }
  else
  {
    mNExtraInvalidLines++;
  }
}

void QgsDelimitedTextProvider::reportErrors( QStringList messages )
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
    if ( mShowInvalidLines )
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

  bool valid = true;

  // If there is a new subset string then encode it..

  QgsExpression *expression = 0;
  if( ! subset.isEmpty() )
  {

    expression = new QgsExpression( subset );
    QString error;
    if( expression->hasParserError())
    {
      error = expression->parserErrorString();
    }
    else
    {
      expression->prepare( fields() );
      if( expression->hasEvalError() )
      {
        error = expression->evalErrorString();
      }
    }
    if( ! error.isEmpty() )
    {
      valid = false;
      delete expression;
      expression = 0;
      QString tag( "DelimitedText" );
      QgsMessageLog::logMessage( tr( "Invalid subset string %1 for %2" ).arg(subset).arg( mFile->fileName() ), tag );
    }
  }


  // if the expression is valid, then reset the subset string and data source Uri

  if( valid )
  {

    if( mSubsetExpression ) delete mSubsetExpression;
    mSubsetString = subset;
    mSubsetExpression = expression;

    // Encode the subset string into the data source URI.

    QUrl url = QUrl::fromEncoded( dataSourceUri().toAscii() );
    if( url.hasQueryItem("subset")) url.removeAllQueryItems("subset");
    if( ! subset.isEmpty()) url.addQueryItem("subset",subset);
    setDataSourceUri( QString::fromAscii( url.toEncoded() ) );

    // Update the feature count and extents if requested
    if( updateFeatureCount )
    {
      resetDataSummary();
    }
  }

  return valid;
}

void QgsDelimitedTextProvider::resetDataSummary()
{
  QgsFeatureIterator fi = getFeatures(QgsFeatureRequest());
  mNumberFeatures = 0;
  mExtent = QgsRectangle();
  QgsFeature f;
  while( fi.nextFeature(f))
  {
    if( mGeometryType != QGis::NoGeometry )
    {
      if( mNumberFeatures == 0 )
      {
        mExtent = f.geometry()->boundingBox();
      }
      else
      {
        QgsRectangle bbox( f.geometry()->boundingBox());
        mExtent.combineExtentWith( &bbox);
      }
    }
    mNumberFeatures++;
  }
}


bool QgsDelimitedTextProvider::nextFeature( QgsFeature& feature, QgsDelimitedTextFile *file, const QgsFeatureRequest& request )
{
  QStringList tokens;
  while ( true )
  {
    // before we do anything else, assume that there's something wrong with
    // the feature
    feature.setValid( false );
    QgsDelimitedTextFile::Status status = file->nextRecord( tokens );
    if ( status == QgsDelimitedTextFile::RecordEOF ) break;
    if ( status != QgsDelimitedTextFile::RecordOk ) continue;

    int fid = file->recordLineNumber();
    if( request.filterType() == QgsFeatureRequest::FilterFid && fid != request.filterFid()) continue;
    if ( recordIsEmpty( tokens ) ) continue;

    while ( tokens.size() < mFieldCount )
      tokens.append( QString::null );

    QgsGeometry *geom = 0;

    // Note: Always need to load geometry even if request has NoGeometry set
    // and subset string doesn't need geometry, as only by loading geometry
    // do we know that this is a valid record in the data set.
    if ( mWktFieldIndex >= 0 )
    {
      geom = loadGeometryWkt( tokens, request );
    }
    else if ( mXFieldIndex >= 0 && mYFieldIndex >= 0 )
    {
      geom = loadGeometryXY( tokens,request );
    }

    if ( !geom && mWkbType != QGis::WKBNoGeometry )
    {
      // Already dealt with invalid lines in provider - no need to repeat
      // removed code (CC 2013-04-13) ...
      //    mInvalidLines << line;
      // In any case it may be a valid line that is excluded because of
      // bounds check...
      continue;
    }

    // At this point the current feature values are valid

    feature.setValid( true );
    feature.setFields( &attributeFields ); // allow name-based attribute lookups
    feature.setFeatureId( fid );
    feature.initAttributes( attributeFields.count() );

    if ( geom )
      feature.setGeometry( geom );

    // If we have subset expression, then ened attributes
    if ( ! mSubsetExpression && request.flags() & QgsFeatureRequest::SubsetOfAttributes )
    {
      const QgsAttributeList& attrs = request.subsetOfAttributes();
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

    // Are we using a subset expression, if so try and evaluate
    // and accept result if passes.

    if( mSubsetExpression )
    {
      QVariant isOk = mSubsetExpression->evaluate( &feature );
      if( mSubsetExpression->hasEvalError() ) continue;
      if( ! isOk.toBool() ) continue;
    }

    // We have a good line, so return
    return true;

  } // !mStream->atEnd()

  return false;
}


QgsGeometry* QgsDelimitedTextProvider::loadGeometryWkt( const QStringList& tokens, const QgsFeatureRequest& request )
{
  QgsGeometry* geom = 0;
  QString sWkt = tokens[mWktFieldIndex];

  geom = geomFromWkt( sWkt );

  if ( geom && geom->type() != mGeometryType )
  {
    delete geom;
    geom = 0;
  }
  if ( geom && !boundsCheck( geom, request ) )
  {
    delete geom;
    geom = 0;
  }
  return geom;
}


QgsGeometry* QgsDelimitedTextProvider::loadGeometryXY( const QStringList& tokens, const QgsFeatureRequest& request )
{
  QString sX = tokens[mXFieldIndex];
  QString sY = tokens[mYFieldIndex];
  QgsPoint pt;
  bool ok = pointFromXY( sX, sY, pt );

  if ( ok && boundsCheck( pt, request ) )
  {
    return QgsGeometry::fromPoint( pt );
  }
  return 0;
}

/**
 * Check to see if the point is within the selection rectangle
 */
bool QgsDelimitedTextProvider::boundsCheck( const QgsPoint &pt, const QgsFeatureRequest& request )
{
  // no selection rectangle or geometry => always in the bounds
  if ( request.filterType() != QgsFeatureRequest::FilterRect || ( request.flags() & QgsFeatureRequest::NoGeometry ) )
    return true;

  return request.filterRect().contains( pt );
}

/**
 * Check to see if the geometry is within the selection rectangle
 */
bool QgsDelimitedTextProvider::boundsCheck( QgsGeometry *geom, const QgsFeatureRequest& request )
{
  // no selection rectangle or geometry => always in the bounds
  if ( request.filterType() != QgsFeatureRequest::FilterRect || ( request.flags() & QgsFeatureRequest::NoGeometry ) )
    return true;

  if ( request.flags() & QgsFeatureRequest::ExactIntersect )
    return geom->intersects( request.filterRect() );
  else
    return geom->boundingBox().intersects( request.filterRect() );
}


void QgsDelimitedTextProvider::fetchAttribute( QgsFeature& feature, int fieldIdx, const QStringList& tokens )
{
  if( fieldIdx < 0 || fieldIdx >= attributeColumns.count()) return;
  int column = attributeColumns[fieldIdx];
  if( column < 0 || column >= tokens.count()) return;
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
  return NoCapabilities;
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
