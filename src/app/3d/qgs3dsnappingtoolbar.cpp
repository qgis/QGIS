/***************************************************************************
    qgs3dsnappingtoolbar.cpp
    -------------------
    begin                : November 2025
    copyright            : (C) 2025 Oslandia
    email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgs3dsnappingtoolbar.h"

#include "qgs3dmapcanvaswidget.h"
#include "qgs3dmapsettings.h"
#include "qgs3dsnappingmanager.h"
#include "qgsapplication.h"
#include "qgsdoublespinbox.h"
#include "qgssettings.h"
#include "qgsunittypes.h"

#include <QMenu>
#include <QSizePolicy>
#include <QToolButton>

Qgs3DSnappingToolbar::Qgs3DSnappingToolbar( Qgs3DMapCanvasWidget *parent, Qgs3DSnappingManager *snapper, const QgsSettings &setting )
  : QToolBar( tr( "Snapping Toolbar" ), parent ), mSnapper( snapper )
{
  setMovable( true );
  setVisible( setting.value( u"/3D/snappingToolbar/visibility"_s, false, QgsSettings::Gui ).toBool() );
  connect( this, &QToolBar::visibilityChanged, this, &Qgs3DSnappingToolbar::onVisibilityChanged );

  const double snapDistance = setting.value( u"/3D/snapping/default_tolerance"_s, 2.0, QgsSettings::Gui ).toDouble();
  mSnapper->setTolerance( snapDistance );

  mSnappingToleranceSpinBox = new QgsDoubleSpinBox();
  mSnappingToleranceSpinBox->setDecimals( 2 );
  mSnappingToleranceSpinBox->setMinimum( 0.0 );
  mSnappingToleranceSpinBox->setMaximum( 9999.99 );
  mSnappingToleranceSpinBox->setToolTip( tr( "Snapping Tolerance" ) );
  mSnappingToleranceSpinBox->setValue( snapDistance );
  mSnappingToleranceSpinBox->setClearValue( snapDistance );
  mSnappingToleranceSpinBox->setEnabled( true );

  connect( mSnappingToleranceSpinBox, qOverload<double>( &QgsDoubleSpinBox::valueChanged ), this, &Qgs3DSnappingToolbar::onToleranceChanged );

  mSnappingAction = new QAction( QgsApplication::getThemeIcon( u"mIconSnapping.svg"_s ), tr( "Snapping" ), this );

  mSnappingMenu = new QMenu( this );
  mSnappingAction->setMenu( mSnappingMenu );
  addAction( mSnappingAction );

  QToolButton *snappingButton = qobject_cast<QToolButton *>( widgetForAction( mSnappingAction ) );
  snappingButton->setToolTip( tr( "Snapping Type" ) );
  snappingButton->setPopupMode( QToolButton::ToolButtonPopupMode::InstantPopup );

  QAction *snappingVertexAction = new QAction( QIcon( QgsApplication::iconPath( u"mIconSnappingEndpoint.svg"_s ) ), tr( "Snap on vertex" ) );
  snappingVertexAction->setData( QVariant::fromValue( Qgs3DSnappingManager::SnappingType3D::Vertex ) );
  snappingVertexAction->setCheckable( true );
  snappingVertexAction->setChecked( mSnapper->snappingType() & Qgs3DSnappingManager::SnappingType3D::Vertex );
  mSnappingMenu->addAction( snappingVertexAction );

  QAction *snappingMidEdgeAction = new QAction( QIcon( QgsApplication::iconPath( u"mIconSnappingMiddle.svg"_s ) ), tr( "Snap on mid edge" ) );
  snappingMidEdgeAction->setData( QVariant::fromValue( Qgs3DSnappingManager::SnappingType3D::MiddleEdge ) );
  snappingMidEdgeAction->setCheckable( true );
  snappingMidEdgeAction->setChecked( mSnapper->snappingType() & Qgs3DSnappingManager::SnappingType3D::MiddleEdge );
  mSnappingMenu->addAction( snappingMidEdgeAction );

  QAction *snappingAlongEdgeAction = new QAction( QIcon( QgsApplication::iconPath( u"mIconSnappingSegment.svg"_s ) ), tr( "Snap on edge" ) );
  snappingAlongEdgeAction->setData( QVariant::fromValue( Qgs3DSnappingManager::SnappingType3D::AlongEdge ) );
  snappingAlongEdgeAction->setCheckable( true );
  snappingAlongEdgeAction->setChecked( mSnapper->snappingType() & Qgs3DSnappingManager::SnappingType3D::AlongEdge );
  mSnappingMenu->addAction( snappingAlongEdgeAction );

  QAction *snappingCenterFaceAction = new QAction( QIcon( QgsApplication::iconPath( u"mIconSnappingCentroid.svg"_s ) ), tr( "Snap on face center" ) );
  snappingCenterFaceAction->setData( QVariant::fromValue( Qgs3DSnappingManager::SnappingType3D::CenterFace ) );
  snappingCenterFaceAction->setCheckable( true );
  snappingCenterFaceAction->setChecked( mSnapper->snappingType() & Qgs3DSnappingManager::SnappingType3D::CenterFace );
  mSnappingMenu->addAction( snappingCenterFaceAction );

  mSnappingAction->setIcon( QgsApplication::getThemeIcon( u"mIconSnappingDisabled.svg"_s ) );
  snappingButton->setChecked( false );
  connect( snappingButton, &QToolButton::triggered, this, &Qgs3DSnappingToolbar::onSnappingButtonTriggered );

  addWidget( mSnappingToleranceSpinBox );
  QWidget *snappingSpacer = new QWidget( this );
  snappingSpacer->setFixedWidth( 6 );
  addWidget( snappingSpacer );

  onSnappingButtonTriggered( nullptr );
}

void Qgs3DSnappingToolbar::onSnappingButtonTriggered( QAction *action )
{
  if ( action != nullptr )
  {
    unsigned int newSnappingMode = static_cast<int>( mSnapper->snappingType() );
    const Qgs3DSnappingManager::SnappingType3D actionFlag = static_cast<Qgs3DSnappingManager::SnappingType3D>( action->data().toInt() );
    newSnappingMode ^= actionFlag;

    mSnapper->setSnappingType( static_cast<Qgs3DSnappingManager::SnappingType3D>( newSnappingMode ) );
  }

  if ( mSnapper->snappingType() == Qgs3DSnappingManager::SnappingType3D::Off )
  {
    mSnappingAction->setIcon( QgsApplication::getThemeIcon( u"mIconSnappingDisabled.svg"_s ) );
    mSnappingToleranceSpinBox->setEnabled( false );
  }
  else
  {
    mSnappingAction->setIcon( QgsApplication::getThemeIcon( u"mIconSnapping.svg"_s ) );
    mSnappingToleranceSpinBox->setEnabled( true );
  }
}

void Qgs3DSnappingToolbar::update( const Qgs3DMapSettings *mapSettings )
{
  mSnappingToleranceSpinBox->setSuffix( u" %1"_s.arg( QgsUnitTypes::toAbbreviatedString( mapSettings->crs().mapUnits() ) ) );
}

void Qgs3DSnappingToolbar::onVisibilityChanged()
{
  const bool toolbarIsVisible = isVisible();
  QgsSettings setting;
  setting.setValue( u"/3D/snappingToolbar/visibility"_s, toolbarIsVisible, QgsSettings::Gui );

  // disable snapping if the toolbar is hidden
  if ( !toolbarIsVisible )
  {
    mSnapper->setSnappingType( Qgs3DSnappingManager::SnappingType3D::Off );
    for ( QAction *action : mSnappingMenu->actions() )
    {
      action->setChecked( false );
    }
    onSnappingButtonTriggered( nullptr );
  }
}

void Qgs3DSnappingToolbar::onToleranceChanged()
{
  const double newTolerance = mSnappingToleranceSpinBox->value();
  mSnapper->setTolerance( newTolerance );
  QgsSettings setting;
  setting.setValue( u"/3D/snapping/default_tolerance"_s, newTolerance, QgsSettings::Gui );
}
