/***************************************************************************
     qgsclipboard.cpp  -  QGIS internal clipboard for storage of features
     --------------------------------------------------------------------
    begin                : 20 May, 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <fstream>

#include <QApplication>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QClipboard>
#include <QMimeData>
#include <QTextCodec>

#include "qgsclipboard.h"
#include "qgsfeature.h"
#include "qgsfeaturestore.h"
#include "qgsfields.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsogrutils.h"
#include "qgsjsonutils.h"
#include "qgssettings.h"
#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"

QgsClipboard::QgsClipboard()
{
  connect( QApplication::clipboard(), &QClipboard::dataChanged, this, &QgsClipboard::systemClipboardChanged );
}

void QgsClipboard::replaceWithCopyOf( QgsVectorLayer *src )
{
  if ( !src )
    return;

  // Replace the QGis clipboard.
  mFeatureFields = src->fields();
  mFeatureClipboard = src->selectedFeatures();
  mCRS = src->crs();
  mSrcLayer = src;
  QgsDebugMsg( QStringLiteral( "replaced QGis clipboard." ) );

  setSystemClipboard();
  mUseSystemClipboard = false;
  emit changed();
}

void QgsClipboard::replaceWithCopyOf( QgsFeatureStore &featureStore )
{
  QgsDebugMsg( QStringLiteral( "features count = %1" ).arg( featureStore.features().size() ) );
  mFeatureFields = featureStore.fields();
  mFeatureClipboard = featureStore.features();
  mCRS = featureStore.crs();
  mSrcLayer = nullptr;
  setSystemClipboard();
  mUseSystemClipboard = false;
  emit changed();
}

QString QgsClipboard::generateClipboardText() const
{
  CopyFormat format = QgsSettings().enumValue( QStringLiteral( "qgis/copyFeatureFormat" ),  AttributesWithWKT );

  switch ( format )
  {
    case AttributesOnly:
    case AttributesWithWKT:
    {
      QStringList textLines;
      QStringList textFields;

      // first do the field names
      if ( format == AttributesWithWKT )
      {
        textFields += QStringLiteral( "wkt_geom" );
      }

      Q_FOREACH ( const QgsField &field, mFeatureFields )
      {
        textFields += field.name();
      }
      textLines += textFields.join( QStringLiteral( "\t" ) );
      textFields.clear();

      // then the field contents
      for ( QgsFeatureList::const_iterator it = mFeatureClipboard.constBegin(); it != mFeatureClipboard.constEnd(); ++it )
      {
        QgsAttributes attributes = it->attributes();

        // TODO: Set up Paste Transformations to specify the order in which fields are added.
        if ( format == AttributesWithWKT )
        {
          if ( it->hasGeometry() )
            textFields += it->geometry().asWkt();
          else
          {
            textFields += QgsApplication::nullRepresentation();
          }
        }

        for ( int idx = 0; idx < attributes.count(); ++idx )
        {
          // QgsDebugMsg(QString("inspecting field '%1'.").arg(it2->toString()));
          textFields += attributes.at( idx ).toString();
        }

        textLines += textFields.join( QStringLiteral( "\t" ) );
        textFields.clear();
      }

      return textLines.join( QStringLiteral( "\n" ) );
    }
    case GeoJSON:
    {
      QgsJsonExporter exporter;
      exporter.setSourceCrs( mCRS );
      return exporter.exportFeatures( mFeatureClipboard );
    }
  }
  return QString();
}

void QgsClipboard::setSystemClipboard()
{
  // avoid overwriting internal clipboard - note that on Windows, the call to QClipboard::setText
  // below doesn't immediately trigger QClipboard::dataChanged, and accordingly the call to
  // systemClipboardChanged() is delayed. So by setting mIgnoreNextSystemClipboardChange we indicate
  // that just the next call to systemClipboardChanged() should be ignored
  mIgnoreNextSystemClipboardChange = true;

  QClipboard *cb = QApplication::clipboard();

  // Copy text into the clipboard
  QString textCopy = generateClipboardText();
  QMimeData *m = new QMimeData();
  m->setText( textCopy );

  if ( mFeatureClipboard.count() < 1000 )
  {
    CopyFormat format = QgsSettings().enumValue( QStringLiteral( "qgis/copyFeatureFormat" ), AttributesWithWKT );

    QString htmlCopy;
    switch ( format )
    {
      case AttributesOnly:
      case AttributesWithWKT:
        htmlCopy = textCopy;
        htmlCopy.replace( '\n', QStringLiteral( "</td></tr><tr><td>" ) );
        htmlCopy.replace( '\t', QStringLiteral( "</td><td>" ) );
        htmlCopy = QStringLiteral( "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/></head><body><table border=\"1\"><tr><td>" ) + htmlCopy + QStringLiteral( "</td></tr></table></body></html>" );
        break;
      case GeoJSON:
        break;
    }
    if ( !htmlCopy.isEmpty() )
    {
      m->setHtml( htmlCopy );
    }
  }

  // With qgis running under Linux, but with a Windows based X
  // server (Xwin32), ::Selection was necessary to get the data into
  // the Windows clipboard (which seems contrary to the Qt
  // docs). With a Linux X server, ::Clipboard was required.
  // The simple solution was to put the text into both clipboards.
#ifdef Q_OS_LINUX
  cb->setMimeData( m, QClipboard::Selection );
#endif
  cb->setMimeData( m, QClipboard::Clipboard );

  QgsDebugMsgLevel( QStringLiteral( "replaced system clipboard with: %1." ).arg( textCopy ), 4 );
}

QgsFeatureList QgsClipboard::stringToFeatureList( const QString &string, const QgsFields &fields ) const
{
  //first try using OGR to read string
  QgsFeatureList features = QgsOgrUtils::stringToFeatureList( string, fields, QTextCodec::codecForName( "System" ) );

  if ( !features.isEmpty() )
    return features;

  // otherwise try to read in as WKT
  QStringList values = string.split( '\n' );
  if ( values.isEmpty() || string.isEmpty() )
    return features;

  QgsFields sourceFields = retrieveFields();

  Q_FOREACH ( const QString &row, values )
  {
    // Assume that it's just WKT for now. because GeoJSON is managed by
    // previous QgsOgrUtils::stringToFeatureList call
    // Get the first value of a \t separated list. WKT clipboard pasted
    // feature has first element the WKT geom.
    // This split is to fix the following issue: https://issues.qgis.org/issues/16870
    // Value separators are set in generateClipboardText
    QStringList fieldValues = row.split( '\t' );
    if ( fieldValues.isEmpty() )
      continue;

    QgsFeature feature;
    feature.setFields( sourceFields );
    feature.initAttributes( fieldValues.size() - 1 );

    //skip header line
    if ( fieldValues.at( 0 ) == QLatin1String( "wkt_geom" ) )
    {
      continue;
    }

    for ( int i = 1; i < fieldValues.size(); ++i )
    {
      feature.setAttribute( i - 1, fieldValues.at( i ) );
    }

    QgsGeometry geometry = QgsGeometry::fromWkt( fieldValues[0] );
    if ( !geometry.isNull() )
    {
      feature.setGeometry( geometry );
    }

    features.append( feature );
  }
  return features;
}

QgsFields QgsClipboard::retrieveFields() const
{
  QClipboard *cb = QApplication::clipboard();

#ifdef Q_OS_LINUX
  QString string = cb->text( QClipboard::Selection );
#else
  QString string = cb->text( QClipboard::Clipboard );
#endif

  QgsFields f = QgsOgrUtils::stringToFields( string, QTextCodec::codecForName( "System" ) );
  if ( f.size() < 1 )
  {
    if ( string.isEmpty() )
    {
      return f;
    }

    //wkt?
    QStringList lines = string.split( '\n' );
    if ( !lines.empty() )
    {
      QStringList fieldNames = lines.at( 0 ).split( '\t' );
      //wkt / text always has wkt_geom as first attribute (however values can be NULL)
      if ( fieldNames.at( 0 ) != QLatin1String( "wkt_geom" ) )
      {
        return f;
      }

      for ( int i = 0; i < fieldNames.size(); ++i )
      {
        QString fieldName = fieldNames.at( i );
        if ( fieldName == QLatin1String( "wkt_geom" ) )
        {
          continue;
        }

        f.append( QgsField( fieldName, QVariant::String ) );
      }
    }
  }
  return f;
}

QgsFeatureList QgsClipboard::copyOf( const QgsFields &fields ) const
{
  QgsDebugMsg( QStringLiteral( "returning clipboard." ) );
  if ( !mUseSystemClipboard )
    return mFeatureClipboard;

  QClipboard *cb = QApplication::clipboard();

#ifdef Q_OS_LINUX
  QString text = cb->text( QClipboard::Selection );
#else
  QString text = cb->text( QClipboard::Clipboard );
#endif

  return stringToFeatureList( text, fields );
}

void QgsClipboard::clear()
{
  mFeatureClipboard.clear();

  QgsDebugMsg( QStringLiteral( "cleared clipboard." ) );
  emit changed();
}

void QgsClipboard::insert( const QgsFeature &feature )
{
  mFeatureClipboard.push_back( feature );

  QgsDebugMsgLevel( "inserted " + feature.geometry().asWkt(), 4 );
  mUseSystemClipboard = false;
  emit changed();
}

bool QgsClipboard::isEmpty() const
{
  QClipboard *cb = QApplication::clipboard();
#ifdef Q_OS_LINUX
  QString text = cb->text( QClipboard::Selection );
#else
  QString text = cb->text( QClipboard::Clipboard );
#endif
  return text.isEmpty() && mFeatureClipboard.empty();
}

QgsFeatureList QgsClipboard::transformedCopyOf( const QgsCoordinateReferenceSystem &destCRS, const QgsFields &fields ) const
{
  QgsFeatureList featureList = copyOf( fields );

  QgisApp::instance()->askUserForDatumTransform( crs(), destCRS );
  QgsCoordinateTransform ct = QgsCoordinateTransform( crs(), destCRS, QgsProject::instance() );

  QgsDebugMsg( QStringLiteral( "transforming clipboard." ) );
  for ( QgsFeatureList::iterator iter = featureList.begin(); iter != featureList.end(); ++iter )
  {
    QgsGeometry g = iter->geometry();
    g.transform( ct );
    iter->setGeometry( g );
  }

  return featureList;
}

QgsCoordinateReferenceSystem QgsClipboard::crs() const
{
  return mCRS;
}

void QgsClipboard::setData( const QString &mimeType, const QByteArray &data, const QString &text )
{
  mUseSystemClipboard = true;
  QMimeData *mdata = new QMimeData();
  mdata->setData( mimeType, data );
  if ( !text.isEmpty() )
  {
    mdata->setText( text );
  }
  // Transfers ownership to the clipboard object
#ifdef Q_OS_LINUX
  QApplication::clipboard()->setMimeData( mdata, QClipboard::Selection );
#endif
  QApplication::clipboard()->setMimeData( mdata, QClipboard::Clipboard );
}

void QgsClipboard::setText( const QString &text )
{
#ifdef Q_OS_LINUX
  QApplication::clipboard()->setText( text, QClipboard::Selection );
#endif
  QApplication::clipboard()->setText( text, QClipboard::Clipboard );
}

bool QgsClipboard::hasFormat( const QString &mimeType ) const
{
  return QApplication::clipboard()->mimeData()->hasFormat( mimeType );
}

QByteArray QgsClipboard::data( const QString &mimeType ) const
{
  return QApplication::clipboard()->mimeData()->data( mimeType );
}

QgsFields QgsClipboard::fields() const
{
  if ( !mUseSystemClipboard )
    return mFeatureFields;
  else
    return retrieveFields();
}

void QgsClipboard::systemClipboardChanged()
{
  if ( mIgnoreNextSystemClipboardChange )
  {
    mIgnoreNextSystemClipboardChange = false;
    return;
  }

  mUseSystemClipboard = true;
  emit changed();
}
