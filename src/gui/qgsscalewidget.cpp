/***************************************************************************
   qgsscalewidget.cpp
    --------------------------------------
   Date                 : 08.01.2015
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QHBoxLayout>

#include "qgsapplication.h"
#include "qgsscalewidget.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgslayoutmanager.h"
#include "qgslayoutitemmap.h"
#include "qgsprintlayout.h"

#include <QMenu>

QgsScaleWidget::QgsScaleWidget( QWidget *parent )
  : QWidget( parent )
{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 6 );

  mScaleComboBox = new QgsScaleComboBox( this );
  layout->addWidget( mScaleComboBox );

  mCurrentScaleButton = new QToolButton( this );
  mCurrentScaleButton->setToolTip( tr( "Set to current canvas scale" ) );
  mCurrentScaleButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapIdentification.svg" ) ) );

  mMenu = new QMenu( this );
  mCurrentScaleButton->setMenu( mMenu );
  mCurrentScaleButton->setPopupMode( QToolButton::MenuButtonPopup );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsScaleWidget::menuAboutToShow );

  layout->addWidget( mCurrentScaleButton );
  mCurrentScaleButton->hide();

  connect( mScaleComboBox, &QgsScaleComboBox::scaleChanged, this, &QgsScaleWidget::scaleChanged );
  connect( mCurrentScaleButton, &QAbstractButton::clicked, this, &QgsScaleWidget::setScaleFromCanvas );
}

void QgsScaleWidget::setShowCurrentScaleButton( bool showCurrentScaleButton )
{
  mShowCurrentScaleButton = showCurrentScaleButton;
  mCurrentScaleButton->setVisible( mShowCurrentScaleButton && mCanvas );
}

void QgsScaleWidget::setMapCanvas( QgsMapCanvas *canvas )
{
  mCanvas = canvas;
  mCurrentScaleButton->setVisible( mShowCurrentScaleButton && mCanvas );
}

bool QgsScaleWidget::isNull() const
{
  return mScaleComboBox->isNull();
}

void QgsScaleWidget::setAllowNull( bool allowNull )
{
  mScaleComboBox->setAllowNull( allowNull );
}

bool QgsScaleWidget::allowNull() const
{
  return mScaleComboBox->allowNull();
}

void QgsScaleWidget::setScaleFromCanvas()
{
  if ( !mCanvas )
    return;

  setScale( mCanvas->scale() );
}

void QgsScaleWidget::setNull()
{
  mScaleComboBox->setNull();
}

void QgsScaleWidget::setScale( double scale )
{
  mScaleComboBox->setScale( scale );
}

void QgsScaleWidget::menuAboutToShow()
{
  mMenu->clear();

  double scale = mCanvas->scale();
  QAction *canvasScaleAction = new QAction( QgsApplication::getThemeIcon( QStringLiteral( "/mActionMapIdentification.svg" ) ),
      tr( "Use Current Map Canvas Scale (1:%1)" ).arg( qgsDoubleToString( scale, 0 ) ), mMenu );
  connect( canvasScaleAction, &QAction::triggered, this, [this, scale] { setScale( scale ); } );
  mMenu->addAction( canvasScaleAction );

  bool first = true;
  if ( QgsLayoutManager *manager = QgsProject::instance()->layoutManager() )
  {
    const QList<QgsPrintLayout *> layouts = manager->printLayouts();
    for ( const QgsPrintLayout *layout : layouts )
    {
      QList< QgsLayoutItemMap * > maps;
      layout->layoutItems( maps );
      if ( maps.empty() )
        continue;

      if ( first )
        mMenu->addSeparator();

      first = false;

      QMenu *layoutMenu = new QMenu( layout->name(), mMenu );
      for ( const QgsLayoutItemMap *map : std::as_const( maps ) )
      {
        scale = map->scale();
        QAction *mapScaleAction = new QAction( tr( "%1 (1:%2)" ).arg( map->displayName(), qgsDoubleToString( scale, 0 ) ), mMenu );
        connect( mapScaleAction, &QAction::triggered, this, [this, scale] { setScale( scale ); } );
        layoutMenu->addAction( mapScaleAction );
      }
      mMenu->addMenu( layoutMenu );
    }
  }
}
