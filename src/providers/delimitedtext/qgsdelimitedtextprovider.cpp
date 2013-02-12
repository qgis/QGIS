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
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsrectangle.h"
#include "qgis.h"

#include "qgsdelimitedtextsourceselect.h"
#include "qgsdelimitedtextfeatureiterator.h"

static const QString TEXT_PROVIDER_KEY = "delimitedtext";
static const QString TEXT_PROVIDER_DESCRIPTION = "Delimited text data provider";


QString QgsDelimitedTextProvider::readLine( QTextStream *stream )
{
  QString buffer;

  while ( !stream->atEnd() )
  {
    QChar c = stream->read( 1 ).at( 0 );

    if ( c == '\r' || c == '\n' )
    {
      if ( buffer.isEmpty() )
      {
        // skip leading CR / LF
        continue;
      }

      break;
    }

    buffer.append( c );
  }

  return buffer;
}

QStringList QgsDelimitedTextProvider::splitLine( QString line, QString delimiterType, QString delimiter )
{
  QgsDebugMsgLevel( "Attempting to split the input line: " + line + " using delimiter " + delimiter, 3 );

  QStringList parts;
  if ( delimiterType == "regexp" )
    parts = line.split( QRegExp( delimiter ) );
  else
    parts = line.split( delimiter );

  QgsDebugMsgLevel( "Split line into " + QString::number( parts.size() ) + " parts", 3 );

  if ( delimiterType == "plain" )
  {
    QChar delim;
    int i = 0, first = parts.size();
    while ( i < parts.size() )
    {
      if ( delim == 0 && ( parts[i][0] == '"' || parts[i][0] == '\'' ) )
      {
        delim = parts[i][0];
        first = i;
        continue;
      }

      if ( delim != 0 && parts[i][ parts[i].length() - 1 ] == delim )
      {
        parts[first] = parts[first].mid( 1 );
        parts[i] = parts[i].left( parts[i].length() - 1 );

        if ( first < i )
        {
          QStringList values;
          while ( first <= i )
          {
            values << parts[first];
            parts.takeAt( first );
            i--;
          }

          parts.insert( first, values.join( delimiter ) );
        }

        first = -1;
        delim = 0;
      }

      i++;

      if ( i == parts.size() && first >= 0 )
      {
        i = first + 1;
        first = -1;
        delim = 0;
      }
    }
  }

  return parts;
}

QgsDelimitedTextProvider::QgsDelimitedTextProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , mDelimiter( "," )
    , mDelimiterType( "plain" )
    , mFieldCount( 0 )
    , mXFieldIndex( -1 )
    , mYFieldIndex( -1 )
    , mWktFieldIndex( -1 )
    , mWktHasZM( false )
    , mWktZMRegexp( "\\s+(?:z|m|zm)(?=\\s*\\()", Qt::CaseInsensitive )
    , mWktCrdRegexp( "(\\-?\\d+(?:\\.\\d*)?\\s+\\-?\\d+(?:\\.\\d*)?)\\s[\\s\\d\\.\\-]+" )
    , mFile( 0 )
    , mStream( 0 )
    , mSkipLines( 0 )
    , mFirstDataLine( 0 )
    , mShowInvalidLines( false )
    , mCrs()
    , mWkbType( QGis::WKBUnknown )
    , mActiveIterator( 0 )
{
  QUrl url = QUrl::fromEncoded( uri.toAscii() );

  // Extract the provider definition from the url
  mFileName = url.toLocalFile();

  QString wktField;
  QString xField;
  QString yField;

  if ( url.hasQueryItem( "delimiter" ) )
    mDelimiter = url.queryItemValue( "delimiter" );
  if ( url.hasQueryItem( "delimiterType" ) )
    mDelimiterType = url.queryItemValue( "delimiterType" );
  if ( url.hasQueryItem( "wktField" ) )
    wktField = url.queryItemValue( "wktField" );
  if ( url.hasQueryItem( "xField" ) )
    xField = url.queryItemValue( "xField" );
  if ( url.hasQueryItem( "yField" ) )
    yField = url.queryItemValue( "yField" );
  if ( url.hasQueryItem( "skipLines" ) )
    mSkipLines = url.queryItemValue( "skipLines" ).toInt();
  if ( url.hasQueryItem( "crs" ) )
    mCrs.createFromString( url.queryItemValue( "crs" ) );
  if ( url.hasQueryItem( "decimalPoint" ) )
    mDecimalPoint = url.queryItemValue( "decimalPoint" );

  QgsDebugMsg( "Data source uri is " + uri );
  QgsDebugMsg( "Delimited text file is: " + mFileName );
  QgsDebugMsg( "Delimiter is: " + mDelimiter );
  QgsDebugMsg( "Delimiter type is: " + mDelimiterType );
  QgsDebugMsg( "wktField is: " + wktField );
  QgsDebugMsg( "xField is: " + xField );
  QgsDebugMsg( "yField is: " + yField );
  QgsDebugMsg( "skipLines is: " + QString::number( mSkipLines ) );

  // if delimiter contains some special characters, convert them
  if ( mDelimiterType != "regexp" )
    mDelimiter.replace( "\\t", "\t" ); // replace "\t" with a real tabulator

  // assume the layer is invalid until proven otherwise
  mValid = false;
  if ( mFileName.isEmpty() || mDelimiter.isEmpty() )
  {
    // uri is invalid so the layer must be too...
    QgsDebugMsg( "Data source is invalid" );
    return;
  }

  // check to see that the file exists and perform some sanity checks
  if ( !QFile::exists( mFileName ) )
  {
    QgsDebugMsg( "Data source " + dataSourceUri() + " doesn't exist" );
    return;
  }

  // Open the file and get number of rows, etc. We assume that the
  // file has a header row and process accordingly. Caller should make
  // sure that the delimited file is properly formed.
  mFile = new QFile( mFileName );
  if ( !mFile->open( QIODevice::ReadOnly ) )
  {
    QgsDebugMsg( "Data source " + dataSourceUri() + " could not be opened" );
    delete mFile;
    mFile = 0;
    return;
  }

  // now we have the file opened and ready for parsing

  // set the initial extent
  mExtent = QgsRectangle();

  QMap<int, bool> couldBeInt;
  QMap<int, bool> couldBeDouble;

  mStream = new QTextStream( mFile );
  QString line;
  mNumberFeatures = 0;
  int lineNumber = 0;
  bool hasFields = false;
  while ( !mStream->atEnd() )
  {
    lineNumber++;
    line = readLine( mStream ); // line of text excluding '\n', default local 8 bit encoding.

    if ( lineNumber < mSkipLines + 1 )
      continue;

    if ( line.isEmpty() )
      continue;

    if ( !hasFields )
    {
      // Get the fields from the header row and store them in the
      // fields vector
      QStringList fieldList = splitLine( line );

      mFieldCount = fieldList.count();

      // We don't know anything about a text based field other
      // than its name. All fields are assumed to be text
      int fieldPos = 0;
      for ( int column = 0; column < mFieldCount; column++ )
      {
        QString field = fieldList[column];

        if (( field.left( 1 ) == "'" || field.left( 1 ) == "\"" ) &&
            field.left( 1 ) == field.right( 1 ) )
          // eat quotes
          field = field.mid( 1, field.length() - 2 );

        if ( field.length() == 0 )
          // skip empty field names
          continue;

        // check to see if this field matches either the x or y field
        if ( !wktField.isEmpty() && wktField == field )
        {
          QgsDebugMsg( "Found wkt field: " + ( field ) );
          mWktFieldIndex = column;
        }
        else if ( !xField.isEmpty() && xField == field )
        {
          QgsDebugMsg( "Found x field: " + ( field ) );
          mXFieldIndex = column;
        }
        else if ( !yField.isEmpty() && yField == field )
        {
          QgsDebugMsg( "Found y field: " + ( field ) );
          mYFieldIndex = column;
        }

        // WKT geometry field won't be displayed in attribute tables
        if ( column == mWktFieldIndex )
          continue;

        QgsDebugMsg( "Adding field: " + ( field ) );
        // assume that the field could be integer or double
        // for now, let's set field type as text
        attributeColumns.append( column );
        attributeFields.append( QgsField( field, QVariant::String, "Text" ) );
        couldBeInt.insert( fieldPos, true );
        couldBeDouble.insert( fieldPos, true );
        fieldPos++;
      }
      if ( mWktFieldIndex >= 0 )
      {
        mXFieldIndex = -1;
        mYFieldIndex = -1;
      }
      QgsDebugMsg( "wktfield index: " + QString::number( mWktFieldIndex ) );
      QgsDebugMsg( "xfield index: " + QString::number( mXFieldIndex ) );
      QgsDebugMsg( "yfield index: " + QString::number( mYFieldIndex ) );
      QgsDebugMsg( "Field count for the delimited text file is " + QString::number( attributeFields.size() ) );
      hasFields = true;
    }
    else // hasFields == true - field names already read
    {
      if ( mFirstDataLine == 0 )
        mFirstDataLine = lineNumber;

      // split the line on the delimiter
      QStringList parts = splitLine( line );

      // Ensure that the input has at least the required number of fields (mainly to tolerate
      // missed blank strings at end of row)
      while ( parts.size() < mFieldCount )
        parts.append( QString::null );

      if ( mWktFieldIndex >= 0 )
      {
        // Get the wkt - confirm it is valid, get the type, and
        // if compatible with the rest of file, add to the extents

        QString sWkt = parts[mWktFieldIndex];
        QgsGeometry *geom = 0;
        try
        {
          if ( !mWktHasZM && sWkt.indexOf( mWktZMRegexp ) >= 0 )
            mWktHasZM = true;
          if ( mWktHasZM )
          {
            sWkt.remove( mWktZMRegexp ).replace( mWktCrdRegexp, "\\1" );
          }
          geom = QgsGeometry::fromWkt( sWkt );
        }
        catch ( ... )
        {
          mInvalidLines << line;
          geom = 0;
        }

        if ( geom )
        {
          QGis::WkbType type = geom->wkbType();
          if ( type != QGis::WKBNoGeometry )
          {
            if ( mNumberFeatures == 0 )
            {
              mNumberFeatures++;
              mWkbType = type;
              mExtent = geom->boundingBox();
            }
            else if ( type == mWkbType )
            {
              mNumberFeatures++;
              QgsRectangle bbox( geom->boundingBox() );
              mExtent.combineExtentWith( &bbox );
            }
          }
          delete geom;
        }
      }
      else if ( mWktFieldIndex == -1 && mXFieldIndex >= 0 && mYFieldIndex >= 0 )
      {
        // Get the x and y values, first checking to make sure they
        // aren't null.

        QString sX = parts[mXFieldIndex];
        QString sY = parts[mYFieldIndex];


        if ( !mDecimalPoint.isEmpty() )
        {
          sX.replace( mDecimalPoint, "." );
          sY.replace( mDecimalPoint, "." );
        }

        bool xOk = false;
        bool yOk = false;
        double x = sX.toDouble( &xOk );
        double y = sY.toDouble( &yOk );

        if ( xOk && yOk )
        {
          if ( mNumberFeatures > 0 )
          {
            mExtent.combineExtentWith( x, y );
          }
          else
          {
            // Extent for the first point is just the first point
            mExtent.set( x, y, x, y );
            mWkbType = QGis::WKBPoint;
          }
          mNumberFeatures++;
        }
        else
        {
          mInvalidLines << line;
        }
      }
      else
      {
        mWkbType = QGis::WKBNoGeometry;
        mNumberFeatures++;
      }

      for ( int i = 0; i < attributeFields.size(); i++ )
      {
        QString &value = parts[attributeColumns[i]];
        if ( value.isEmpty() )
          continue;
        // try to convert attribute values to integer and double
        if ( couldBeInt[i] )
        {
          value.toInt( &couldBeInt[i] );
        }
        if ( couldBeDouble[i] )
        {
          value.toDouble( &couldBeDouble[i] );
        }
      }
    }
  }

  QgsDebugMsg( "geometry type is: " + QString::number( mWkbType ) );
  QgsDebugMsg( "feature count is: " + QString::number( mNumberFeatures ) );

  // now it's time to decide the types for the fields
  for ( int i = 0; i < attributeFields.count(); ++i )
  {
    QgsField& fld = attributeFields[i];
    if ( couldBeInt[i] )
    {
      fld.setType( QVariant::Int );
      fld.setTypeName( "integer" );
    }
    else if ( couldBeDouble[i] )
    {
      fld.setType( QVariant::Double );
      fld.setTypeName( "double" );
    }
  }

  mValid = mWkbType != QGis::WKBUnknown;
}

QgsDelimitedTextProvider::~QgsDelimitedTextProvider()
{
  if ( mActiveIterator )
    mActiveIterator->close();

  if ( mFile )
    mFile->close();
  delete mFile;
  delete mStream;
}


QString QgsDelimitedTextProvider::storageType() const
{
  return "Delimited text file";
}


QgsFeatureIterator QgsDelimitedTextProvider::getFeatures( const QgsFeatureRequest& request )
{
  return QgsFeatureIterator( new QgsDelimitedTextFeatureIterator( this, request ) );
}


void QgsDelimitedTextProvider::handleInvalidLines()
{
  if ( mShowInvalidLines && !mInvalidLines.isEmpty() )
  {
    mShowInvalidLines = false;
    QgsMessageOutput* output = QgsMessageOutput::createMessageOutput();
    output->setTitle( tr( "Error" ) );
    output->setMessage( tr( "Note: the following lines were not loaded because QGIS was "
                            "unable to determine values for the x and y coordinates:\n" ),
                        QgsMessageOutput::MessageText );

    output->appendMessage( "Start of invalid lines." );
    for ( int i = 0; i < mInvalidLines.size(); ++i )
      output->appendMessage( mInvalidLines.at( i ) );
    output->appendMessage( "End of invalid lines." );

    output->showMessage();

    // We no longer need these lines.
    mInvalidLines.clear();
  }
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
