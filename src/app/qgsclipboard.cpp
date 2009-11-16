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

/* $Id$ */

#include <fstream>

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QClipboard>

#include "qgsclipboard.h"
#include "qgsfeature.h"
#include "qgsfield.h"
#include "qgsgeometry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgslogger.h"


QgsClipboard::QgsClipboard()
    : mFeatureClipboard()
{
}

QgsClipboard::~QgsClipboard()
{
}

void QgsClipboard::replaceWithCopyOf( const QgsFieldMap& fields, QgsFeatureList& features )
{

  // Replace the QGis clipboard.
  mFeatureClipboard = features;
  QgsDebugMsg( "replaced QGis clipboard." );

  // Replace the system clipboard.

  QStringList textLines;
  QStringList textFields;

  // first do the field names
  textFields += "wkt_geom";
  for ( QgsFieldMap::const_iterator fit = fields.begin(); fit != fields.end(); ++fit )
  {
    textFields += fit->name();
  }
  textLines += textFields.join( "\t" );
  textFields.clear();


  // then the field contents
  for ( QgsFeatureList::iterator it = features.begin(); it != features.end(); ++it )
  {
    QgsAttributeMap attributes = it->attributeMap();


    // TODO: Set up Paste Transformations to specify the order in which fields are added.

    if ( it->geometry() )
      textFields += it->geometry()->exportToWkt();
    else
      textFields += "NULL";

    // QgsDebugMsg("about to traverse fields.");
    //
    for ( QgsAttributeMap::iterator it2 = attributes.begin(); it2 != attributes.end(); ++it2 )
    {
      // QgsDebugMsg(QString("inspecting field '%1'.").arg(it2->toString()));
      textFields += it2->toString();
    }

    textLines += textFields.join( "\t" );
    textFields.clear();
  }

  QString textCopy = textLines.join( "\n" );

  QClipboard *cb = QApplication::clipboard();

  // Copy text into the clipboard

  // With qgis running under Linux, but with a Windows based X
  // server (Xwin32), ::Selection was necessary to get the data into
  // the Windows clipboard (which seems contrary to the Qt
  // docs). With a Linux X server, ::Clipboard was required.
  // The simple solution was to put the text into both clipboards.

  // The ::Selection setText() below one may need placing inside so
  // #ifdef so that it doesn't get compiled under Windows.
  cb->setText( textCopy, QClipboard::Selection );
  cb->setText( textCopy, QClipboard::Clipboard );

  QgsDebugMsg( QString( "replaced system clipboard with: %1." ).arg( textCopy ) );
}

QgsFeatureList QgsClipboard::copyOf()
{

  QgsDebugMsg( "returning clipboard." );

  //TODO: Slurp from the system clipboard as well.

  return mFeatureClipboard;

//  return mFeatureClipboard;

}

void QgsClipboard::clear()
{
  mFeatureClipboard.clear();

  QgsDebugMsg( "cleared clipboard." );
}

void QgsClipboard::insert( QgsFeature& feature )
{
  mFeatureClipboard.push_back( feature );

  QgsDebugMsg( "inserted " + feature.geometry()->exportToWkt() );
}

bool QgsClipboard::empty()
{
  return mFeatureClipboard.empty();
}

QgsFeatureList QgsClipboard::transformedCopyOf( QgsCoordinateReferenceSystem destCRS )
{

  QgsFeatureList featureList = copyOf();
  QgsCoordinateTransform ct( crs(), destCRS );

  QgsDebugMsg( "transforming clipboard." );
  for ( QgsFeatureList::iterator iter = featureList.begin(); iter != featureList.end(); ++iter )
  {
    iter->geometry()->transform( ct );
  }

  return featureList;
}

void QgsClipboard::setCRS( QgsCoordinateReferenceSystem crs )
{
  mCRS = crs;
}

QgsCoordinateReferenceSystem QgsClipboard::crs()
{
  return mCRS;
}
