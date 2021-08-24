
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
#include <QObject>
#include <QKeyEvent>

#include "qgsapplication.h"
#include "qgsfieldexpressionwidget.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldproxymodel.h"
#include "qgsdistancearea.h"
#include "qgsfieldmodel.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsexpressioncontextutils.h"
#include "qgsexpressioncontextgenerator.h"

QgsFieldExpressionWidget::QgsFieldExpressionWidget( QWidget *parent )
  : QWidget( parent )
  , mExpressionDialogTitle( tr( "Expression Dialog" ) )
  , mDistanceArea( nullptr )

{
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );

  mCombo = new QComboBox( this );
  mCombo->setEditable( true );
  mCombo->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
  const int width = mCombo->minimumSizeHint().width();
  mCombo->setMinimumWidth( width );

  mFieldProxyModel = new QgsFieldProxyModel( mCombo );
  mFieldProxyModel->sourceFieldModel()->setAllowExpression( true );
  mCombo->setModel( mFieldProxyModel );

  mButton = new QToolButton( this );
  mButton->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  mButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mIconExpression.svg" ) ) );

  layout->addWidget( mCombo );
  layout->addWidget( mButton );

  // give focus to the combo
  // hence if the widget is used as a delegate
  // it will allow pressing on the expression dialog button
  setFocusProxy( mCombo );

  connect( mCombo->lineEdit(), &QLineEdit::textEdited, this, &QgsFieldExpressionWidget::expressionEdited );
  connect( mCombo->lineEdit(), &QLineEdit::editingFinished, this, &QgsFieldExpressionWidget::expressionEditingFinished );
  connect( mCombo, static_cast < void ( QComboBox::* )( int ) > ( &QComboBox::activated ), this, &QgsFieldExpressionWidget::currentFieldChanged );
  connect( mButton, &QAbstractButton::clicked, this, &QgsFieldExpressionWidget::editExpression );
  connect( mFieldProxyModel, &QAbstractItemModel::modelAboutToBeReset, this, &QgsFieldExpressionWidget::beforeResetModel );
  connect( mFieldProxyModel, &QAbstractItemModel::modelReset, this, &QgsFieldExpressionWidget::afterResetModel );

  mExpressionContext = QgsExpressionContext();
  mExpressionContext << QgsExpressionContextUtils::globalScope()
                     << QgsExpressionContextUtils::projectScope( QgsProject::instance() );

  mCombo->installEventFilter( this );
}

void QgsFieldExpressionWidget::setExpressionDialogTitle( const QString &title )
{
  mExpressionDialogTitle = title;
}

void QgsFieldExpressionWidget::setFilters( QgsFieldProxyModel::Filters filters )
{
  mFieldProxyModel->setFilters( filters );
}

void QgsFieldExpressionWidget::setAllowEmptyFieldName( bool allowEmpty )
{
  mCombo->lineEdit()->setClearButtonEnabled( allowEmpty );
  mFieldProxyModel->sourceFieldModel()->setAllowEmptyFieldName( allowEmpty );
}

bool QgsFieldExpressionWidget::allowEmptyFieldName() const
{
  return mFieldProxyModel->sourceFieldModel()->allowEmptyFieldName();
}

void QgsFieldExpressionWidget::setLeftHandButtonStyle( bool isLeft )
{
  QHBoxLayout *layout = dynamic_cast<QHBoxLayout *>( this->layout() );
  if ( !layout )
    return;

  if ( isLeft )
  {
    QLayoutItem *item = layout->takeAt( 1 );
    layout->insertWidget( 0, item->widget() );
  }
  else
    layout->addWidget( mCombo );
}

void QgsFieldExpressionWidget::setGeomCalculator( const QgsDistanceArea &da )
{
  mDistanceArea = std::shared_ptr<const QgsDistanceArea>( new QgsDistanceArea( da ) );
}

QString QgsFieldExpressionWidget::currentText() const
{
  return mCombo->currentText();
}

QString QgsFieldExpressionWidget::asExpression() const
{
  return isExpression() ? currentText() : QgsExpression::quotedColumnRef( currentText() );
}

QString QgsFieldExpressionWidget::expression() const
{
  return asExpression();
}

bool QgsFieldExpressionWidget::isValidExpression( QString *expressionError ) const
{
  QString temp;
  return QgsExpression::checkExpression( currentText(), &mExpressionContext, expressionError ? *expressionError : temp );
}

bool QgsFieldExpressionWidget::isExpression() const
{
  return !mFieldProxyModel->sourceFieldModel()->isField( currentText() );
}

QString QgsFieldExpressionWidget::currentField( bool *isExpression, bool *isValid ) const
{
  QString text = currentText();
  const bool valueIsExpression = this->isExpression();
  if ( isValid )
  {
    // valid if not an expression (ie, set to a field), or set to an expression and expression is valid
    *isValid = !valueIsExpression || isValidExpression();
  }
  if ( isExpression )
  {
    *isExpression = valueIsExpression;
  }
  return text;
}

QgsVectorLayer *QgsFieldExpressionWidget::layer() const
{
  return mFieldProxyModel->sourceFieldModel()->layer();
}

void QgsFieldExpressionWidget::registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator )
{
  mExpressionContextGenerator = generator;
}

void QgsFieldExpressionWidget::setLayer( QgsMapLayer *layer )
{
  QgsVectorLayer *vl = qobject_cast< QgsVectorLayer * >( layer );

  if ( mFieldProxyModel->sourceFieldModel()->layer() )
    disconnect( mFieldProxyModel->sourceFieldModel()->layer(), &QgsVectorLayer::updatedFields, this, &QgsFieldExpressionWidget::reloadLayer );

  if ( vl )
    mExpressionContext = vl->createExpressionContext();
  else
    mExpressionContext = QgsProject::instance()->createExpressionContext();

  mFieldProxyModel->sourceFieldModel()->setLayer( vl );

  if ( mFieldProxyModel->sourceFieldModel()->layer() )
    connect( mFieldProxyModel->sourceFieldModel()->layer(), &QgsVectorLayer::updatedFields, this, &QgsFieldExpressionWidget::reloadLayer, Qt::UniqueConnection );
}

void QgsFieldExpressionWidget::setField( const QString &fieldName )
{
  if ( fieldName.isEmpty() )
  {
    setRow( -1 );
    emit fieldChanged( QString() );
    emit fieldChanged( QString(), true );
    return;
  }

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
  const QModelIndex proxyIndex = mFieldProxyModel->mapFromSource( idx );
  mCombo->setCurrentIndex( proxyIndex.row() );
  currentFieldChanged();
}

void QgsFieldExpressionWidget::setFields( const QgsFields &fields )
{
  mFieldProxyModel->sourceFieldModel()->setFields( fields );
}

void QgsFieldExpressionWidget::setExpression( const QString &expression )
{
  setField( expression );
}

void QgsFieldExpressionWidget::editExpression()
{
  const QString currentExpression = asExpression();
  QgsVectorLayer *vl = layer();

  const QgsExpressionContext context = mExpressionContextGenerator ? mExpressionContextGenerator->createExpressionContext() : mExpressionContext;

  QgsExpressionBuilderDialog dlg( vl, currentExpression, this, QStringLiteral( "generic" ), context );
  if ( mDistanceArea )
  {
    dlg.setGeomCalculator( *mDistanceArea );
  }
  dlg.setWindowTitle( mExpressionDialogTitle );
  dlg.setAllowEvalErrors( mAllowEvalErrors );

  if ( !vl )
    dlg.expressionBuilder()->expressionTree()->loadFieldNames( mFieldProxyModel->sourceFieldModel()->fields() );

  if ( dlg.exec() )
  {
    const QString newExpression = dlg.expressionText();
    setField( newExpression );
  }
}

void QgsFieldExpressionWidget::expressionEdited( const QString &expression )
{
  updateLineEditStyle( expression );
  emit fieldChanged( expression, isValidExpression() );
}

void QgsFieldExpressionWidget::expressionEditingFinished()
{
  const QString expression = mCombo->lineEdit()->text();
  mFieldProxyModel->sourceFieldModel()->setExpression( expression );
  const QModelIndex idx = mFieldProxyModel->sourceFieldModel()->indexFromName( expression );
  const QModelIndex proxyIndex = mFieldProxyModel->mapFromSource( idx );
  mCombo->setCurrentIndex( proxyIndex.row() );
  currentFieldChanged();
}

void QgsFieldExpressionWidget::changeEvent( QEvent *event )
{
  if ( event->type() == QEvent::EnabledChange )
  {
    updateLineEditStyle();
  }
}

void QgsFieldExpressionWidget::reloadLayer()
{
  setLayer( mFieldProxyModel->sourceFieldModel()->layer() );
}

void QgsFieldExpressionWidget::beforeResetModel()
{
  // Backup expression
  mBackupExpression = mCombo->currentText();
}

void QgsFieldExpressionWidget::afterResetModel()
{
  // Restore expression
  mCombo->lineEdit()->setText( mBackupExpression );
}

bool QgsFieldExpressionWidget::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == mCombo && event->type() == QEvent::KeyPress )
  {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>( event );
    if ( keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return )
    {
      expressionEditingFinished();
      return true;
    }
  }
  return QObject::eventFilter( watched, event );
}

bool QgsFieldExpressionWidget::allowEvalErrors() const
{
  return mAllowEvalErrors;
}

void QgsFieldExpressionWidget::setAllowEvalErrors( bool allowEvalErrors )
{
  if ( allowEvalErrors == mAllowEvalErrors )
    return;

  mAllowEvalErrors = allowEvalErrors;
  emit allowEvalErrorsChanged();
}

void QgsFieldExpressionWidget::currentFieldChanged()
{
  updateLineEditStyle();

  bool isExpression, isValid;
  const QString fieldName = currentField( &isExpression, &isValid );

  // display tooltip if widget is shorter than expression
  const QFontMetrics metrics( mCombo->lineEdit()->font() );
  if ( metrics.boundingRect( fieldName ).width() > mCombo->lineEdit()->width() )
  {
    mCombo->setToolTip( fieldName );
  }
  else
  {
    mCombo->setToolTip( QString() );
  }

  emit fieldChanged( fieldName );
  emit fieldChanged( fieldName, isValid );
}

void QgsFieldExpressionWidget::updateLineEditStyle( const QString &expression )
{
  QString stylesheet;
  if ( !isEnabled() )
  {
    stylesheet = QStringLiteral( "QLineEdit { color: %1; }" ).arg( QColor( Qt::gray ).name() );
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
      stylesheet = QStringLiteral( "QLineEdit { color: %1; }" ).arg( QColor( Qt::red ).name() );
    }
  }
  mCombo->lineEdit()->setStyleSheet( stylesheet );
}

bool QgsFieldExpressionWidget::isExpressionValid( const QString &expressionStr )
{
  QgsExpression expression( expressionStr );
  expression.prepare( &mExpressionContext );
  return !expression.hasParserError();
}

void QgsFieldExpressionWidget::appendScope( QgsExpressionContextScope *scope )
{
  mExpressionContext.appendScope( scope );
}
