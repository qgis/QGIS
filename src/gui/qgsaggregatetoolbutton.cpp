/***************************************************************************
    qgsaggregatetoolbutton.cpp
     --------------------------------------
    Date                 : Nov 2017
    Copyright            : (C) 2017 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaggregatetoolbutton.h"
#include "qgsaggregatecalculator.h"

#include <QMenu>

QgsAggregateToolButton::QgsAggregateToolButton()
{
  setFocusPolicy( Qt::StrongFocus );
  setPopupMode( QToolButton::InstantPopup );

  mMenu = new QMenu( this );
  connect( mMenu, &QMenu::aboutToShow, this, &QgsAggregateToolButton::aboutToShowMenu );
  setMenu( mMenu );

  setText( tr( "Exclude" ) );
}

void QgsAggregateToolButton::setType( QVariant::Type type )
{
  if ( mType == type )
    return;

  mType = type;
  updateAvailableAggregates();
}

void QgsAggregateToolButton::aboutToShowMenu()
{
  mMenu->clear();

  QAction *action = mMenu->addAction( tr( "Exclude" ) );
  connect( action, &QAction::triggered, this, [ this ]
  {
    setActive( false );
  } );

  for ( const auto &aggregate : qgis::as_const( mAvailableAggregates ) )
  {
    QAction *action = mMenu->addAction( aggregate.name );
    connect( action, &QAction::triggered, this, [ this, aggregate ]
    {
      setText( aggregate.name );
      setAggregate( aggregate.function );
    } );
  }
}

void QgsAggregateToolButton::updateAvailableAggregates()
{
  QList<QgsAggregateCalculator::AggregateInfo> aggregates = QgsAggregateCalculator::aggregates();

  for ( const auto &aggregate : aggregates )
  {
    if ( aggregate.supportedTypes.contains( mType ) )
    {
      mAvailableAggregates.append( aggregate );
    }
  }
}

void QgsAggregateToolButton::setActive( bool active )
{
  if ( active == mActive )
    return;

  mActive = active;

  if ( !active )
    setText( tr( "Exclude" ) );
  emit activeChanged();
}

QString QgsAggregateToolButton::aggregate() const
{
  return mAggregate;
}

void QgsAggregateToolButton::setAggregate( const QString &aggregate )
{
  if ( aggregate == mAggregate )
    return;

  mAggregate = QString();

  for ( const auto &agg : qgis::as_const( mAvailableAggregates ) )
  {
    if ( agg.function == aggregate )
    {
      mAggregate = aggregate;
      setText( agg.name );
      break;
    }
  }

  setActive( !mAggregate.isEmpty() );

  emit aggregateChanged();
}

bool QgsAggregateToolButton::active() const
{
  return mActive;
}

QVariant::Type QgsAggregateToolButton::type() const
{
  return mType;
}
