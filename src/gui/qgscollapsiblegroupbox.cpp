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

#include <QStyleOptionGroupBox>
#include <QStylePainter>
#include <QLayout>
#include <QProxyStyle>

class QgsCollapsibleGroupBoxStyle : public QProxyStyle
{
  public:
    QgsCollapsibleGroupBoxStyle( QStyle * style = 0 ) : QProxyStyle( style ) {}

    void drawPrimitive( PrimitiveElement element, const QStyleOption *option,
                        QPainter *painter, const QWidget *widget ) const
    {
      if ( element == PE_IndicatorCheckBox )
      {
        const QgsCollapsibleGroupBox* groupBox =
          dynamic_cast<const QgsCollapsibleGroupBox*>( widget );
        if ( groupBox )
        {
          return drawPrimitive( groupBox->isCollapsed() ?
                                PE_IndicatorArrowRight : PE_IndicatorArrowDown,
                                option, painter, widget );
        }
      }
      return QProxyStyle::drawPrimitive( element, option, painter, widget );
    }
};

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
  setStyle( new QgsCollapsibleGroupBoxStyle( QApplication::style() ) );
  connect( this, SIGNAL( toggled( bool ) ), this, SLOT( setToggled( bool ) ) );
}

void QgsCollapsibleGroupBox::showEvent( QShowEvent * event )
{
  QGroupBox::showEvent( event );
  // collapse if needed - any calls to setCollapsed() before have no effect
  if ( isCheckable() && ! isChecked() && ! isCollapsed() )
    setCollapsed( true );
}

void QgsCollapsibleGroupBox::setCollapsed( bool collapse )
{
  if ( ! isVisible() )
    return;

  mCollapsed = collapse;

  // minimize layout margins and save for subsequent restore
  if ( collapse )
  {
    if ( layout() )
    {
      mMargins = layout()->contentsMargins();
      layout()->setContentsMargins( 1, 1, 1, 1 );
    }
  }
  else
  {
    if ( layout() )
    {
      layout()->setContentsMargins( mMargins );
    }
  }

  // if we are collapsing, save hidden widgets in a list
  if ( collapse )
  {
    mHiddenWidgets.clear();
    foreach ( QWidget *widget, findChildren<QWidget*>() )
    {
      if ( widget->isHidden() )
        mHiddenWidgets << widget;
    }
  }

  // show/hide widgets
  foreach ( QWidget *widget, findChildren<QWidget*>() )
    widget->setHidden( collapse );

  // if we are expanding, re-hide saved hidden widgets
  if ( ! collapse )
  {
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

