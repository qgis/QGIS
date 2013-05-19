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

QgsReaderFeatures::~QgsReaderFeatures()
{
  if ( mListSelectedFeature.count() > 0 )
  {
    mListSelectedFeature.clear();
  }

} // QgsReaderFeatures::~QgsReaderFeatures()

bool QgsReaderFeatures::nextFeature( QgsFeature & feature )
{
  return ( this->*mFuncNextFeature )( feature );

} // bool QgsReaderFeatures::nextFeature(QgsFeature & feature)

void QgsReaderFeatures::initReader( bool useSelection )
{
  if ( useSelection )
  {
    mListSelectedFeature = mLayer->selectedFeatures();
    mIterSelectedFeature = mListSelectedFeature.begin();
    mFuncNextFeature = &QgsReaderFeatures::nextFeatureSelected;
  }
  else
  {
    mFit = mLayer->getFeatures( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );
    mFuncNextFeature = &QgsReaderFeatures::nextFeatureTotal;
  }

} // void QgsReaderFeatures::initReader()

bool QgsReaderFeatures::nextFeatureTotal( QgsFeature & feature )
{
  return mFit.nextFeature( feature );
} // bool QgsReaderFeatures::nextFeatureTotal ( QgsFeature & feature )

bool QgsReaderFeatures::nextFeatureSelected( QgsFeature & feature )
{
  if ( mIterSelectedFeature == mListSelectedFeature.end() )
    return false;

  feature = *mIterSelectedFeature;
  mIterSelectedFeature++;

  return true;
} // bool QgsReaderFeatures::nextFeatureSelected( QgsFeature &feature )
