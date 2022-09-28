/***************************************************************************
                         qgsattributetypedialog.cpp  -  description
                             -------------------
    begin                : June 2009
    copyright            : (C) 2000 by Richard Kostecky
    email                : cSf.Kostej@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributetypedialog.h"
#include "qgsattributeeditorelement.h"
#include "qgsattributetypeloaddialog.h"
#include "qgsvectordataprovider.h"
#include "qgsmapcanvas.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"
#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsexpressioncontextutils.h"

#include <QTableWidgetItem>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QScrollBar>

#include <climits>
#include <cfloat>

QgsAttributeTypeDialog::QgsAttributeTypeDialog( QgsVectorLayer *vl, int fieldIdx, QWidget *parent )
  : QWidget( parent )
  , mLayer( vl )
  , mFieldIdx( fieldIdx )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  if ( fieldIdx < 0 )
    return;

  QMapIterator<QString, QgsEditorWidgetFactory *> it( QgsGui::editorWidgetRegistry()->factories() );
  QStandardItemModel *widgetTypeModel = qobject_cast<QStandardItemModel *>( mWidgetTypeComboBox->model() );
  while ( it.hasNext() )
  {
    it.next();
    mWidgetTypeComboBox->addItem( it.value()->name(), it.key() );
    QStandardItem *item = widgetTypeModel->item( mWidgetTypeComboBox->count() - 1 );
    if ( !it.value()->supportsField( vl, fieldIdx ) )
      item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
  }

  connect( mWidgetTypeComboBox, static_cast< void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsAttributeTypeDialog::onCurrentWidgetChanged );

  if ( vl->fields().fieldOrigin( fieldIdx ) == QgsFields::OriginJoin ||
       vl->fields().fieldOrigin( fieldIdx ) == QgsFields::OriginExpression )
  {
    isFieldEditableCheckBox->setEnabled( false );
  }

  mExpressionWidget->registerExpressionContextGenerator( this );
  mExpressionWidget->setLayer( mLayer );

  mAliasExpressionButton->registerExpressionContextGenerator( this );
  connect( mAliasExpressionButton, &QgsPropertyOverrideButton::changed, this, [ = ]
  {
    mDataDefinedProperties.setProperty( QgsEditFormConfig::DataDefinedProperty::Alias, mAliasExpressionButton->toProperty() );
  } );

  connect( mExpressionWidget, &QgsExpressionLineEdit::expressionChanged, this, &QgsAttributeTypeDialog::defaultExpressionChanged );
  connect( mUniqueCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mCheckBoxEnforceUnique->setEnabled( checked );
    if ( !checked )
      mCheckBoxEnforceUnique->setChecked( false );
  } );
  connect( notNullCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mCheckBoxEnforceNotNull->setEnabled( checked );
    if ( !checked )
      mCheckBoxEnforceNotNull->setChecked( false );
  } );

  mWarnDefaultValueHasFieldsWidget->setVisible( false );
  connect( mApplyDefaultValueOnUpdateCheckBox, &QCheckBox::stateChanged, this, &QgsAttributeTypeDialog::defaultExpressionChanged );

  constraintExpressionWidget->setAllowEmptyFieldName( true );
  constraintExpressionWidget->setLayer( vl );

}

QgsAttributeTypeDialog::~QgsAttributeTypeDialog()
{
  qDeleteAll( mEditorConfigWidgets );
}

const QString QgsAttributeTypeDialog::editorWidgetType()
{
  QStandardItem *item = currentItem();
  if ( item )
  {
    return item->data( Qt::UserRole ).toString();
  }
  else
  {
    return QString();
  }
}

const QString QgsAttributeTypeDialog::editorWidgetText()
{
  QStandardItem *item = currentItem();
  if ( item )
  {
    return item->text();
  }
  else
  {
    return QString();
  }
}

const QVariantMap QgsAttributeTypeDialog::editorWidgetConfig()
{
  QStandardItem *item = currentItem();
  if ( item )
  {
    const QString widgetType = item->data( Qt::UserRole ).toString();
    QgsEditorConfigWidget *cfgWdg = mEditorConfigWidgets.value( widgetType );
    if ( cfgWdg )
    {
      return cfgWdg->config();
    }
  }

  return QVariantMap();
}

void QgsAttributeTypeDialog::setEditorWidgetType( const QString &type )
{

  mWidgetTypeComboBox->setCurrentIndex( mWidgetTypeComboBox->findData( type ) );

  if ( mEditorConfigWidgets.contains( type ) && mEditorConfigWidgets.value( type ) /* may be a null pointer */ )
  {
    stackedWidget->setCurrentWidget( mEditorConfigWidgets[type] );
  }
  else
  {
    QgsEditorConfigWidget *cfgWdg = QgsGui::editorWidgetRegistry()->createConfigWidget( type, mLayer, mFieldIdx, this );

    if ( cfgWdg )
    {
      cfgWdg->setConfig( mWidgetConfig );

      stackedWidget->addWidget( cfgWdg );
      stackedWidget->setCurrentWidget( cfgWdg );
      mEditorConfigWidgets.insert( type, cfgWdg );
      connect( cfgWdg, &QgsEditorConfigWidget::changed, this, &QgsAttributeTypeDialog::defaultExpressionChanged );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Oops, couldn't create editor widget config dialog..." ) );
    }
  }

  //update default expression preview
  defaultExpressionChanged();
}

void QgsAttributeTypeDialog::setEditorWidgetConfig( const QVariantMap &config )
{
  mWidgetConfig = config;
}

bool QgsAttributeTypeDialog::fieldEditable() const
{
  return isFieldEditableCheckBox->isChecked();
}

void QgsAttributeTypeDialog::setProviderConstraints( QgsFieldConstraints::Constraints constraints )
{
  if ( constraints & QgsFieldConstraints::ConstraintNotNull )
  {
    notNullCheckBox->setChecked( true );
    notNullCheckBox->setEnabled( false );
    notNullCheckBox->setToolTip( tr( "The provider for this layer has a NOT NULL constraint set on the field." ) );
  }

  if ( constraints & QgsFieldConstraints::ConstraintUnique )
  {
    mUniqueCheckBox->setChecked( true );
    mUniqueCheckBox->setEnabled( false );
    mUniqueCheckBox->setToolTip( tr( "The provider for this layer has a UNIQUE constraint set on the field." ) );
  }
}

void QgsAttributeTypeDialog::setNotNull( bool notNull )
{
  notNullCheckBox->setChecked( notNull );
}

bool QgsAttributeTypeDialog::labelOnTop() const
{
  return labelOnTopCheckBox->isChecked();
}

void QgsAttributeTypeDialog::setReuseLastValues( bool reuse )
{
  reuseLastValuesCheckBox->setChecked( reuse );
}

bool QgsAttributeTypeDialog::reuseLastValues() const
{
  return reuseLastValuesCheckBox->isChecked();
}

void QgsAttributeTypeDialog::setConstraintExpressionDescription( const QString &desc )
{
  leConstraintExpressionDescription->setText( desc );
}

QString QgsAttributeTypeDialog::constraintExpressionDescription()
{
  return leConstraintExpressionDescription->text();
}

bool QgsAttributeTypeDialog::notNull() const
{
  return notNullCheckBox->isEnabled() && notNullCheckBox->isChecked();
}

bool QgsAttributeTypeDialog::notNullFromProvider() const
{
  return ( !notNullCheckBox->isEnabled() ) && notNullCheckBox->isChecked();
}

void QgsAttributeTypeDialog::setNotNullEnforced( bool enforced )
{
  mCheckBoxEnforceNotNull->setChecked( enforced );
}

bool QgsAttributeTypeDialog::notNullEnforced() const
{
  return mCheckBoxEnforceNotNull->isChecked();
}

void QgsAttributeTypeDialog::setUnique( bool unique )
{
  mUniqueCheckBox->setChecked( unique );
}

bool QgsAttributeTypeDialog::unique() const
{
  return mUniqueCheckBox->isEnabled() && mUniqueCheckBox->isChecked();
}

bool QgsAttributeTypeDialog::uniqueFromProvider() const
{
  return ( !mUniqueCheckBox->isEnabled() ) && mUniqueCheckBox->isChecked();
}

void QgsAttributeTypeDialog::setUniqueEnforced( bool enforced )
{
  mCheckBoxEnforceUnique->setChecked( enforced );
}

bool QgsAttributeTypeDialog::uniqueEnforced() const
{
  return mCheckBoxEnforceUnique->isChecked();
}

void QgsAttributeTypeDialog::setConstraintExpression( const QString &str )
{
  constraintExpressionWidget->setField( str );
}

void QgsAttributeTypeDialog::setConstraintExpressionEnforced( bool enforced )
{
  mCheckBoxEnforceExpression->setChecked( enforced );
}

bool QgsAttributeTypeDialog::constraintExpressionEnforced() const
{
  return mCheckBoxEnforceExpression->isChecked();
}

QString QgsAttributeTypeDialog::defaultValueExpression() const
{
  return mExpressionWidget->expression();
}

void QgsAttributeTypeDialog::setDefaultValueExpression( const QString &expression )
{
  mExpressionWidget->setExpression( expression );
}

int QgsAttributeTypeDialog::fieldIdx() const
{
  return mFieldIdx;
}


QgsExpressionContext QgsAttributeTypeDialog::createExpressionContext() const
{
  QgsExpressionContext context;
  context
      << QgsExpressionContextUtils::globalScope()
      << QgsExpressionContextUtils::projectScope( QgsProject::instance() )
      << QgsExpressionContextUtils::layerScope( mLayer )
      << QgsExpressionContextUtils::mapToolCaptureScope( QList<QgsPointLocator::Match>() );

  return context;
}

void QgsAttributeTypeDialog::onCurrentWidgetChanged( int index )
{
  Q_UNUSED( index )
  QStandardItem *item = currentItem();
  const QString editType = item ? item->data( Qt::UserRole ).toString() : QString();

  setEditorWidgetType( editType );
}

bool QgsAttributeTypeDialog::applyDefaultValueOnUpdate() const
{
  return mApplyDefaultValueOnUpdateCheckBox->isChecked();
}

void QgsAttributeTypeDialog::setApplyDefaultValueOnUpdate( bool applyDefaultValueOnUpdate )
{
  mApplyDefaultValueOnUpdateCheckBox->setChecked( applyDefaultValueOnUpdate );
}

QString QgsAttributeTypeDialog::constraintExpression() const
{
  return constraintExpressionWidget->asExpression();
}

void QgsAttributeTypeDialog::setFieldEditable( bool editable )
{
  isFieldEditableCheckBox->setChecked( editable );
}

void QgsAttributeTypeDialog::setAlias( const QString &alias )
{
  mAlias->setText( alias );
}

QString QgsAttributeTypeDialog::alias() const
{
  return mAlias->text();
}

void QgsAttributeTypeDialog::setDataDefinedProperties( const QgsPropertyCollection &properties )
{
  mDataDefinedProperties = properties;
  if ( properties.hasProperty( QgsEditFormConfig::DataDefinedProperty::Alias ) )
  {
    mAliasExpressionButton->setToProperty( properties.property( QgsEditFormConfig::DataDefinedProperty::Alias ) );
  }
}

void QgsAttributeTypeDialog::setComment( const QString &comment )
{
  laComment->setText( comment );
}

void QgsAttributeTypeDialog::setLabelOnTop( bool onTop )
{
  labelOnTopCheckBox->setChecked( onTop );
}

void QgsAttributeTypeDialog::defaultExpressionChanged()
{
  mWarnDefaultValueHasFieldsWidget->hide();

  const QString expression = mExpressionWidget->expression();
  if ( expression.isEmpty() )
  {
    mDefaultPreviewLabel->setText( QString() );
    return;
  }

  QgsExpressionContext context = mLayer->createExpressionContext();

  if ( !mPreviewFeature.isValid() )
  {
    // get first feature
    QgsFeatureIterator it = mLayer->getFeatures( QgsFeatureRequest().setLimit( 1 ) );
    it.nextFeature( mPreviewFeature );
  }

  context.setFeature( mPreviewFeature );

  QgsExpression exp = QgsExpression( expression );
  exp.prepare( &context );

  if ( exp.hasParserError() )
  {
    mDefaultPreviewLabel->setText( "<i>" + exp.parserErrorString() + "</i>" );
    return;
  }

  const QVariant val = exp.evaluate( &context );
  if ( exp.hasEvalError() )
  {
    mDefaultPreviewLabel->setText( "<i>" + exp.evalErrorString() + "</i>" );
    return;
  }

  // if the expression uses fields and it's not on update,
  // there is no warranty that the field will be available
  const bool expressionHasFields = exp.referencedAttributeIndexes( mLayer->fields() ).count() > 0;
  mWarnDefaultValueHasFieldsWidget->setVisible( expressionHasFields && !mApplyDefaultValueOnUpdateCheckBox->isChecked() );

  QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( editorWidgetType() );

  const QString previewText = fieldFormatter->representValue( mLayer, mFieldIdx, editorWidgetConfig(), QVariant(), val );

  mDefaultPreviewLabel->setText( "<i>" + previewText + "</i>" );
}

QStandardItem *QgsAttributeTypeDialog::currentItem() const
{
  QStandardItemModel *widgetTypeModel = qobject_cast<QStandardItemModel *>( mWidgetTypeComboBox->model() );
  return widgetTypeModel->item( mWidgetTypeComboBox->currentIndex() );
}

QgsPropertyCollection QgsAttributeTypeDialog::dataDefinedProperties() const
{
  return mDataDefinedProperties;
}
