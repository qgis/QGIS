/***************************************************************************
   qgsfieldexpressionwidget.cpp
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include <QHBoxLayout>

#include "qgsapplication.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldcombobox.h"
#include "qgsdistancearea.h"

QgsFieldExpressionWidget::QgsFieldExpressionWidget( QWidget *parent )
    : QWidget( parent )
    , mExpressionDialogTitle( tr( "Expression dialog" ) )
    , mDa( 0 )
{
  QHBoxLayout* layout = new QHBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  mCombo = new QgsFieldComboBox( this );
  mCombo->setAllowExpression( true );
  mCombo->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  layout->addWidget( mCombo );
  mButton = new QToolButton( this );
  mButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::MinimumExpanding );
  mButton->setIcon( QgsApplication::getThemeIcon( "/mIconExpressionEditorOpen.svg" ) );
  layout->addWidget( mButton );

  connect( mButton, SIGNAL( clicked() ), this, SLOT( editExpression() ) );
}

void QgsFieldExpressionWidget::setExpressionDialogTitle( QString title )
{
  mExpressionDialogTitle = title;
}

void QgsFieldExpressionWidget::setGeomCalculator( const QgsDistanceArea &da )
{
  mDa = QSharedPointer<const QgsDistanceArea>( new QgsDistanceArea( da ) );
}

QgsFieldComboBox *QgsFieldExpressionWidget::fieldComboBox()
{
  return mCombo;
}

QToolButton *QgsFieldExpressionWidget::toolButton()
{
  return mButton;
}

QString QgsFieldExpressionWidget::currentField( bool *isExpression )
{
  return mCombo->currentField( isExpression );
}

QgsVectorLayer *QgsFieldExpressionWidget::layer()
{
  return mCombo->layer();
}

void QgsFieldExpressionWidget::setLayer( QgsMapLayer *layer )
{
  mCombo->setLayer( layer );
}

void QgsFieldExpressionWidget::setField( QString fieldName )
{
  mCombo->setField( fieldName );
}

void QgsFieldExpressionWidget::editExpression()
{
  QString currentExpression = mCombo->currentField();
  QgsVectorLayer* layer = mCombo->layer();

  if ( !layer )
    return;

  QgsExpressionBuilderDialog dlg( layer, currentExpression );
  if ( !mDa.isNull() )
  {
    dlg.setGeomCalculator( *mDa );
  }
  dlg.setWindowTitle( mExpressionDialogTitle );

  if ( dlg.exec() )
  {
    QString newExpression = dlg.expressionText();
    mCombo->setField( newExpression );
  }

}
