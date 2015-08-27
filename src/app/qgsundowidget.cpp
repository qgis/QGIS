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

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgslayertreeview.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"


QgsUndoWidget::QgsUndoWidget( QWidget * parent, QgsMapCanvas * mapCanvas )
    : QDockWidget( parent )
{
  setupUi( this );
  setWidget( dockWidgetContents );

  connect( undoButton, SIGNAL( clicked() ), this, SLOT( undo() ) );
  connect( redoButton, SIGNAL( clicked() ), this, SLOT( redo() ) );
  connect( QgisApp::instance()->layerTreeView(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ),
           this, SLOT( layerChanged( QgsMapLayer* ) ) );

  undoButton->setDisabled( true );
  redoButton->setDisabled( true );
  mMapCanvas = mapCanvas;
  mUndoView = new QUndoView( dockWidgetContents );
  gridLayout->addWidget( mUndoView, 0, 0, 1, 2 );
  mUndoStack = NULL;
  mPreviousIndex = 0;
  mPreviousCount = 0;
}


void QgsUndoWidget::layerChanged( QgsMapLayer * layer )
{
  if ( layer != NULL )
  {
    setUndoStack( layer->undoStack() );
  }
  else
  {
    destroyStack();
  }
  emit undoStackChanged();
}


void QgsUndoWidget::destroyStack()
{
  if ( mUndoStack != NULL )
  {
    // do not clear undo stack here, just null pointer
    mUndoStack = NULL;
  }
  if ( mUndoView != NULL )
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
  int offset = qAbs( mPreviousIndex - curIndx );

  // when individually redoing, differentiate between last redo and a new command added to stack
  bool lastRedo = ( mPreviousIndex == ( mPreviousCount - 1 ) && mPreviousCount == curCount && !canRedo );

  if ( offset != 0 )
  {
    QgsDebugMsg( QString( "curIndx : %1" ).arg( curIndx ) );
    QgsDebugMsg( QString( "offset  : %1" ).arg( offset ) );
    QgsDebugMsg( QString( "curCount: %1" ).arg( curCount ) );
    if ( lastRedo )
    {
      QgsDebugMsg( QString( "lastRedo: true" ) );
    }
  }

  // avoid canvas redraws when only new command was added to stack (i.e. no user undo/redo action)
  // or when user has clicked back in QUndoView history then added a new command to the stack
  if ( offset > 1 || ( offset == 1 && ( canRedo || lastRedo ) ) )
  {
    if ( mMapCanvas )
    {
      QgsDebugMsg( QString( "trigger redraw" ) );
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

void QgsUndoWidget::setUndoStack( QUndoStack* undoStack )
{
  if ( mUndoView != NULL )
  {
    mUndoView->close();
    delete mUndoView;
    mUndoView = NULL;
  }

  mUndoStack = undoStack;
  mPreviousIndex = mUndoStack->index();
  mPreviousCount = mUndoStack->count();

  mUndoView = new QUndoView( dockWidgetContents );
  mUndoView->setStack( undoStack );
  mUndoView->setObjectName( "undoView" );
  gridLayout->addWidget( mUndoView, 0, 0, 1, 2 );
  setWidget( dockWidgetContents );
  connect( mUndoStack, SIGNAL( canUndoChanged( bool ) ), this, SLOT( undoChanged( bool ) ) );
  connect( mUndoStack, SIGNAL( canRedoChanged( bool ) ), this, SLOT( redoChanged( bool ) ) );

  // gets triggered also when a new command is added to stack, and twice when clicking a command in QUndoView
  connect( mUndoStack, SIGNAL( indexChanged( int ) ), this, SLOT( indexChanged( int ) ) );

  undoButton->setDisabled( !mUndoStack->canUndo() );
  redoButton->setDisabled( !mUndoStack->canRedo() );
}



void QgsUndoWidget::setupUi( QDockWidget *UndoWidget )
{
  if ( UndoWidget->objectName().isEmpty() )
    UndoWidget->setObjectName( QString::fromUtf8( "UndoWidget" ) );
  UndoWidget->resize( 200, 223 );
  UndoWidget->setMinimumSize( QSize( 200, 220 ) );
  dockWidgetContents = new QWidget( UndoWidget );
  dockWidgetContents->setObjectName( QString::fromUtf8( "dockWidgetContents" ) );
  gridLayout = new QGridLayout( dockWidgetContents );
  gridLayout->setObjectName( QString::fromUtf8( "gridLayout" ) );
  gridLayout->setContentsMargins( 0, 0, 0, 0 );
  spacerItem = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );

  gridLayout->addItem( spacerItem, 0, 0, 1, 1 );

  undoButton = new QPushButton( dockWidgetContents );
  undoButton->setObjectName( QString::fromUtf8( "undoButton" ) );
  undoButton->setIcon( QgsApplication::getThemeIcon( "mActionUndo.png" ) );

  gridLayout->addWidget( undoButton, 1, 0, 1, 1 );

  redoButton = new QPushButton( dockWidgetContents );
  redoButton->setObjectName( QString::fromUtf8( "redoButton" ) );
  redoButton->setIcon( QgsApplication::getThemeIcon( "mActionRedo.png" ) );

  gridLayout->addWidget( redoButton, 1, 1, 1, 1 );

  spacerItem1 = new QSpacerItem( 20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding );

  gridLayout->addItem( spacerItem1, 0, 1, 1, 1 );

  UndoWidget->setWidget( dockWidgetContents );

  retranslateUi( UndoWidget );

  QMetaObject::connectSlotsByName( UndoWidget );
} // setupUi

void QgsUndoWidget::retranslateUi( QDockWidget *UndoWidget )
{
  UndoWidget->setWindowTitle( QApplication::translate( "UndoWidget", "Undo/Redo Panel", 0, QApplication::UnicodeUTF8 ) );
  undoButton->setText( QApplication::translate( "UndoWidget", "Undo", 0, QApplication::UnicodeUTF8 ) );
  redoButton->setText( QApplication::translate( "UndoWidget", "Redo", 0, QApplication::UnicodeUTF8 ) );
  Q_UNUSED( UndoWidget );
}

