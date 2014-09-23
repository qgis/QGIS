/***************************************************************************
                          qgsreaderfeatures.cpp
                             -------------------
    begin                : Dec 29, 2009
    copyright            : (C) 2009 by Diego Moreira And Luiz Motta
    email                : moreira.geo at gmail.com And motta.luiz at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qgsvectordataprovider.h>

#include "qgsreaderfeatures.h"

QgsReaderFeatures::QgsReaderFeatures( QgsVectorLayer *layer, bool useSelection )
{
  mLayer = layer;

  initReader( useSelection );

} // QgsReaderFeatures::QgsReaderFeatures(QgsVectorLayer *layer, bool useSelection)

bool QgsReaderFeatures::nextFeature( QgsFeature & feature )
{
  return mFit.nextFeature( feature );
} // bool QgsReaderFeatures::nextFeature(QgsFeature & feature)

void QgsReaderFeatures::initReader( bool useSelection )
{
  if ( useSelection )
  {
    mFit = mLayer->selectedFeaturesIterator( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );
  }
  else
  {
    mFit = mLayer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );
  }
} // void QgsReaderFeatures::initReader()
