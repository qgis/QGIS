/***************************************************************************
    qgsundowidget.cpp
    ---------------------
    begin                : June 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsundowidget.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"


QgsUndoWidget::QgsUndoWidget( QWidget *parent, QgsMapCanvas *mapCanvas )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  connect( undoButton, &QAbstractButton::clicked, this, &QgsUndoWidget::undo );
  connect( redoButton, &QAbstractButton::clicked, this, &QgsUndoWidget::redo );

  undoButton->setDisabled( true );
  redoButton->setDisabled( true );
  mMapCanvas = mapCanvas;
  mUndoView = new QUndoView( this );
  gridLayout->addWidget( mUndoView, 0, 0, 1, 2 );
  mUndoStack = nullptr;
  mPreviousIndex = 0;
  mPreviousCount = 0;
}

void QgsUndoWidget::setButtonsVisible( bool show )
{
  undoButton->setVisible( show );
  redoButton->setVisible( show );
}

void QgsUndoWidget::destroyStack()
{
  if ( mUndoStack )
  {
    // do not clear undo stack here, just null pointer
    mUndoStack = nullptr;
  }
  if ( mUndoView )
  {
    mUndoView->close();
    delete mUndoView;
    mUndoView = new QUndoView( dockWidgetContents );
    gridLayout->addWidget( mUndoView, 0, 0, 1, 2 );
  }
}

void QgsUndoWidget::undoChanged( bool value )
{
  undoButton->setDisabled( !value );
  emit undoStackChanged();
}

void QgsUndoWidget::redoChanged( bool value )
{
  redoButton->setDisabled( !value );
  emit undoStackChanged();
}

void QgsUndoWidget::indexChanged( int curIndx )
{
  // this is called twice when a non-current command is clicked in QUndoView
  //   first call has offset, second call will have offset of 0
  int curCount = 0;
  bool canRedo = true;
  if ( mUndoStack )
  {
    canRedo = mUndoStack->canRedo();
    curCount = mUndoStack->count();
  }
  int offset = std::abs( mPreviousIndex - curIndx );

  // when individually redoing, differentiate between last redo and a new command added to stack
  bool lastRedo = ( mPreviousIndex == ( mPreviousCount - 1 ) && mPreviousCount == curCount && !canRedo );

  if ( offset != 0 )
  {
    QgsDebugMsgLevel( QStringLiteral( "curIndx : %1" ).arg( curIndx ), 4 );
    QgsDebugMsgLevel( QStringLiteral( "offset  : %1" ).arg( offset ), 4 );
    QgsDebugMsgLevel( QStringLiteral( "curCount: %1" ).arg( curCount ), 4 );
    if ( lastRedo )
    {
      QgsDebugMsgLevel( QStringLiteral( "lastRedo: true" ), 4 );
    }
  }

  // avoid canvas redraws when only new command was added to stack (i.e. no user undo/redo action)
  // or when user has clicked back in QUndoView history then added a new command to the stack
  if ( offset > 1 || ( offset == 1 && ( canRedo || lastRedo ) ) )
  {
    if ( mMapCanvas )
    {
      mMapCanvas->refresh();
    }
  }

  mPreviousIndex = curIndx;
  mPreviousCount = curCount;
}

void QgsUndoWidget::undo()
{
  if ( mUndoStack )
    mUndoStack->undo();
}

void QgsUndoWidget::redo()
{
  if ( mUndoStack )
    mUndoStack->redo();
}

void QgsUndoWidget::setUndoStack( QUndoStack *undoStack )
{
  if ( mUndoView )
  {
    mUndoView->close();
    delete mUndoView;
    mUndoView = nullptr;
  }

  mUndoStack = undoStack;
  mPreviousIndex = mUndoStack->index();
  mPreviousCount = mUndoStack->count();

  mUndoView = new QUndoView( dockWidgetContents );
  mUndoView->setStack( undoStack );
  mUndoView->setObjectName( QStringLiteral( "undoView" ) );
  gridLayout->addWidget( mUndoView, 0, 0, 1, 2 );
//  setWidget( dockWidgetContents );
  connect( mUndoStack, &QUndoStack::canUndoChanged, this, &QgsUndoWidget::undoChanged );
  connect( mUndoStack, &QUndoStack::canRedoChanged, this, &QgsUndoWidget::redoChanged );

  // gets triggered also when a new command is added to stack, and twice when clicking a command in QUndoView
  connect( mUndoStack, &QUndoStack::indexChanged, this, &QgsUndoWidget::indexChanged );

  undoButton->setDisabled( !mUndoStack->canUndo() );
  redoButton->setDisabled( !mUndoStack->canRedo() );
}



void QgsUndoWidget::setupUi( QWidget *UndoWidget )
{
  if ( UndoWidget->objectName().isEmpty() )
    UndoWidget->setObjectName( QStringLiteral( "UndoWidget" ) );
  UndoWidget->resize( 200, 223 );
  UndoWidget->setMinimumSize( QSize( 200, 220 ) );
  dockWidgetContents = new QWidget( UndoWidget );
  dockWidgetContents->setObjectName( QStringLiteral( "dockWidgetContents" ) );
  gridLayout = new QGridLayout( dockWidgetContents );
  gridLayout->setObjectName( QStringLiteral( "gridLayout" ) );
  gridLayout->setContentsMargins( 0, 0, 0, 0 );
  spacerItem = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );

  gridLayout->addItem( spacerItem, 0, 0, 1, 1 );

  undoButton = new QPushButton( dockWidgetContents );
  undoButton->setObjectName( QStringLiteral( "undoButton" ) );
  undoButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionUndo.svg" ) ) );

  gridLayout->addWidget( undoButton, 1, 0, 1, 1 );

  redoButton = new QPushButton( dockWidgetContents );
  redoButton->setObjectName( QStringLiteral( "redoButton" ) );
  redoButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionRedo.svg" ) ) );

  gridLayout->addWidget( redoButton, 1, 1, 1, 1 );

  spacerItem1 = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );

  gridLayout->addItem( spacerItem1, 0, 1, 1, 1 );

  UndoWidget->setLayout( gridLayout );

  retranslateUi( UndoWidget );

  QMetaObject::connectSlotsByName( UndoWidget );
} // setupUi

void QgsUndoWidget::retranslateUi( QWidget *UndoWidget )
{
  UndoWidget->setWindowTitle( QApplication::translate( "UndoWidget", "Undo/Redo", nullptr, QApplication::UnicodeUTF8 ) );
  undoButton->setText( QApplication::translate( "UndoWidget", "Undo", nullptr, QApplication::UnicodeUTF8 ) );
  redoButton->setText( QApplication::translate( "UndoWidget", "Redo", nullptr, QApplication::UnicodeUTF8 ) );
  Q_UNUSED( UndoWidget );
}

