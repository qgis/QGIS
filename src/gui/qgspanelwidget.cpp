/***************************************************************************
    qgspanelwidget.cpp
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                :
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDialog>
#include <QVBoxLayout>

#include "qgssettings.h"
#include "qgspanelwidget.h"
#include "qgslogger.h"

QgsPanelWidget::QgsPanelWidget( QWidget *parent )
  : QWidget( parent )
{
}

void QgsPanelWidget::connectChildPanels( const QList<QgsPanelWidget *> &panels )
{
  const auto constPanels = panels;
  for ( QgsPanelWidget *widget : constPanels )
  {
    connectChildPanel( widget );
  }
}

void QgsPanelWidget::connectChildPanel( QgsPanelWidget *panel )
{
  connect( panel, &QgsPanelWidget::showPanel, this, &QgsPanelWidget::openPanel );
  connect( panel, &QgsPanelWidget::widgetChanged, this, &QgsPanelWidget::widgetChanged );
}

void QgsPanelWidget::setDockMode( bool dockMode )
{
  mDockMode = dockMode;
}

bool QgsPanelWidget::applySizeConstraintsToStack() const
{
  return false;
}

QgsPanelWidget *QgsPanelWidget::findParentPanel( QWidget *widget )
{
  QWidget *p = widget;
  while ( p )
  {
    if ( QgsPanelWidget *panel = qobject_cast< QgsPanelWidget * >( p ) )
      return panel;

    if ( p->window() == p )
    {
      // break on encountering a window - e.g., a dialog opened from a panel should not inline
      // widgets inside the parent panel
      return nullptr;
    }

    p = p->parentWidget();
  }
  return nullptr;
}

QString QgsPanelWidget::menuButtonTooltip() const
{
  return QString();
}

QMenu *QgsPanelWidget::menuButtonMenu()
{
  return nullptr;
}

void QgsPanelWidget::openPanel( QgsPanelWidget *panel )
{
  //panel dock mode inherits from this panel
  panel->setDockMode( dockMode() );

  if ( mDockMode )
  {
    emit showPanel( panel );
  }
  else
  {
    // Show the dialog version if no one is connected
    QDialog *dlg = new QDialog();
    const QString key = QStringLiteral( "/UI/paneldialog/%1" ).arg( panel->panelTitle() );
    QgsSettings settings;
    dlg->restoreGeometry( settings.value( key ).toByteArray() );
    dlg->setWindowTitle( panel->panelTitle() );
    dlg->setLayout( new QVBoxLayout() );
    dlg->layout()->addWidget( panel );
    QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok );
    connect( buttonBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept );
    dlg->layout()->addWidget( buttonBox );
    dlg->exec();
    settings.setValue( key, dlg->saveGeometry() );
    panel->acceptPanel();
  }
}

void QgsPanelWidget::acceptPanel()
{
  emit panelAccepted( this );
}

void QgsPanelWidget::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_Escape )
  {
    acceptPanel();
  }
  else
  {
    QWidget::keyPressEvent( event );
  }
}

QgsPanelWidgetWrapper::QgsPanelWidgetWrapper( QWidget *widget, QWidget *parent )
  : QgsPanelWidget( parent )
  , mWidget( widget )
{
  this->setLayout( new QVBoxLayout() );
  this->layout()->setContentsMargins( 0, 0, 0, 0 );
  this->layout()->addWidget( widget );
}
