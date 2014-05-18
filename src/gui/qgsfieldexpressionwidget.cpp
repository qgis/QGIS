
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
#include "qgsfieldproxymodel.h"
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
  mFieldProxyModel = new QgsFieldProxyModel( mCombo );
  mFieldProxyModel->sourceFieldModel()->setAllowExpression( true );
  mCombo->setModel( mFieldProxyModel );

  mButton = new QToolButton( this );
  mButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mButton->setIcon( QgsApplication::getThemeIcon( "/mIconExpressionEditorOpen.svg" ) );

  layout->addWidget( mCombo );
  layout->addWidget( mButton );

  // give focus to the combo
  // hence if the widget is used as a delegate
  // it will allow pressing on the expression dialog button
  setFocusProxy( mCombo );

  connect( mCombo->lineEdit(), SIGNAL( textEdited( QString ) ), this, SLOT( expressionEdited( QString ) ) );
  connect( mCombo->lineEdit(), SIGNAL( editingFinished() ), this, SLOT( expressionEditingFinished() ) );
  connect( mCombo, SIGNAL( activated( int ) ), this, SLOT( currentFieldChanged( int ) ) );
  connect( mButton, SIGNAL( clicked() ), this, SLOT( editExpression() ) );
}

void QgsFieldExpressionWidget::setExpressionDialogTitle( QString title )
{
  mExpressionDialogTitle = title;
}

void QgsFieldExpressionWidget::setFilters( QgsFieldProxyModel::Filters filters )
{
  mFieldProxyModel->setFilters( filters );
}

void QgsFieldExpressionWidget::setGeomCalculator( const QgsDistanceArea &da )
{
  mDa = QSharedPointer<const QgsDistanceArea>( new QgsDistanceArea( da ) );
}

QString QgsFieldExpressionWidget::currentField( bool *isExpression , bool *isValid )
{
  if ( isExpression )
  {
    *isExpression = false;
  }
  if ( isValid )
  {
    *isValid = true;
  }

  int i = mCombo->currentIndex();
  const QModelIndex proxyIndex = mFieldProxyModel->index( i, 0 );
  if ( !proxyIndex.isValid() )
    return "";

  if ( isExpression )
  {
    *isExpression = mFieldProxyModel->data( proxyIndex, QgsFieldModel::IsExpressionRole ).toBool();
  }
  if ( isValid )
  {
    *isValid = mFieldProxyModel->data( proxyIndex, QgsFieldModel::ExpressionValidityRole ).toBool();
  }
  QString expression = mFieldProxyModel->data( proxyIndex, QgsFieldModel::ExpressionRole ).toString();
  return expression;
}

QgsVectorLayer *QgsFieldExpressionWidget::layer()
{
  return mFieldProxyModel->sourceFieldModel()->layer();
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
  mFieldProxyModel->sourceFieldModel()->setLayer( layer );
}

void QgsFieldExpressionWidget::setField( const QString &fieldName )
{
  if ( fieldName.isEmpty() )
    return;

  QModelIndex idx = mFieldProxyModel->sourceFieldModel()->indexFromName( fieldName );
  if ( !idx.isValid() )
  {
    // new expression
    idx = mFieldProxyModel->sourceFieldModel()->setExpression( fieldName );
  }
  QModelIndex proxyIndex = mFieldProxyModel->mapFromSource( idx );
  mCombo->setCurrentIndex( proxyIndex.row() );
  currentFieldChanged();
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

void QgsFieldExpressionWidget::expressionEdited( const QString expression )
{
  updateLineEditStyle( expression );
}

void QgsFieldExpressionWidget::expressionEditingFinished()
{
  const QString expression = mCombo->lineEdit()->text();
  QModelIndex idx = mFieldProxyModel->sourceFieldModel()->setExpression( expression );
  QModelIndex proxyIndex = mFieldProxyModel->mapFromSource( idx );
  mCombo->setCurrentIndex( proxyIndex.row() );
  currentFieldChanged();
}

void QgsFieldExpressionWidget::changeEvent( QEvent* event )
{
  if ( event->type() == QEvent::EnabledChange )
  {
    updateLineEditStyle();
  }
}

void QgsFieldExpressionWidget::currentFieldChanged( int i /* =0 */ )
{
  Q_UNUSED( i );

  updateLineEditStyle();

  bool isExpression, isValid;
  QString fieldName = currentField( &isExpression, &isValid );
  emit fieldChanged( fieldName );
  emit fieldChanged( fieldName, isValid );
}

void QgsFieldExpressionWidget::updateLineEditStyle( const QString expression )
{
  QPalette palette;
  if ( !isEnabled() )
  {
    palette.setColor( QPalette::Text, Qt::gray );
  }
  else
  {
    bool isExpression, isValid;
    if ( !expression.isEmpty() )
    {
      isExpression = true;
      isValid = isExpressionValid( expression );
    }
    else
    {
      currentField( &isExpression, &isValid );
    }
    QFont font = mCombo->lineEdit()->font();
    font.setItalic( isExpression );
    mCombo->lineEdit()->setFont( font );

    if ( isExpression && !isValid )
    {
      palette.setColor( QPalette::Text, Qt::red );
    }
    else
    {
      palette.setColor( QPalette::Text, Qt::black );
    }
  }
  mCombo->lineEdit()->setPalette( palette );
}

bool QgsFieldExpressionWidget::isExpressionValid( const QString expressionStr )
{
  QgsVectorLayer* vl = layer();
  if ( !vl )
    return false;

  QgsExpression expression( expressionStr );
  expression.prepare( vl->pendingFields() );
  return !expression.hasParserError();
}
