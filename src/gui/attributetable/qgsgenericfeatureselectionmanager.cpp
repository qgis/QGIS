/***************************************************************************
    qgsgenericfeatureselectionmanager.cpp
     --------------------------------------
    Date                 : 11.6.2013
    Copyright            : (C) 2013 Matthias Kuhn
    Email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgenericfeatureselectionmanager.h"

QgsGenericFeatureSelectionManager::QgsGenericFeatureSelectionManager( QObject *parent )
    : QgsIFeatureSelectionManager( parent )
{
}

QgsGenericFeatureSelectionManager::QgsGenericFeatureSelectionManager( const QgsFeatureIds& initialSelection, QObject* parent )
    : QgsIFeatureSelectionManager( parent )
    , mSelectedFeatures( initialSelection )
{
}

int QgsGenericFeatureSelectionManager::selectedFeatureCount()
{
  return mSelectedFeatures.size();
}

void QgsGenericFeatureSelectionManager::select( const QgsFeatureIds& ids )
{
  mSelectedFeatures += ids;
  emit selectionChanged( ids, QgsFeatureIds(), false );
}

void QgsGenericFeatureSelectionManager::deselect( const QgsFeatureIds& ids )
{
  mSelectedFeatures -= ids;
  emit selectionChanged( QgsFeatureIds(), ids, false );
}

void QgsGenericFeatureSelectionManager::setSelectedFeatures( const QgsFeatureIds& ids )
{
  QgsFeatureIds selected = mSelectedFeatures - ids;
  QgsFeatureIds deselected = ids - mSelectedFeatures;

  mSelectedFeatures = ids;
  emit selectionChanged( selected, deselected, true );
}

const QgsFeatureIds& QgsGenericFeatureSelectionManager::selectedFeaturesIds() const
{
  return mSelectedFeatures;
}
