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
#include "qgs3dmapsettings.h"
#include "qgscameracontroller.h"
#include <QMenu>
#include "qgs3dmapcanvaswidget.h"
#include "qgsmapviewsmanager.h"
#include "qgsoffscreen3dengine.h"
#include "qgs3dmapscene.h"
#include "qscreen.h"
#include "qgsmapcanvas.h"

float _normalizedAngle( float x )
{
  x = std::fmod( x, 360 );
  if ( x < 0 ) x += 360;
  return x;
}
void _prepare3DViewsMenu( QMenu *menu, QgsLayout3DMapWidget *w, const std::function< void( Qgs3DMapScene * ) > &slot )
{
  QObject::connect( menu, &QMenu::aboutToShow, w, [menu, slot]
  {
    const QList< QDomElement > views = QgsProject::instance()->viewsManager()->get3DViews();

    menu->clear();

    for ( const QDomElement &viewConfig : views )
    {
      QString viewName = viewConfig.attribute( QStringLiteral( "name" ) );
      QAction *a = new QAction( viewName, menu );
      menu->addAction( a );
      QObject::connect( a, &QAction::triggered, a, [slot, viewName]
      {
        QgsReadWriteContext readWriteContext;
        readWriteContext.setPathResolver( QgsProject::instance()->pathResolver() );

        QDomElement elem3DMap = QgsProject::instance()->viewsManager()->get3DViewSettings( viewName );

        QDomElement elem3D = elem3DMap.firstChildElement( QStringLiteral( "qgis3d" ) );
        Qgs3DMapSettings mapSettings = Qgs3DMapSettings();

        mapSettings.readXml( elem3D, readWriteContext );
        mapSettings.resolveReferences( *QgsProject::instance() );

        mapSettings.setTransformContext( QgsProject::instance()->transformContext() );
        mapSettings.setPathResolver( QgsProject::instance()->pathResolver() );
        mapSettings.setMapThemeCollection( QgsProject::instance()->mapThemeCollection() );

        // these things are not saved in project
        mapSettings.setSelectionColor( QgisApp::instance()->mapCanvas()->selectionColor() );
        mapSettings.setBackgroundColor( QgisApp::instance()->mapCanvas()->canvasColor() );
        mapSettings.setOutputDpi( QGuiApplication::primaryScreen()->logicalDotsPerInch() );

        QgsOffscreen3DEngine *engine = new QgsOffscreen3DEngine;
        Qgs3DMapScene *scene = new Qgs3DMapScene( mapSettings, engine );

        QDomElement elemCamera = elem3DMap.firstChildElement( QStringLiteral( "camera" ) );
        if ( !elemCamera.isNull() )
        {
          scene->cameraController()->readXml( elemCamera );
        }

        slot( scene );
      } );
    }

    if ( views.isEmpty() )
    {
      menu->addAction( QObject::tr( "No 3D maps defined" ) )->setEnabled( false );
    }
  } );
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
  _prepare3DViewsMenu( mMenu3DCanvases, this, [ = ]( Qgs3DMapScene * scene )
  {
    copy3DMapSettings( scene );
  } );

  mMenu3DCanvasesPose = new QMenu( this );
  mPoseFromViewButton->setMenu( mMenu3DCanvasesPose );
  _prepare3DViewsMenu( mMenu3DCanvasesPose, this, [ = ]( Qgs3DMapScene * scene ) { copyCameraPose( scene ); } );

  QList<QgsDoubleSpinBox *> lst;
  lst << mCenterXSpinBox << mCenterYSpinBox << mCenterZSpinBox << mDistanceToCenterSpinBox << mPitchAngleSpinBox << mHeadingAngleSpinBox;
  for ( QgsDoubleSpinBox *spinBox : std::as_const( lst ) )
    connect( spinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &QgsLayout3DMapWidget::updateCameraPose );

  updateCameraPoseWidgetsFromItem();
}

void QgsLayout3DMapWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
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

void QgsLayout3DMapWidget::copy3DMapSettings( Qgs3DMapScene *scene )
{
  if ( !scene )
    return;
  Qgs3DMapSettings *settings = new Qgs3DMapSettings( *scene->mapSettings() );

  // first setting passed on
  if ( !mMap3D->mapSettings() )
  {
    // copy background color
    mMap3D->setBackgroundColor( settings->backgroundColor() );

    // copy camera position details
    mMap3D->setCameraPose( scene->cameraController()->cameraPose() );
    updateCameraPoseWidgetsFromItem();
  }

  mMap3D->setMapSettings( settings );
}

void QgsLayout3DMapWidget::copyCameraPose( Qgs3DMapScene *scene )
{
  if ( scene )
  {
    QgsCameraPose cameraPose = QgsCameraPose( scene->cameraController()->cameraPose() );
    mMap3D->setCameraPose( cameraPose );
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

bool QgsLayout3DMapWidget::setNewItem( QgsLayoutItem *item )
{
  QgsLayoutItem3DMap *newItem = qobject_cast< QgsLayoutItem3DMap * >( item );
  if ( !newItem )
    return false;

  mMap3D = newItem;
  mItemPropertiesWidget->setItem( newItem );

  updateCameraPoseWidgetsFromItem();

  return true;
}
