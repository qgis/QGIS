/***************************************************************************
    qgslegendfilterbutton.h - QToolButton for legend filter by map content
     --------------------------------------
    Date                 : June 2015
    Copyright            : (C) 2015 by Hugo Mercier at Oslandia
    Email                : hugo dot mercier at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslegendfilterbutton.h"

#include <QMenu>
#include <QAction>

#include <qgsapplication.h>
#include <qgsexpressionbuilderdialog.h>

QgsLegendFilterButton::QgsLegendFilterButton( QWidget* parent )
    : QToolButton( parent )
    , mMenu( nullptr )
    , mLayer( nullptr )
{
  mMenu = new QMenu( this );
  mSetExpressionAction = new QAction( tr( "Edit filter expression" ), mMenu );
  connect( mSetExpressionAction, SIGNAL( triggered( bool ) ), this, SLOT( onSetLegendFilterExpression() ) );

  mClearExpressionAction = new QAction( tr( "Clear filter expression" ), mMenu );
  connect( mClearExpressionAction, SIGNAL( triggered( bool ) ), this, SLOT( onClearFilterExpression() ) );
  mClearExpressionAction->setEnabled( false );

  mMenu->addAction( mSetExpressionAction );
  mMenu->addAction( mClearExpressionAction );

  setCheckable( true );
  setIcon( QgsApplication::getThemeIcon( "/mIconExpressionFilter.svg" ) );
  setPopupMode( QToolButton::MenuButtonPopup );

  setMenu( mMenu );

  connect( this, SIGNAL( toggled( bool ) ), this, SLOT( onToggle( bool ) ) );
}

QgsLegendFilterButton::~QgsLegendFilterButton()
{
}

void QgsLegendFilterButton::onToggle( bool checked )
{
  if ( checked && expressionText().isEmpty() )
  {
    // show the dialog if the current expression is empty
    blockSignals( true );
    onSetLegendFilterExpression();
    blockSignals( false );
  }
}

void QgsLegendFilterButton::onSetLegendFilterExpression()
{
  QgsExpressionBuilderDialog dlg( mLayer, mExpression );
  if ( dlg.exec() )
  {
    setExpressionText( dlg.expressionText() );

    bool emitSignal = false;
    if ( !expressionText().isEmpty() )
    {
      emitSignal = isChecked();
      setChecked( true );
    }
    else
    {
      emitSignal = !isChecked();
      setChecked( false );
    }
    if ( emitSignal )
      emit toggled( isChecked() );
  }
}

void QgsLegendFilterButton::onClearFilterExpression()
{
  mClearExpressionAction->setEnabled( false );
  setExpressionText( "" );

  setChecked( false );
}

void QgsLegendFilterButton::updateMenu()
{
  if ( !mExpression.isEmpty() )
  {
    mClearExpressionAction->setEnabled( true );
    mSetExpressionAction->setText( QString( tr( "Edit filter expression (current: %1)" ) ).arg( mExpression ) );
  }
  else
  {
    mClearExpressionAction->setEnabled( false );
    mSetExpressionAction->setText( tr( "Edit filter expression" ) );
  }
}

QString QgsLegendFilterButton::expressionText() const
{
  return mExpression;
}

void QgsLegendFilterButton::setExpressionText( const QString& expression )
{
  mExpression = expression;
  updateMenu();
}

QgsVectorLayer* QgsLegendFilterButton::vectorLayer() const
{
  return mLayer;
}

void QgsLegendFilterButton::setVectorLayer( QgsVectorLayer* layer )
{
  mLayer = layer;
}
