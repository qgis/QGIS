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
#include "qgsattributetypeloaddialog.h"
#include "qgsvectordataprovider.h"
#include "qgsmapcanvas.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgisapp.h"
#include "qgsproject.h"
#include "qgslogger.h"
#include "qgsfieldformatterregistry.h"
#include "qgsfieldformatter.h"
#include "qgseditorwidgetfactory.h"
#include "qgseditorwidgetregistry.h"
#include "qgsgui.h"

#include <QTableWidgetItem>
#include <QFile>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QScrollBar>

#include <climits>
#include <cfloat>

QgsAttributeTypeDialog::QgsAttributeTypeDialog( QgsVectorLayer *vl, int fieldIdx )
  : QDialog()
  , mLayer( vl )
  , mFieldIdx( fieldIdx )
{
  setupUi( this );
  setWindowTitle( tr( "Edit Widget Properties - %1 (%2)" ).arg( vl->fields().at( fieldIdx ).name(), vl->name() ) );

  QMapIterator<QString, QgsEditorWidgetFactory *> it( QgsGui::editorWidgetRegistry()->factories() );
  while ( it.hasNext() )
  {
    it.next();
    QListWidgetItem *item = new QListWidgetItem( selectionListWidget );
    item->setText( it.value()->name() );
    item->setData( Qt::UserRole, it.key() );
    if ( !it.value()->supportsField( vl, fieldIdx ) )
      item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
    selectionListWidget->addItem( item );
  }

  // Set required list width based on content + twice the border width
  selectionListWidget->setMinimumWidth( selectionListWidget->sizeHintForColumn( 0 )
                                        + 2 );
  selectionListWidget->setMaximumWidth( selectionListWidget->sizeHintForColumn( 0 )
                                        + 2 );

  if ( vl->fields().fieldOrigin( fieldIdx ) == QgsFields::OriginJoin ||
       vl->fields().fieldOrigin( fieldIdx ) == QgsFields::OriginExpression )
  {
    isFieldEditableCheckBox->setEnabled( false );
  }

  mExpressionWidget->registerExpressionContextGenerator( this );

  connect( mExpressionWidget, &QgsExpressionLineEdit::expressionChanged, this, &QgsAttributeTypeDialog::defaultExpressionChanged );
  connect( mUniqueCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mCheckBoxEnforceUnique->setEnabled( checked );
    if ( !checked )
      mCheckBoxEnforceUnique->setChecked( false );
  }
         );
  connect( notNullCheckBox, &QCheckBox::toggled, this, [ = ]( bool checked )
  {
    mCheckBoxEnforceNotNull->setEnabled( checked );
    if ( !checked )
      mCheckBoxEnforceNotNull->setChecked( false );
  }
         );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/QgsAttributeTypeDialog/geometry" ) ).toByteArray() );

  constraintExpressionWidget->setLayer( vl );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsAttributeTypeDialog::showHelp );
}

QgsAttributeTypeDialog::~QgsAttributeTypeDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/QgsAttributeTypeDialog/geometry" ), saveGeometry() );

  qDeleteAll( mEditorConfigWidgets );
}

const QString QgsAttributeTypeDialog::editorWidgetType()
{
  QListWidgetItem *item = selectionListWidget->currentItem();
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
  QListWidgetItem *item = selectionListWidget->currentItem();
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
  QListWidgetItem *item = selectionListWidget->currentItem();
  if ( item )
  {
    QString widgetType = item->data( Qt::UserRole ).toString();
    QgsEditorConfigWidget *cfgWdg = mEditorConfigWidgets[ widgetType ];
    if ( cfgWdg )
    {
      return cfgWdg->config();
    }
  }

  return QVariantMap();
}

void QgsAttributeTypeDialog::setEditorWidgetType( const QString &type )
{
  for ( int i = 0; i < selectionListWidget->count(); i++ )
  {
    QListWidgetItem *item = selectionListWidget->item( i );
    if ( item->data( Qt::UserRole ).toString() == type )
    {
      selectionListWidget->setCurrentItem( item );
      break;
    }
  }

  if ( mEditorConfigWidgets.contains( type ) )
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
      QgsDebugMsg( "Oops, couldn't create editor widget config dialog..." );
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
    mCheckBoxEnforceNotNull->setChecked( true );
    mCheckBoxEnforceNotNull->setEnabled( false );
  }

  if ( constraints & QgsFieldConstraints::ConstraintUnique )
  {
    mUniqueCheckBox->setChecked( true );
    mUniqueCheckBox->setEnabled( false );
    mUniqueCheckBox->setToolTip( tr( "The provider for this layer has a UNIQUE constraint set on the field." ) );
    mCheckBoxEnforceUnique->setChecked( true );
    mCheckBoxEnforceUnique->setEnabled( false );
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
  return notNullCheckBox->isChecked();
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
  return mUniqueCheckBox->isChecked();
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

QString QgsAttributeTypeDialog::constraintExpression() const
{
  return constraintExpressionWidget->asExpression();
}

void QgsAttributeTypeDialog::setFieldEditable( bool editable )
{
  isFieldEditableCheckBox->setChecked( editable );
}

void QgsAttributeTypeDialog::setLabelOnTop( bool onTop )
{
  labelOnTopCheckBox->setChecked( onTop );
}

void QgsAttributeTypeDialog::on_selectionListWidget_currentRowChanged( int index )
{
  const QString editType = selectionListWidget->item( index )->data( Qt::UserRole ).toString();

  setEditorWidgetType( editType );
}

void QgsAttributeTypeDialog::defaultExpressionChanged()
{
  QString expression = mExpressionWidget->expression();
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

  QVariant val = exp.evaluate( &context );
  if ( exp.hasEvalError() )
  {
    mDefaultPreviewLabel->setText( "<i>" + exp.evalErrorString() + "</i>" );
    return;
  }

  QgsFieldFormatter *fieldFormatter = QgsApplication::fieldFormatterRegistry()->fieldFormatter( editorWidgetType() );

  QString previewText = fieldFormatter->representValue( mLayer, mFieldIdx, editorWidgetConfig(), QVariant(), val );

  mDefaultPreviewLabel->setText( "<i>" + previewText + "</i>" );
}

void QgsAttributeTypeDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#configure-the-field-behavior" ) );
}
