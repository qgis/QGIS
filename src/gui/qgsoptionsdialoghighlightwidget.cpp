/***************************************************************************
    qgsoptionsdialoghighlightwidget.cpp
     -------------------------------
    Date                 : February 2018
    Copyright            : (C) 2018 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QCheckBox>
#include <QEvent>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QTimer>
#include <QTreeView>
#include <QTreeWidget>

#include "qgsoptionsdialoghighlightwidget.h"
#include "qgsmessagebaritem.h"

#include "qgsoptionsdialoghighlightwidgetsimpl.h"




QgsOptionsDialogHighlightWidget::QgsOptionsDialogHighlightWidget( QWidget *widget )
  : QObject( widget )
  , mWidget( widget )
{}

QgsOptionsDialogHighlightWidget *QgsOptionsDialogHighlightWidget::createWidget( QWidget *widget )
{
  QWidget *parent = widget;
  while ( ( parent = parent->parentWidget() ) )
  {
    // do not register message bar content, items disappear and causes QGIS to crash
    if ( qobject_cast< QgsMessageBarItem * >( parent ) )
    {
      // return invalid widget
      return nullptr;
    }
  }

  if ( qobject_cast<QLabel *>( widget ) )
  {
    return new QgsOptionsDialogHighlightLabel( qobject_cast<QLabel *>( widget ) );
  }
  else if ( qobject_cast<QCheckBox *>( widget ) )
  {
    return new QgsOptionsDialogHighlightCheckBox( qobject_cast<QCheckBox *>( widget ) );
  }
  else if ( qobject_cast<QAbstractButton *>( widget ) )
  {
    return new QgsOptionsDialogHighlightButton( qobject_cast<QAbstractButton *>( widget ) );
  }
  else if ( qobject_cast<QGroupBox *>( widget ) )
  {
    return new QgsOptionsDialogHighlightGroupBox( qobject_cast<QGroupBox *>( widget ) );
  }
  else if ( qobject_cast<QTreeView *>( widget ) )
  {
    return new QgsOptionsDialogHighlightTree( qobject_cast<QTreeView *>( widget ) );
  }
  else
  {
    // return invalid widget
    return nullptr;
  }
}

bool QgsOptionsDialogHighlightWidget::searchHighlight( const QString &text )
{
  mSearchText = text;
  bool found = false;

  if ( !mWidget )
    return found;

  if ( mChangedStyle )
  {
    reset();
    mChangedStyle = false;
  }

  if ( mInstalledFilter )
  {
    mWidget->removeEventFilter( this );
    mInstalledFilter = false;
  }

  if ( !text.isEmpty() )
  {
    found = searchText( text );
  }

  if ( found )
  {

    if ( !mWidget->isVisible() )
    {
      mWidget->installEventFilter( this );
      mInstalledFilter = true;
    }
    else
    {
      mChangedStyle = highlightText( text );
    }
  }

  return found;
}

bool QgsOptionsDialogHighlightWidget::eventFilter( QObject *obj, QEvent *event )
{
  if ( mInstalledFilter && event->type() == QEvent::Show && obj == mWidget )
  {
    mWidget->removeEventFilter( this );
    mInstalledFilter = false;
    // instead of catching the event and calling show again
    // it might be better to use a timer to change the style
    // after the widget is shown
#if 1
    mWidget->show();
    mChangedStyle = highlightText( mSearchText );
    return true;
#else
    QTimer::singleShot( 500, this, [ = ]
    {
      mChangedStyle = highlightText( mSearchText );
    } );
#endif
  }
  return QObject::eventFilter( obj, event );
}



