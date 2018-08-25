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
#include "qgscameracontroller.h"
#include <QMenu>


float _normalizedAngle( float x )
{
  x = std::fmod( x, 360 );
  if ( x < 0 ) x += 360;
  return x;
}

template<typename Func1>
void _prepare3DViewsMenu( QMenu *menu, QgsLayout3DMapWidget *w, Func1 slot )
{
  QObject::connect( menu, &QMenu::aboutToShow, w, [menu, w, slot]
  {
    const QList<Qgs3DMapCanvasDockWidget *> lst = QgisApp::instance()->findChildren<Qgs3DMapCanvasDockWidget *>();
    menu->clear();
    for ( auto dock : lst )
    {
      QAction *a = menu->addAction( dock->mapCanvas3D()->objectName(), w, slot );
      // need to use a custom property for identification because Qt likes to add "&" to the action text
      a->setProperty( "name", dock->mapCanvas3D()->objectName() );
    }
    if ( lst.isEmpty() )
    {
      menu->addAction( QObject::tr( "No 3D maps defined" ) )->setEnabled( false );
    }
  } );
}

Qgs3DMapCanvasDockWidget *_dock3DViewFromSender( QObject *sender )
{
  QAction *action = qobject_cast<QAction *>( sender );
  if ( !action )
    return nullptr;

  QString actionText = action->property( "name" ).toString();
  const QList<Qgs3DMapCanvasDockWidget *> lst = QgisApp::instance()->findChildren<Qgs3DMapCanvasDockWidget *>();
  for ( auto dock : lst )
  {
    QString objName = dock->mapCanvas3D()->objectName();
    if ( objName == actionText )
    {
      return dock;
    }
  }
  return nullptr;
}


QgsLayout3DMapWidget::QgsLayout3DMapWidget( QgsLayoutItem3DMap *map3D )
  : QgsLayoutItemBaseWidget( nullptr, map3D )
  , mMap3D( map3D )
{
  setupUi( this );

  mCenterXSpinBox->setClearValue( 0 );
  mCenterYSpinBox->setClearValue( 0 );
  mCenterZSpinBox->setClearValue( 0 );
  mDistanceToCenterSpinBox->setClearValue( 1000 );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, map3D );
  mainLayout->addWidget( mItemPropertiesWidget );

  mMenu3DCanvases = new QMenu( this );
  mCopySettingsButton->setMenu( mMenu3DCanvases );
  _prepare3DViewsMenu( mMenu3DCanvases, this, &QgsLayout3DMapWidget::copy3DMapSettings );

  mMenu3DCanvasesPose = new QMenu( this );
  mPoseFromViewButton->setMenu( mMenu3DCanvasesPose );
  _prepare3DViewsMenu( mMenu3DCanvasesPose, this, &QgsLayout3DMapWidget::copeCameraPose );

  QList<QgsDoubleSpinBox *> lst;
  lst << mCenterXSpinBox << mCenterYSpinBox << mCenterZSpinBox << mDistanceToCenterSpinBox << mPitchAngleSpinBox << mHeadingAngleSpinBox;
  for ( QgsDoubleSpinBox *spinBox : lst )
    connect( spinBox, qgis::overload<double>::of( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayout3DMapWidget::updateCameraPose );

  updateCameraPoseWidgetsFromItem();
}

void QgsLayout3DMapWidget::updateCameraPoseWidgetsFromItem()
{
  QgsCameraPose pose = mMap3D->cameraPose();
  whileBlocking( mCenterXSpinBox )->setValue( pose.centerPoint().x() );
  whileBlocking( mCenterYSpinBox )->setValue( pose.centerPoint().y() );
  whileBlocking( mCenterZSpinBox )->setValue( pose.centerPoint().z() );
  whileBlocking( mDistanceToCenterSpinBox )->setValue( pose.distanceFromCenterPoint() );
  whileBlocking( mPitchAngleSpinBox )->setValue( _normalizedAngle( pose.pitchAngle() ) );
  whileBlocking( mHeadingAngleSpinBox )->setValue( _normalizedAngle( pose.headingAngle() ) );
}

void QgsLayout3DMapWidget::copy3DMapSettings()
{
  Qgs3DMapCanvasDockWidget *dock = _dock3DViewFromSender( sender() );
  if ( dock )
    mMap3D->setMapSettings( new Qgs3DMapSettings( *dock->mapCanvas3D()->map() ) );
}

void QgsLayout3DMapWidget::copeCameraPose()
{
  Qgs3DMapCanvasDockWidget *dock = _dock3DViewFromSender( sender() );
  if ( dock )
  {
    mMap3D->setCameraPose( dock->mapCanvas3D()->cameraController()->cameraPose() );
    updateCameraPoseWidgetsFromItem();
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
