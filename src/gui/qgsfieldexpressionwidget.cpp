
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
#include "qgsfieldmodel.h"
#include "qgsdistancearea.h"

QgsFieldExpressionWidget::QgsFieldExpressionWidget( QWidget *parent )
    : QWidget( parent )
    , mExpressionDialogTitle( tr( "Expression dialog" ) )
    , mDa( 0 )
{
  QHBoxLayout* layout = new QHBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  mCombo = new QComboBox( this );
  mCombo->setEditable( true );
  mCombo->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  mFieldModel = new QgsFieldModel( mCombo );
  mFieldModel->setAllowExpression( true );
  mCombo->setModel( mFieldModel );

  mButton = new QToolButton( this );
  mButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mButton->setIcon( QgsApplication::getThemeIcon( "/mIconExpressionEditorOpen.svg" ) );

  layout->addWidget( mCombo );
  layout->addWidget( mButton );

  connect( mCombo->lineEdit(), SIGNAL( textEdited( QString ) ), this, SLOT( expressionEdited( QString ) ) );
  connect( mCombo, SIGNAL( currentIndexChanged( int ) ), this, SLOT( indexChanged( int ) ) );
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

QString QgsFieldExpressionWidget::currentField( bool *isExpression )
{
  if ( isExpression )
  {
    *isExpression = false;
  }

  int i = mCombo->currentIndex();

  const QModelIndex index = mFieldModel->index( i, 0 );
  if ( !index.isValid() )
  {
    return "";
  }

  if ( isExpression )
  {
    *isExpression = mFieldModel->data( index, QgsFieldModel::IsExpressionRole ).toBool();
  }
  QString expression = mFieldModel->data( index, QgsFieldModel::ExpressionRole ).toString();
  return expression;
}

QgsVectorLayer *QgsFieldExpressionWidget::layer()
{
  return mFieldModel->layer();
}

void QgsFieldExpressionWidget::setLayer( QgsMapLayer *layer )
{
  QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer );
  if ( vl )
  {
    setLayer( vl );
  }
}

void QgsFieldExpressionWidget::setLayer( QgsVectorLayer *layer )
{
  mFieldModel->setLayer( layer );
}

void QgsFieldExpressionWidget::setField( QString fieldName )
{
  if ( fieldName.isEmpty() )
    return;

  QModelIndex idx = mFieldModel->indexFromName( fieldName );
  if ( !idx.isValid() )
  {
    // new expression
    idx = mFieldModel->setExpression( fieldName );
  }
  mCombo->setCurrentIndex( idx.row() );
}

void QgsFieldExpressionWidget::editExpression()
{
  QString currentExpression = currentField();
  QgsVectorLayer* vl = layer();

  if ( !vl )
    return;

  QgsExpressionBuilderDialog dlg( vl, currentExpression );
  if ( !mDa.isNull() )
  {
    dlg.setGeomCalculator( *mDa );
  }
  dlg.setWindowTitle( mExpressionDialogTitle );

  if ( dlg.exec() )
  {
    QString newExpression = dlg.expressionText();
    setField( newExpression );
  }
}

void QgsFieldExpressionWidget::expressionEdited( QString expression )
{
  mFieldModel->removeExpression();
  setField( expression );
}

void QgsFieldExpressionWidget::indexChanged( int i )
{
  Q_UNUSED( i );
  bool isExpression;
  QString fieldName = currentField( &isExpression );
  bool isValid = true;

  QFont font = mCombo->lineEdit()->font();
  font.setItalic( isExpression );
  mCombo->lineEdit()->setFont( font );

  QPalette palette;
  palette.setColor( QPalette::Text, Qt::black );
  if ( isExpression )
  {
    QModelIndex idx = mFieldModel->indexFromName( fieldName );
    if ( idx.isValid() )
    {
      isValid = mFieldModel->data( idx, QgsFieldModel::ExpressionValidityRole ).toBool();
      if ( !isValid )
      {
        palette.setColor( QPalette::Text, Qt::red );
      }
    }
  }
  mCombo->lineEdit()->setPalette( palette );

  emit fieldChanged( fieldName );
  emit fieldChanged( fieldName, isValid );
}
