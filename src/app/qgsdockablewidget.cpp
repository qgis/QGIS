/***************************************************************************
  qgsdockablewidget.cpp
  --------------------------------------
  Date                 : January 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdockablewidget.h"

#include "qgisapp.h"
#include "qgs3dmapcanvaswidget.h"
#include "qgsdockwidget.h"

#include <QWidget>

QgsDockableWidget::QgsDockableWidget( QWidget *parent )
  : QWidget( parent )
{
  this->setAttribute( Qt::WA_DeleteOnClose );

  mDock = new QgsDockWidget( this );
  mDock->setAttribute( Qt::WA_DeleteOnClose );

  mDock->setWidget( nullptr );
  mDock->setAllowedAreas( Qt::AllDockWidgetAreas );

  mDock->setVisible( false );

  mDialog = new QDialog( this, Qt::Window );
  mDialog->setAttribute( Qt::WA_DeleteOnClose );
  QVBoxLayout *vl = new QVBoxLayout();
  vl->setContentsMargins( 0, 0, 0, 0 );
  mDialog->setLayout( vl );
  mDialog->hide();

  connect( mDock, &QgsDockWidget::closed, [ = ]()
  {
    this->close();
  } );
  connect( mDialog, &QDialog::finished, [ = ]()
  {
    this->close();
  } );

  toggleDockMode( true );
}

void QgsDockableWidget::setWidget( QWidget *widget )
{
  mWidget = widget;
  mWidget->setParent( this );
  toggleDockMode( mIsDocked );
}

bool QgsDockableWidget::isDocked() const
{
  return mIsDocked;
}

void QgsDockableWidget::toggleDockMode( bool docked )
{
  if ( docked )
  {
    // going from window -> dock
    mIsDocked = true;

    mDialog->layout()->removeWidget( mWidget );
    mDock->setWidget( mWidget );

    mDialog->hide();
    mDock->setVisible( true );

    // TODO: apply resizing in a better way
    mDock->resize( mDock->size() + QSize( 1, 1 ) );
    mDock->resize( mDock->size() - QSize( 1, 1 ) );
  }
  else
  {
    // going from dock -> window
    mIsDocked = false;

    mDialog->layout()->addWidget( mWidget );
    mDock->setWidget( nullptr );

    mDialog->raise();
    mDialog->show();
    mDock->setVisible( false );

    // TODO: apply resizing in a better way
    mDialog->resize( mDialog->size() + QSize( 1, 1 ) );
    mDialog->resize( mDialog->size() - QSize( 1, 1 ) );
  }
}

void QgsDockableWidget::setWindowTitle( const QString &title )
{
  this->QWidget::setWindowTitle( title );
  mDialog->setWindowTitle( title );
  mDock->setWindowTitle( title );
}

void QgsDockableWidget::closeEvent( QCloseEvent *e )
{
  mDialog->layout()->removeWidget( mWidget );
  mDock->setWidget( nullptr );
  mWidget->setParent( this );

  emit closed();
  QWidget::closeEvent( e );
}

