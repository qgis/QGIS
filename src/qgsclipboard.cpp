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

  mFeatureClipboard = features;
#ifdef QGISDEBUG
        std::cerr << "QgsClipboard::replaceWith: replaced clipboard."
                  << std::endl;
#endif

}

std::vector<QgsFeature> QgsClipboard::copyOf()
{

#ifdef QGISDEBUG
        std::cerr << "QgsClipboard::copyOf: returning clipboard."
                  << std::endl;
#endif
  
  return mFeatureClipboard;
  
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
 
