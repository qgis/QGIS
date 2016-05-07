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
#include <QSettings>
#include <QMimeData>
#include <QTextCodec>

#include "qgsclipboard.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsvectorlayer.h"
#include "qgsogrutils.h"
#include "qgsjsonutils.h"

QgsClipboard::QgsClipboard()
    : QObject()
    , mFeatureClipboard()
    , mFeatureFields()
    , mUseSystemClipboard( false )
{
  connect( QApplication::clipboard(), SIGNAL( dataChanged() ), this, SLOT( systemClipboardChanged() ) );
}

QgsClipboard::~QgsClipboard()
{
}

void QgsClipboard::replaceWithCopyOf( QgsVectorLayer *src )
{
  if ( !src )
    return;

  // Replace the QGis clipboard.
  mFeatureFields = src->fields();
  mFeatureClipboard = src->selectedFeatures();
  mCRS = src->crs();

  QgsDebugMsg( "replaced QGis clipboard." );

  setSystemClipboard();
  mUseSystemClipboard = false;
  emit changed();
}

void QgsClipboard::replaceWithCopyOf( QgsFeatureStore & featureStore )
{
  QgsDebugMsg( QString( "features count = %1" ).arg( featureStore.features().size() ) );
  mFeatureFields = featureStore.fields();
  mFeatureClipboard = featureStore.features();
  mCRS = featureStore.crs();
  setSystemClipboard();
  mUseSystemClipboard = false;
  emit changed();
}

QString QgsClipboard::generateClipboardText() const
{
  QSettings settings;
  CopyFormat format = AttributesWithWKT;
  if ( settings.contains( "/qgis/copyFeatureFormat" ) )
    format = static_cast< CopyFormat >( settings.value( "/qgis/copyFeatureFormat", true ).toInt() );
  else
  {
    //old format setting
    format = settings.value( "/qgis/copyGeometryAsWKT", true ).toBool() ? AttributesWithWKT : AttributesOnly;
  }

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
        textFields += "wkt_geom";
      }

      Q_FOREACH ( const QgsField& field, mFeatureFields )
      {
        textFields += field.name();
      }
      textLines += textFields.join( "\t" );
      textFields.clear();

      // then the field contents
      for ( QgsFeatureList::const_iterator it = mFeatureClipboard.constBegin(); it != mFeatureClipboard.constEnd(); ++it )
      {
        QgsAttributes attributes = it->attributes();

        // TODO: Set up Paste Transformations to specify the order in which fields are added.
        if ( format == AttributesWithWKT )
        {
          if ( it->constGeometry() )
            textFields += it->constGeometry()->exportToWkt();
          else
          {
            textFields += settings.value( "qgis/nullValue", "NULL" ).toString();
          }
        }

        // QgsDebugMsg("about to traverse fields.");
        for ( int idx = 0; idx < attributes.count(); ++idx )
        {
          // QgsDebugMsg(QString("inspecting field '%1'.").arg(it2->toString()));
          textFields += attributes.at( idx ).toString();
        }

        textLines += textFields.join( "\t" );
        textFields.clear();
      }

      return textLines.join( "\n" );
    }
    case GeoJSON:
    {
      QgsJSONExporter exporter;
      exporter.setSourceCrs( mCRS );
      return exporter.exportFeatures( mFeatureClipboard );
    }
  }
  return QString();
}

void QgsClipboard::setSystemClipboard()
{
  QString textCopy = generateClipboardText();

  QClipboard *cb = QApplication::clipboard();

  // Copy text into the clipboard

  // With qgis running under Linux, but with a Windows based X
  // server (Xwin32), ::Selection was necessary to get the data into
  // the Windows clipboard (which seems contrary to the Qt
  // docs). With a Linux X server, ::Clipboard was required.
  // The simple solution was to put the text into both clipboards.

#ifdef Q_OS_LINUX
  cb->setText( textCopy, QClipboard::Selection );
#endif
  cb->setText( textCopy, QClipboard::Clipboard );

  QgsDebugMsgLevel( QString( "replaced system clipboard with: %1." ).arg( textCopy ), 4 );
}

QgsFeatureList QgsClipboard::stringToFeatureList( const QString& string, const QgsFields& fields ) const
{
  //first try using OGR to read string
  QgsFeatureList features = QgsOgrUtils::stringToFeatureList( string, fields, QTextCodec::codecForName( "System" ) );

  if ( !features.isEmpty() )
    return features;

  // otherwise try to read in as WKT
  QStringList values = string.split( '\n' );
  if ( values.isEmpty() || string.isEmpty() )
    return features;

  Q_FOREACH ( const QString& row, values )
  {
    // Assume that it's just WKT for now.
    QgsGeometry* geometry = QgsGeometry::fromWkt( row );
    if ( !geometry )
      continue;

    QgsFeature feature;
    if ( !fields.isEmpty() )
      feature.setFields( fields, true );

    feature.setGeometry( geometry );
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

  return QgsOgrUtils::stringToFields( string, QTextCodec::codecForName( "System" ) );
}

QgsFeatureList QgsClipboard::copyOf( const QgsFields &fields ) const
{
  QgsDebugMsg( "returning clipboard." );
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

  QgsDebugMsg( "cleared clipboard." );
  emit changed();
}

void QgsClipboard::insert( const QgsFeature& feature )
{
  mFeatureClipboard.push_back( feature );

  QgsDebugMsgLevel( "inserted " + feature.constGeometry()->exportToWkt(), 4 );
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

QgsFeatureList QgsClipboard::transformedCopyOf( const QgsCoordinateReferenceSystem& destCRS, const QgsFields &fields ) const
{
  QgsFeatureList featureList = copyOf( fields );
  QgsCoordinateTransform ct( crs(), destCRS );

  QgsDebugMsg( "transforming clipboard." );
  for ( QgsFeatureList::iterator iter = featureList.begin(); iter != featureList.end(); ++iter )
  {
    iter->geometry()->transform( ct );
  }

  return featureList;
}

QgsCoordinateReferenceSystem QgsClipboard::crs() const
{
  return mCRS;
}

void QgsClipboard::setData( const QString& mimeType, const QByteArray& data, const QString& text )
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

void QgsClipboard::setText( const QString& text )
{
#ifdef Q_OS_LINUX
  QApplication::clipboard()->setText( text, QClipboard::Selection );
#endif
  QApplication::clipboard()->setText( text, QClipboard::Clipboard );
}

bool QgsClipboard::hasFormat( const QString& mimeType ) const
{
  return QApplication::clipboard()->mimeData()->hasFormat( mimeType );
}

QByteArray QgsClipboard::data( const QString& mimeType ) const
{
  return QApplication::clipboard()->mimeData()->data( mimeType );
}

void QgsClipboard::systemClipboardChanged()
{
  mUseSystemClipboard = true;
  emit changed();
}
