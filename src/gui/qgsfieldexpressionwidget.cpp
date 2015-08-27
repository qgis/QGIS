
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
    , mExpressionContextCallback( 0 )
    , mExpressionContextCallbackContext( 0 )
{
  QHBoxLayout* layout = new QHBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  mCombo = new QComboBox( this );
  mCombo->setEditable( true );
  mCombo->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  int width = mCombo->minimumSizeHint().width();
  mCombo->setMinimumWidth( width );
  mFieldProxyModel = new QgsFieldProxyModel( mCombo );
  mFieldProxyModel->sourceFieldModel()->setAllowExpression( true );
  mCombo->setModel( mFieldProxyModel );

  mButton = new QToolButton( this );
  mButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mButton->setIcon( QgsApplication::getThemeIcon( "/mIconExpression.svg" ) );

  layout->addWidget( mCombo );
  layout->addWidget( mButton );

  // give focus to the combo
  // hence if the widget is used as a delegate
  // it will allow pressing on the expression dialog button
  setFocusProxy( mCombo );

  connect( mCombo->lineEdit(), SIGNAL( textEdited( QString ) ), this, SLOT( expressionEdited( QString ) ) );
  connect( mCombo->lineEdit(), SIGNAL( editingFinished() ), this, SLOT( expressionEditingFinished() ) );
  connect( mCombo, SIGNAL( activated( int ) ), this, SLOT( currentFieldChanged() ) );
  connect( mButton, SIGNAL( clicked() ), this, SLOT( editExpression() ) );
  // NW TODO - Fix in 2.6
//  connect( mCombo->lineEdit(), SIGNAL( returnPressed() ), this, SIGNAL( returnPressed() ) );

  mExpressionContext.reset( new QgsExpressionContext() );
  mExpressionContext->appendScope( QgsExpressionContextUtils::globalScope() );
  mExpressionContext->appendScope( QgsExpressionContextUtils::projectScope() );
}

void QgsFieldExpressionWidget::setExpressionDialogTitle( QString title )
{
  mExpressionDialogTitle = title;
}

void QgsFieldExpressionWidget::setFilters( QgsFieldProxyModel::Filters filters )
{
  mFieldProxyModel->setFilters( filters );
}

void QgsFieldExpressionWidget::setLeftHandButtonStyle( bool isLeft )
{
  QHBoxLayout* layout = dynamic_cast<QHBoxLayout*>( this->layout() );
  if ( !layout )
    return;

  if ( isLeft )
  {
    QLayoutItem* item = layout->takeAt( 1 );
    layout->insertWidget( 0, item->widget() );
  }
  else
    layout->addWidget( mCombo );
}

void QgsFieldExpressionWidget::setGeomCalculator( const QgsDistanceArea &da )
{
  mDa = QSharedPointer<const QgsDistanceArea>( new QgsDistanceArea( da ) );
}

QString QgsFieldExpressionWidget::currentText() const
{
  return mCombo->currentText();
}

bool QgsFieldExpressionWidget::isValidExpression( QString *expressionError ) const
{
  QString temp;
  return QgsExpression::isValid( currentText(), mExpressionContext.data(), expressionError ? *expressionError : temp );
}

bool QgsFieldExpressionWidget::isExpression() const
{
  return !mFieldProxyModel->sourceFieldModel()->isField( currentText() );
}

QString QgsFieldExpressionWidget::currentField( bool *isExpression, bool *isValid ) const
{
  QString text = currentText();
  if ( isValid )
  {
    *isValid = isValidExpression();
  }
  if ( isExpression )
  {
    *isExpression = this->isExpression();
  }
  return text;
}

QgsVectorLayer *QgsFieldExpressionWidget::layer() const
{
  return mFieldProxyModel->sourceFieldModel()->layer();
}

void QgsFieldExpressionWidget::registerGetExpressionContextCallback( QgsFieldExpressionWidget::ExpressionContextCallback fnGetExpressionContext, const void *context )
{
  mExpressionContextCallback = fnGetExpressionContext;
  mExpressionContextCallbackContext = context;
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
  mExpressionContext.reset( new QgsExpressionContext() );
  mExpressionContext->appendScope( QgsExpressionContextUtils::globalScope() );
  mExpressionContext->appendScope( QgsExpressionContextUtils::projectScope() );
  if ( layer )
    mExpressionContext->appendScope( QgsExpressionContextUtils::layerScope( layer ) );

  mFieldProxyModel->sourceFieldModel()->setLayer( layer );
}

void QgsFieldExpressionWidget::setField( const QString &fieldName )
{
  if ( fieldName.isEmpty() )
    return;

  QModelIndex idx = mFieldProxyModel->sourceFieldModel()->indexFromName( fieldName );
  if ( !idx.isValid() )
  {
    // try to remove quotes and white spaces
    QString simpleFieldName = fieldName.trimmed();
    if ( simpleFieldName.startsWith( '"' ) && simpleFieldName.endsWith( '"' ) )
    {
      simpleFieldName.remove( 0, 1 ).chop( 1 );
      idx = mFieldProxyModel->sourceFieldModel()->indexFromName( simpleFieldName );
    }

    if ( !idx.isValid() )
    {
      // new expression
      mFieldProxyModel->sourceFieldModel()->setExpression( fieldName );
      idx = mFieldProxyModel->sourceFieldModel()->indexFromName( fieldName );
    }
  }
  QModelIndex proxyIndex = mFieldProxyModel->mapFromSource( idx );
  mCombo->setCurrentIndex( proxyIndex.row() );
  currentFieldChanged();
}

void QgsFieldExpressionWidget::editExpression()
{
  QString currentExpression = currentText();
  QgsVectorLayer* vl = layer();

  QgsExpressionContext context = mExpressionContextCallback ? mExpressionContextCallback( mExpressionContextCallbackContext ) : *mExpressionContext;

  QgsExpressionBuilderDialog dlg( vl, currentExpression, this, "generic", context );
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
  emit fieldChanged( expression, isValidExpression() );
}

void QgsFieldExpressionWidget::expressionEditingFinished()
{
  QgsDebugMsg( "Editing finished" );
  const QString expression = mCombo->lineEdit()->text();
  mFieldProxyModel->sourceFieldModel()->setExpression( expression );
  QModelIndex idx = mFieldProxyModel->sourceFieldModel()->indexFromName( expression );
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

void QgsFieldExpressionWidget::currentFieldChanged()
{
  updateLineEditStyle();

  bool isExpression, isValid;
  QString fieldName = currentField( &isExpression, &isValid );

  // display tooltip if widget is shorter than expression
  QFontMetrics metrics( mCombo->lineEdit()->font() );
  if ( metrics.width( fieldName ) > mCombo->lineEdit()->width() )
  {
    mCombo->setToolTip( fieldName );
  }
  else
  {
    mCombo->setToolTip( "" );
  }

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
  QgsExpression expression( expressionStr );
  expression.prepare( mExpressionContext.data() );
  return !expression.hasParserError();
}
