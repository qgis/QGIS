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
#include <QLayout>
#include <QStyle>


QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( QWidget *parent )
    : QGroupBox( parent ), mCollapsed( false )
{
  init();
}

QgsCollapsibleGroupBox::QgsCollapsibleGroupBox( const QString &title,
    QWidget *parent )
    : QGroupBox( title, parent ), mCollapsed( false )
{
  init();
}

void QgsCollapsibleGroupBox::init()
{

  connect( this, SIGNAL( toggled( bool ) ), this, SLOT( checkboxToggled() ) );

  QString ss;
  ss += "QGroupBox {";
  ss += "  margin-top: 28px;";  // fix for non-checkbox-groupbox spacing
  ss += "}";
  ss += "QGroupBox::title {";
  ss += "  subcontrol-origin: margin;";
  ss += "  subcontrol-position: top left;";
  ss += "  margin-left: 24px;";  // offset for disclosure triangle
  ss += "  margin-top: 10px;";  // offset to match extra top margin
  ss += "}";
  setStyleSheet( ss );

  mDisclosure = new QToolButton( this );
  mDisclosureName = QString( "grpboxDisclosure" );
  mDisclosure->setObjectName( mDisclosureName );
  mDisclosure->setFixedSize( 16, 16 );
  mDisclosure->move( 0, 10 ); // match title offset
  mDisclosure->setIconSize( QSize( 16, 16 ) );

  // get rid of toolbutton background
  QString ssd;
  ssd += QString( "QToolButton#%1 {" ).arg( mDisclosureName );
  ssd += "  background-color: rgba(255, 255, 255, 0);";
  ssd += "}";
  mDisclosure->setStyleSheet( ssd );

  connect( mDisclosure, SIGNAL( clicked() ), this, SLOT( toggleCollapsed() ) );
}


void QgsCollapsibleGroupBox::showEvent( QShowEvent * event )
{
  QGroupBox::showEvent( event );
  // collapse if needed - any calls to setCollapsed() before have no effect
  setCollapsed( true );
}

void QgsCollapsibleGroupBox::checkboxToggled()
{
  mDisclosure->setEnabled( true ); // always keep enabled
}

void QgsCollapsibleGroupBox::toggleCollapsed()
{
  setCollapsed( !mCollapsed );
}

void QgsCollapsibleGroupBox::setCollapsed( bool collapse )
{
  if ( ! isVisible() )
    return;

  mCollapsed = collapse;

  setFlat( collapse );
  setMaximumHeight( collapse ? 36 : 16777215 );

  // if we are collapsing, save hidden widgets in a list
  if ( collapse )
  {
    mDisclosure->setIcon( QgsApplication::getThemeIcon( "/mIconExpand.png" ) );
    mHiddenWidgets.clear();
    foreach ( QWidget *widget, findChildren<QWidget*>() )
    {
      if ( widget->isHidden() && widget->objectName() != mDisclosureName )
        mHiddenWidgets << widget;
    }
  }

  // show/hide widgets
  foreach ( QWidget *widget, findChildren<QWidget*>() )
    if ( widget->objectName() != mDisclosureName )
      widget->setHidden( collapse );

  // if we are expanding, re-hide saved hidden widgets
  if ( ! collapse )
  {
    mDisclosure->setIcon( QgsApplication::getThemeIcon( "/mIconCollapse.png" ) );
    foreach ( QWidget *widget, mHiddenWidgets )
    {
      widget->setHidden( true );
    }
  }

  if ( mCollapsed )
    emit collapsed( this );
  else
    emit expanded( this );
}

