/***************************************************************************
                          qgscollapsiblegroupbox.cpp
                             -------------------
    begin                : August 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny dot dev at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscollapsiblegroupbox.h"

#include "qgsapplication.h"
#include "qgslogger.h"

#include <QToolButton>


QIcon QgsCollapsibleGroupBox::mCollapseIcon;
QIcon QgsCollapsibleGroupBox::mExpandIcon;

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( QWidget *parent )
    : QGroupBox( parent ), mCollapsed( true )
{
  init();
}

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( const QString &title,
    QWidget *parent )
    : QGroupBox( title, parent ), mCollapsed( true )
{
  init();
}

void QgsCollapsibleGroupBox::init()
{
  // init icons
  if ( mCollapseIcon.isNull() )
  {
    mCollapseIcon = QgsApplication::getThemeIcon( "/mIconCollapse.png" );
    mExpandIcon = QgsApplication::getThemeIcon( "/mIconExpand.png" );
  }

  // customize style sheet
  // TODO: move to app stylesheet system, when appropriate
  QString ss;
  ss += "QgsCollapsibleGroupBox::title {";
  ss += "  subcontrol-origin: margin;";
  ss += "  subcontrol-position: top left;";
  ss += "  margin-left: 20px;";  // offset for disclosure triangle
  ss += "}";
  setStyleSheet( ss );

  // collapse button
  mCollapseButton = new QToolButton( this );
  mCollapseButton->setObjectName( "collapseButton" );
  mCollapseButton->setAutoRaise( true );
  mCollapseButton->setFixedSize( 16, 16 );
  // TODO set size (as well as margins) depending on theme
  mCollapseButton->setIconSize( QSize( 12, 12 ) );
  mCollapseButton->setIcon( mExpandIcon );

  // clear toolbutton default background and border
  // TODO: move to app stylesheet system, when appropriate
  QString ssd;
  ssd = QString( "QgsCollapsibleGroupBox > QToolButton#%1 {" ).arg( mCollapseButton->objectName() );
  ssd += "  background-color: rgba(255, 255, 255, 0); border: none;";
  ssd += "}";
  mCollapseButton->setStyleSheet( ssd );

  connect( mCollapseButton, SIGNAL( clicked() ), this, SLOT( toggleCollapsed() ) );
  connect( this, SIGNAL( clicked( bool ) ), this, SLOT( checkClicked() ) );
  connect( this, SIGNAL( toggled( bool ) ), this, SLOT( checkToggled() ) );
}

void QgsCollapsibleGroupBox::showEvent( QShowEvent * event )
{
  QGroupBox::showEvent( event );
  // expand if needed - any calls to setCollapsed() before have no effect
  if ( mCollapsed )
  {
    setCollapsed( mCollapsed );
  }
  else
  {
    /* manually uncollapsing (already default) on show may scroll scroll areas
       still emit signal for connections using uncollapsed state */
    emit collapsedStateChanged( this );
  }
}

void QgsCollapsibleGroupBox::checkClicked()
{
  mCollapseButton->setEnabled( true ); // always keep enabled
  // expand/collapse when clicked
  if ( isChecked() && isCollapsed() )
    setCollapsed( false );
  else if ( ! isChecked() && ! isCollapsed() )
    setCollapsed( true );
}

void QgsCollapsibleGroupBox::checkToggled()
{
  mCollapseButton->setEnabled( true ); // always keep enabled
}

void QgsCollapsibleGroupBox::toggleCollapsed()
{
  setCollapsed( !mCollapsed );
}

void QgsCollapsibleGroupBox::setCollapsed( bool collapse )
{
  if ( !isVisible() )
    return;

  mCollapsed = collapse;

  // for consistent look/spacing across platforms when collapsed
  setFlat( collapse );
  setMaximumHeight( collapse ? 28 : 16777215 );

  // if we are collapsing, save hidden widgets in a list
  if ( collapse )
  {
    mCollapseButton->setIcon( mExpandIcon );
    mHiddenWidgets.clear();
    foreach ( QWidget *widget, findChildren<QWidget*>() )
    {
      if ( widget->isHidden() && widget->objectName() != mCollapseButton->objectName() )
        mHiddenWidgets << widget;
    }
  }

  // show/hide widgets
  foreach ( QWidget *widget, findChildren<QWidget*>() )
    if ( widget->objectName() != mCollapseButton->objectName() )
      widget->setHidden( collapse );

  // if we are expanding, re-hide saved hidden widgets
  if ( ! collapse )
  {
    mCollapseButton->setIcon( mCollapseIcon );
    foreach ( QWidget *widget, mHiddenWidgets )
    {
      widget->setHidden( true );
    }
  }

  emit collapsedStateChanged( this );
}

