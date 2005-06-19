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
#include <iostream>

#include <qapplication.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qclipboard.h>

#include "qgsclipboard.h"


QgsClipboard::QgsClipboard()
  : mFeatureClipboard(0)
{
}

QgsClipboard::~QgsClipboard()
{
}

void QgsClipboard::replaceWithCopyOf( std::vector<QgsFeature> features )
{

  // Replace the QGis clipboard.
  mFeatureClipboard = features;
#ifdef QGISDEBUG
        std::cerr << "QgsClipboard::replaceWith: replaced QGis clipboard."
                  << std::endl;
#endif

  // Replace the system clipboard.
  // TODO: Add attributes as well.
  
  QStringList textLines;
  
  for (std::vector<QgsFeature>::iterator it  = features.begin();
                                         it != features.end();
                                       ++it)
  {
    textLines += it->geometry()->wkt();
  }
  
  QString textCopy = textLines.join("\n");

  QClipboard *cb = QApplication::clipboard();

  // Copy text into the clipboard
  cb->setText(textCopy, QClipboard::Clipboard);
  
#ifdef QGISDEBUG
        std::cerr << "QgsClipboard::replaceWith: replaced system clipboard with: "
                  << textCopy
                  << "."
                  << std::endl;
#endif

}

std::vector<QgsFeature*>* QgsClipboard::copyOf()
{

#ifdef QGISDEBUG
        std::cerr << "QgsClipboard::copyOf: returning clipboard."
                  << std::endl;
#endif
  
  std::vector<QgsFeature*>* featuresCopy = new std::vector<QgsFeature*>;

  //TODO: Slurp from the system clipboard as well.

  for (std::vector<QgsFeature>::iterator it  = mFeatureClipboard.begin();
                                         it != mFeatureClipboard.end();
                                       ++it)
  {
    featuresCopy->push_back( new QgsFeature(*it) );
  }
  
  return featuresCopy;
    
//  return mFeatureClipboard;
  
}

void QgsClipboard::clear()
{
  
  mFeatureClipboard.clear();

#ifdef QGISDEBUG
        std::cerr << "QgsClipboard::clear: cleared clipboard."
                  << std::endl;
#endif
}
  
void QgsClipboard::insert( QgsFeature& feature )
{
  mFeatureClipboard.push_back(feature);
#ifdef QGISDEBUG
        std::cerr << "QgsClipboard::insert: inserted " << feature.geometry()->wkt()
                  << std::endl;
#endif

}
 
