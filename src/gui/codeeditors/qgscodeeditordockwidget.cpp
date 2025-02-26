/***************************************************************************
  qgscodeeditordockwidget.cpp
  --------------------------------------
  Date                 : March 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscodeeditordockwidget.h"
#include "moc_qgscodeeditordockwidget.cpp"
#include "qgsdockablewidgethelper.h"

QgsCodeEditorDockWidget::QgsCodeEditorDockWidget( const QString &dockId, bool usePersistentWidget )
  : QWidget( nullptr )
{
  QgsDockableWidgetHelper::Options options = QgsDockableWidgetHelper::Option::RaiseTab;
  if ( usePersistentWidget )
    options.setFlag( QgsDockableWidgetHelper::Option::PermanentWidget );

  mDockableWidgetHelper = new QgsDockableWidgetHelper(
    tr( "Code Editor" ),
    this,
    QgsDockableWidgetHelper::sOwnerWindow,
    dockId,
    QStringList(),
    QgsDockableWidgetHelper::OpeningMode::RespectSetting,
    true,
    Qt::BottomDockWidgetArea,
    options
  );

  mDockToggleButton = mDockableWidgetHelper->createDockUndockToolButton();
  mDockToggleButton->setToolTip( tr( "Dock Code Editor" ) );
  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::closed, this, [=]() {
    close();
  } );

  connect( mDockableWidgetHelper, &QgsDockableWidgetHelper::visibilityChanged, this, &QgsCodeEditorDockWidget::visibilityChanged );
}

QgsCodeEditorDockWidget::~QgsCodeEditorDockWidget()
{
  delete mDockableWidgetHelper;
}

void QgsCodeEditorDockWidget::setTitle( const QString &title )
{
  mDockableWidgetHelper->setWindowTitle( title );
}

QToolButton *QgsCodeEditorDockWidget::dockToggleButton()
{
  return mDockToggleButton;
}

void QgsCodeEditorDockWidget::setDockObjectName( const QString &name )
{
  mDockableWidgetHelper->setDockObjectName( name );
}

bool QgsCodeEditorDockWidget::isUserVisible() const
{
  return mDockableWidgetHelper->isUserVisible();
}

void QgsCodeEditorDockWidget::setUserVisible( bool visible )
{
  mDockableWidgetHelper->setUserVisible( visible );
}
