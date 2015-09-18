/***************************************************************************
    qgsfeatureselectiondlg.cpp
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

#include "qgsfeatureselectiondlg.h"

#include "qgsgenericfeatureselectionmanager.h"
#include "qgsdistancearea.h"

QgsFeatureSelectionDlg::QgsFeatureSelectionDlg( QgsVectorLayer* vl, QWidget *parent )
    : QDialog( parent )
    , mVectorLayer( vl )
{
  setupUi( this );

  mFeatureSelection = new QgsGenericFeatureSelectionManager( mDualView );

  mDualView->setFeatureSelectionManager( mFeatureSelection );

  // TODO: Proper QgsDistanceArea, proper mapcanvas
  mDualView->init( mVectorLayer, NULL );
}

const QgsFeatureIds& QgsFeatureSelectionDlg::selectedFeatures()
{
  return mFeatureSelection->selectedFeaturesIds();
}


