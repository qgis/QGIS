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
#include "qgsdelimitedtextfile.h"

static const QString TEXT_PROVIDER_KEY = "delimitedtext";
static const QString TEXT_PROVIDER_DESCRIPTION = "Delimited text data provider";



QgsDelimitedTextProvider::QgsDelimitedTextProvider( QString uri )
    : QgsVectorDataProvider( uri )
    , mFile(0)
    , mFieldCount( 0 )
    , mXFieldIndex( -1 )
    , mYFieldIndex( -1 )
    , mWktFieldIndex( -1 )
    , mWktHasZM( false )
    , mWktHasPrefix( false )
    , mWktZMRegexp( "\\s+(?:z|m|zm)(?=\\s*\\()", Qt::CaseInsensitive )
    , mWktCrdRegexp( "(\\-?\\d+(?:\\.\\d*)?\\s+\\-?\\d+(?:\\.\\d*)?)\\s[\\s\\d\\.\\-]+" )
    , mWktPrefixRegexp( "^(?:\\d+\\s+|SRID\\=\\d+\\;)" )
    , mShowInvalidLines( false )
    , mCrs()
    , mWkbType( QGis::WKBUnknown )
    , mActiveIterator( 0 )
{

  QgsDebugMsg( "Delimited text file uri is " + uri );

  QUrl url = QUrl::fromEncoded( uri.toAscii() );
  mFile = new QgsDelimitedTextFile();
  mFile->setFromUrl(url);

  QString wktField;
  QString xField;
  QString yField;

  if ( url.hasQueryItem( "wktField" ) )
    wktField = url.queryItemValue( "wktField" );
  if ( url.hasQueryItem( "xField" ) )
    xField = url.queryItemValue( "xField" );
  if ( url.hasQueryItem( "yField" ) )
    yField = url.queryItemValue( "yField" );
  if ( url.hasQueryItem( "crs" ) )
    mCrs.createFromString( url.queryItemValue( "crs" ) );
  if ( url.hasQueryItem( "decimalPoint" ) )
    mDecimalPoint = url.queryItemValue( "decimalPoint" );

  QgsDebugMsg( "wktField is: " + wktField );
  QgsDebugMsg( "wktField is: " + wktField );
  QgsDebugMsg( "xField is: " + xField );
  QgsDebugMsg( "yField is: " + yField );

  // assume the layer is invalid until proven otherwise
  mValid = false;
  if ( ! mFile->isValid() )
  {
    // uri is invalid so the layer must be too...
    QgsDebugMsg( "Delimited text source invalid - filename or delimiter parameters" );
    return;
  }

  // Open the file and get number of rows, etc. We assume that the
  // file has a header row and process accordingly. Caller should make
  // sure that the delimited file is properly formed.

  mFile->reset();

  // set the initial extent
  //
  mExtent = QgsRectangle();

  QMap<int, bool> couldBeInt;
  QMap<int, bool> couldBeDouble;

  mNumberFeatures = 0;

  // If using column headers then read field names from the header
  
  mFieldCount = 0;
  bool fieldsCounted = false;

  if( mFile->useHeader())

    {
      // Get the fields from the header row and store them in the
      // fields vector
      QStringList &fieldList = mFile->columnNames();

      mFieldCount = fieldList.count();
      mFile->setMaxFields(mFieldCount);
      fieldsCounted = true;

      // We don't know anything about a text based field other
      // than its name. All fields are assumed to be text
      int fieldPos = 0;
      for ( int column = 0; column < mFieldCount; column++ )
      {
        QString field = fieldList[column];

        // check to see if this field matches either the x or y field
        if ( !wktField.isEmpty() && wktField == field )
        {
          mWktFieldIndex = column;
        }
        else if ( !xField.isEmpty() && xField == field )
        {
          mXFieldIndex = column;
        }
        else if ( !yField.isEmpty() && yField == field )
        {
          mYFieldIndex = column;
        }
        // WKT geometry field won't be displayed in attribute tables
        if ( column != mWktFieldIndex )
        {
            QgsDebugMsg( "Adding field: " + ( field ) );
            // assume that the field could be integer or double
            // for now, let's set field type as text
            attributeColumns.append( column );
            attributeFields.append( QgsField( field, QVariant::String, "Text" ) );
            couldBeInt.insert( fieldPos, true );
            couldBeDouble.insert( fieldPos, true );
            fieldPos++;
        }
      }
    }
      // Otherwise expect column names to be just numbers - make the 
      // column count at least as great as the largest column referenced.
      else
      {
          bool ok;
          int column;

          column = wktField.toInt(&ok);
          if( ok ) 
          {
              if( column > mFieldCount ) mFieldCount = column;
              mWktFieldIndex=column-1;
          }

          column = xField.toInt(&ok);
          if( ok ) 
          {
              if( column > mFieldCount ) mFieldCount = column;
              mXFieldIndex=column-1;
          }

          column = yField.toInt(&ok);
          if( ok ) 
          {
              if( column > mFieldCount ) mFieldCount = column;
              mYFieldIndex=column-1;
          }
      }
      if( mWktFieldIndex >= 0 )  { QgsDebugMsg( "Found wkt field: " + QString::number(mWktFieldIndex+1) );}
      if( mXFieldIndex >= 0 )  { QgsDebugMsg( "Found X field: " + QString::number(mXFieldIndex+1) );}
      if( mYFieldIndex >= 0 )  { QgsDebugMsg( "Found Y field: " + QString::number(mYFieldIndex+1) );}

  QStringList parts;

  while ( true )
  {
      QgsDelimitedTextFile::Status status = mFile->nextRecord( parts );
      if( status == QgsDelimitedTextFile::RecordEOF ) break;
      if( status != QgsDelimitedTextFile::RecordOk )
      {
          mInvalidLines << ("Invalid record format at line "+QString::number(mFile->recordLineNumber()));
          continue;
      }

      // If not using column headers, then expand column count to include
      // last non-null (blank) column

      if( ! fieldsCounted && parts.size() > mFieldCount )
      {
          for( int i = parts.size(); i-- > mFieldCount; )
          {
              if( ! parts[i].isEmpty() )
              {
                  mFieldCount = i+1;
              }
          }
      }

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
          if ( !mWktHasPrefix && sWkt.indexOf( mWktPrefixRegexp ) >= 0 )
            mWktHasPrefix = true;
          if ( mWktHasPrefix )
          {
            sWkt.remove( mWktPrefixRegexp );
          }
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
          mInvalidLines << ("Invalid WKT at line "+QString::number(mFile->recordLineNumber()));
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
          mInvalidLines << ("Invalid X or Y fields at line "+QString::number(mFile->recordLineNumber()));
        }
      }
      else
      {
        mWkbType = QGis::WKBNoGeometry;
        mNumberFeatures++;
      }

      int fieldPos = 0;
      for ( int i = 0; i < mFieldCount; i++ )
      {
        if( i == mWktFieldIndex ) continue;
        fieldPos++;
        if( fieldPos >= attributeColumns.count() )
        {
            QString field = "Col"+QString("%1").arg(i+1,2,10,QChar('0'));
            QgsDebugMsg( "Adding field: " + field );
            // assume that the field could be integer or double
            // for now, let's set field type as text
            attributeColumns.append( i );
            attributeFields.append( QgsField( field, QVariant::String, "Text" ) );
            couldBeInt.insert( fieldPos, true );
            couldBeDouble.insert( fieldPos, true );
        }
        QString &value = parts[i];
        if ( value.isEmpty() )
          continue;
        // try to convert attribute values to integer and double
        if ( couldBeInt[fieldPos] )
        {
          value.toInt( &couldBeInt[fieldPos] );
        }
        if ( couldBeDouble[fieldPos] )
        {
          value.toDouble( &couldBeDouble[fieldPos] );
        }
      }
    }

  QgsDebugMsg( "Field count for the delimited text file is " + QString::number( attributeFields.size() ) );
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

  if ( mFile ) { delete mFile; mFile=0; }
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
