/***************************************************************************
  qgslayout3dmapwidget.cpp
  --------------------------------------
  Date                 : August 2018
  Copyright            : (C) 2018 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayout3dmapwidget.h"

#include "qgisapp.h"
#include "qgs3dmapcanvas.h"
#include "qgs3dmapcanvasdockwidget.h"
#include "qgs3dmapsettings.h"
#include <QMenu>

QgsLayout3DMapWidget::QgsLayout3DMapWidget( QgsLayoutItem3DMap *map3D )
  : QgsLayoutItemBaseWidget( nullptr, map3D )
  , mMap3D( map3D )
{
  setupUi( this );

  mMenu3DCanvases = new QMenu( this );
  mCopySettingsButton->setMenu( mMenu3DCanvases );
  connect( mMenu3DCanvases, &QMenu::aboutToShow, this, [ = ]
  {
    const QList<Qgs3DMapCanvasDockWidget *> lst = QgisApp::instance()->findChildren<Qgs3DMapCanvasDockWidget *>();
    mMenu3DCanvases->clear();
    for ( auto dock : lst )
    {
      QAction *a = mMenu3DCanvases->addAction( dock->mapCanvas3D()->objectName(), this, &QgsLayout3DMapWidget::copy3DMapSettings );
      // need to use a custom property for identification because Qt likes to add "&" to the action text
      a->setProperty( "name", dock->mapCanvas3D()->objectName() );
    }
    if ( lst.isEmpty() )
    {
      mMenu3DCanvases->addAction( tr( "No 3D maps defined" ) )->setEnabled( false );
    }
  } );

  QgsCameraPose pose = mMap3D->cameraPose();
  mCenterXSpinBox->setValue( pose.centerPoint().x() );
  mCenterYSpinBox->setValue( pose.centerPoint().y() );
  mCenterZSpinBox->setValue( pose.centerPoint().z() );
  mDistanceToCenterSpinBox->setValue( pose.distanceFromCenterPoint() );
  mPitchAngleSpinBox->setValue( pose.pitchAngle() );
  mHeadingAngleSpinBox->setValue( pose.headingAngle() );

  QList<QgsDoubleSpinBox *> lst;
  lst << mCenterXSpinBox << mCenterXSpinBox << mCenterXSpinBox << mDistanceToCenterSpinBox << mPitchAngleSpinBox << mHeadingAngleSpinBox;
  for ( QgsDoubleSpinBox *spinBox : lst )
    connect( spinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayout3DMapWidget::updateCameraPose );
}

void QgsLayout3DMapWidget::copy3DMapSettings()
{
  QAction *action = qobject_cast<QAction *>( sender() );
  if ( !action )
    return;

  QString actionText = action->property( "name" ).toString();
  const QList<Qgs3DMapCanvasDockWidget *> lst = QgisApp::instance()->findChildren<Qgs3DMapCanvasDockWidget *>();
  for ( auto dock : lst )
  {
    QString objName = dock->mapCanvas3D()->objectName();
    if ( objName == actionText )
    {
      mMap3D->setMapSettings( new Qgs3DMapSettings( *dock->mapCanvas3D()->map() ) );
      break;
    }
  }
}

void QgsLayout3DMapWidget::updateCameraPose()
{
  QgsCameraPose pose;
  pose.setCenterPoint( QgsVector3D( mCenterXSpinBox->value(), mCenterYSpinBox->value(), mCenterZSpinBox->value() ) );
  pose.setDistanceFromCenterPoint( mDistanceToCenterSpinBox->value() );
  pose.setPitchAngle( mPitchAngleSpinBox->value() );
  pose.setHeadingAngle( mHeadingAngleSpinBox->value() );
  mMap3D->setCameraPose( pose );
}
