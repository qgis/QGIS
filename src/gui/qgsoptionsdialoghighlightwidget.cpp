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
#include <QTableView>

#include "qgsoptionsdialoghighlightwidget.h"
#include "moc_qgsoptionsdialoghighlightwidget.cpp"
#include "qgsmessagebaritem.h"
#include "qgsfilterlineedit.h"

#include "qgsoptionsdialoghighlightwidgetsimpl.h"


QgsOptionsDialogHighlightWidget::QgsOptionsDialogHighlightWidget( QWidget *widget )
  : mWidget( widget )
{}

QgsOptionsDialogHighlightWidget *QgsOptionsDialogHighlightWidget::createWidget( QWidget *widget )
{
  QWidget *parent = widget;
  while ( ( parent = parent->parentWidget() ) )
  {
    // do not register message bar content, items disappear and causes QGIS to crash
    // do not register QgsFilterLineEdit's child widgets, the clear button might be deleted
    if ( qobject_cast<QgsMessageBarItem *>( parent ) || qobject_cast<QgsFilterLineEdit *>( parent ) )
    {
      // return invalid widget
      return nullptr;
    }
  }

  if ( dynamic_cast<QgsOptionsDialogHighlightWidget *>( widget ) )
  {
    return dynamic_cast<QgsOptionsDialogHighlightWidget *>( widget );
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
  else if ( qobject_cast<QTableView *>( widget ) )
  {
    return new QgsOptionsDialogHighlightTable( qobject_cast<QTableView *>( widget ) );
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

  if ( mEventFilter )
  {
    mWidget->removeEventFilter( mEventFilter );
    delete mEventFilter;
    mEventFilter = nullptr;
  }

  if ( !text.isEmpty() )
  {
    found = searchText( mSearchText );
  }
  else
  {
    reset();
    mChangedStyle = false;
  }

  if ( mChangedStyle )
  {
    reset();
    mChangedStyle = false;
  }

  if ( found )
  {
    if ( !mWidget->isVisible() )
    {
      mEventFilter = new QgsOptionsDialogHighlightWidgetEventFilter( this );
      mWidget->installEventFilter( mEventFilter );
    }
    else
    {
      mChangedStyle = highlightText( mSearchText );
    }
  }

  return found;
}


///@cond PRIVATE

QgsOptionsDialogHighlightWidgetEventFilter::QgsOptionsDialogHighlightWidgetEventFilter( QgsOptionsDialogHighlightWidget *highlightWidget )
  : QObject( highlightWidget->widget() )
  , mHighlightWidget( highlightWidget )
{}

bool QgsOptionsDialogHighlightWidgetEventFilter::eventFilter( QObject *obj, QEvent *event )
{
  if ( event->type() == QEvent::Show && obj == mHighlightWidget->widget() )
  {
    mHighlightWidget->widget()->removeEventFilter( this );
    // instead of catching the event and calling show again
    // it might be better to use a timer to change the style
    // after the widget is shown
#if 1
    mHighlightWidget->widget()->show();
    mHighlightWidget->mChangedStyle = mHighlightWidget->highlightText( mHighlightWidget->mSearchText );
    return true;
#else
    QTimer::singleShot( 500, this, [=] {
      mChangedStyle = highlightText( mSearchText );
    } );
#endif
  }
  return QObject::eventFilter( obj, event );
}

///@endcond
