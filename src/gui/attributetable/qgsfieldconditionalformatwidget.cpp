/***************************************************************************
    qgsfieldconditionalformatwidget.cpp
    ---------------------
    begin                : August 2015
    copyright            : (C) 2015 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfieldconditionalformatwidget.h"

#include "qgsexpressionbuilderdialog.h"
#include "qgssymbol.h"
#include "qgssymbolselectordialog.h"
#include "qgssymbollayerutils.h"
#include "qgsstyle.h"
#include "qgsvectorlayer.h"

QgsFieldConditionalFormatWidget::QgsFieldConditionalFormatWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );
  mDeleteButton->hide();
  connect( mFieldCombo, &QgsFieldComboBox::fieldChanged, this, &QgsFieldConditionalFormatWidget::fieldChanged );
  connect( fieldRadio, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::reloadStyles );
  connect( rowRadio, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::reloadStyles );
  connect( mNewButton, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::addNewRule );
  connect( mSaveRule, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::saveRule );
  connect( mCancelButton, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::cancelRule );
  connect( mDeleteButton, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::deleteRule );
  connect( listView, &QAbstractItemView::clicked, this, &QgsFieldConditionalFormatWidget::ruleClicked );
  connect( btnBuildExpression, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::setExpression );
  connect( mPresetsList, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsFieldConditionalFormatWidget::presetSet );
  btnBackgroundColor->setAllowOpacity( true );
  btnBackgroundColor->setShowNoColor( true );
  btnTextColor->setAllowOpacity( true );
  btnTextColor->setShowNoColor( true );
  mPresetsModel = new QStandardItemModel( listView );
  mModel = new QStandardItemModel( listView );
  listView->setModel( mModel );
  mPresetsList->setModel( mPresetsModel );
  btnChangeIcon->setSymbolType( QgsSymbol::Marker );
  btnChangeIcon->setSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );

  setPresets( defaultPresets() );
}

void QgsFieldConditionalFormatWidget::setExpression()
{
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "value" ), 0, true ) );
  context.setHighlightedVariables( QStringList() << QStringLiteral( "value" ) );

  QgsExpressionBuilderDialog dlg( mLayer, mRuleEdit->text(), this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Conditional style rule expression" ) );

  if ( dlg.exec() )
  {
    QString expression = dlg.expressionBuilder()->expressionText();
    mRuleEdit->setText( expression );
  }
}

void QgsFieldConditionalFormatWidget::presetSet( int index )
{
  if ( index == -1 || mPresets.isEmpty() )
    return;

  QgsConditionalStyle style = mPresets.at( index );
  setFormattingFromStyle( style );
}

void QgsFieldConditionalFormatWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
  mFieldCombo->setLayer( layer );
  mFieldCombo->setCurrentIndex( 0 );
}

void QgsFieldConditionalFormatWidget::ruleClicked( const QModelIndex &index )
{
  QList<QgsConditionalStyle> styles = getStyles();
  QgsConditionalStyle style = styles.at( index.row() );
  editStyle( index.row(), style );
}

void QgsFieldConditionalFormatWidget::editStyle( int editIndex, const QgsConditionalStyle &style )
{
  pages->setCurrentIndex( 1 );
  mEditIndex = editIndex;
  mEditing = true;
  mDeleteButton->show();
  loadStyle( style );
}

void QgsFieldConditionalFormatWidget::loadStyle( const QgsConditionalStyle &style )
{
  mRuleEdit->setText( style.rule() );
  mNameEdit->setText( style.name() );
  setFormattingFromStyle( style );
}
void QgsFieldConditionalFormatWidget::setFormattingFromStyle( const QgsConditionalStyle &style )
{
  btnBackgroundColor->setColor( style.backgroundColor() );
  btnTextColor->setColor( style.textColor() );
  if ( style.symbol() )
  {
    btnChangeIcon->setSymbol( style.symbol()->clone() );
    checkIcon->setChecked( true );
  }
  else
  {
    checkIcon->setChecked( false );
  }
  QFont font = style.font();
  mFontBoldBtn->setChecked( font.bold() );
  mFontItalicBtn->setChecked( font.italic() );
  mFontStrikethroughBtn->setChecked( font.strikeOut() );
  mFontUnderlineBtn->setChecked( font.underline() );
  mFontFamilyCmbBx->setFont( font );
}

QList<QgsConditionalStyle> QgsFieldConditionalFormatWidget::getStyles()
{
  QList<QgsConditionalStyle> styles;
  if ( fieldRadio->isChecked() )
  {
    styles = mLayer->conditionalStyles()->fieldStyles( mFieldCombo->currentField() );
  }
  if ( rowRadio->isChecked() )
  {
    styles = mLayer->conditionalStyles()->rowStyles();
  }
  return styles;
}

void QgsFieldConditionalFormatWidget::deleteRule()
{
  QList<QgsConditionalStyle> styles = getStyles();
  styles.removeAt( mEditIndex );
  QString fieldName;
  if ( fieldRadio->isChecked() )
  {
    fieldName = mFieldCombo->currentField();
    mLayer->conditionalStyles()->setFieldStyles( fieldName, styles );
  }
  if ( rowRadio->isChecked() )
  {
    mLayer->conditionalStyles()->setRowStyles( styles );
  }

  pages->setCurrentIndex( 0 );
  reloadStyles();
  emit rulesUpdated( fieldName );
}

void QgsFieldConditionalFormatWidget::cancelRule()
{
  pages->setCurrentIndex( 0 );
  reloadStyles();
  reset();
}

void QgsFieldConditionalFormatWidget::addNewRule()
{
  pages->setCurrentIndex( 1 );
  reset();
}

void QgsFieldConditionalFormatWidget::reset()
{
  mNameEdit->clear();
  mRuleEdit->clear();
  if ( fieldRadio->isChecked() )
  {
    mRuleEdit->setText( QStringLiteral( "@value " ) );
  }
  btnBackgroundColor->setColor( QColor() );
  btnTextColor->setColor( QColor() );
  mPresetsList->setCurrentIndex( 0 );
  mDeleteButton->hide();
  mEditing = false;
  checkIcon->setChecked( false );
  btnChangeIcon->setIcon( QIcon() );
  btnBackgroundColor->setToNoColor();
  btnTextColor->setToNoColor();

  mFontBoldBtn->setChecked( false );
  mFontItalicBtn->setChecked( false );
  mFontStrikethroughBtn->setChecked( false );
  mFontUnderlineBtn->setChecked( false );
}


void QgsFieldConditionalFormatWidget::setPresets( const QList<QgsConditionalStyle> &styles )
{
  mPresets.clear();
  mPresetsModel->clear();
  Q_FOREACH ( const QgsConditionalStyle &style, styles )
  {
    if ( style.isValid() )
    {
      QStandardItem *item = new QStandardItem( QStringLiteral( "abc - 123" ) );
      if ( style.backgroundColor().isValid() )
        item->setBackground( style.backgroundColor() );
      if ( style.textColor().isValid() )
        item->setForeground( style.textColor() );
      if ( style.symbol() )
        item->setIcon( style.icon() );
      item->setFont( style.font() );
      mPresetsModel->appendRow( item );
      mPresets.append( style );
    }
  }
  mPresetsList->setCurrentIndex( 0 );
}

QList<QgsConditionalStyle> QgsFieldConditionalFormatWidget::defaultPresets() const
{
  QList<QgsConditionalStyle> styles;
  QgsConditionalStyle style = QgsConditionalStyle();
  style.setBackgroundColor( QColor( 154, 216, 113 ) );
  styles.append( style );
  style = QgsConditionalStyle();
  style.setBackgroundColor( QColor( 251, 193, 78 ) );
  styles.append( style );
  style = QgsConditionalStyle();
  style.setBackgroundColor( QColor( 251, 154, 153 ) );
  styles.append( style );
  style = QgsConditionalStyle();
  style.setTextColor( QColor( 154, 216, 113 ) );
  styles.append( style );
  style = QgsConditionalStyle();
  style.setTextColor( QColor( 251, 193, 78 ) );
  styles.append( style );
  style = QgsConditionalStyle();
  style.setTextColor( QColor( 251, 154, 153 ) );
  styles.append( style );
  return styles;
}

void QgsFieldConditionalFormatWidget::saveRule()
{
  QList<QgsConditionalStyle> styles = getStyles();

  QgsConditionalStyle style = QgsConditionalStyle();

  style.setRule( mRuleEdit->text() );
  style.setName( mNameEdit->text() );

  QColor backColor = btnBackgroundColor->color();
  QColor fontColor = btnTextColor->color();

  QFont font = mFontFamilyCmbBx->currentFont();
  font.setBold( mFontBoldBtn->isChecked() );
  font.setItalic( mFontItalicBtn->isChecked() );
  font.setStrikeOut( mFontStrikethroughBtn->isChecked() );
  font.setUnderline( mFontUnderlineBtn->isChecked() );
  style.setFont( font );
  style.setBackgroundColor( backColor );
  style.setTextColor( fontColor );
  if ( checkIcon->isChecked() )
  {
    style.setSymbol( btnChangeIcon->clonedSymbol< QgsMarkerSymbol >() );
  }
  else
  {
    style.setSymbol( nullptr );
  }
  if ( mEditing )
  {
    styles.replace( mEditIndex, style );
  }
  else
  {
    styles.append( style );
  }

  QString fieldName;
  if ( fieldRadio->isChecked() )
  {
    fieldName = mFieldCombo->currentField();
    mLayer->conditionalStyles()->setFieldStyles( fieldName, styles );
  }
  if ( rowRadio->isChecked() )
  {
    mLayer->conditionalStyles()->setRowStyles( styles );
  }
  pages->setCurrentIndex( 0 );
  reloadStyles();
  emit rulesUpdated( fieldName );
  reset();
}

void QgsFieldConditionalFormatWidget::reloadStyles()
{
  mModel->clear();

  Q_FOREACH ( const QgsConditionalStyle &style, getStyles() )
  {
    QStandardItem *item = new QStandardItem( style.displayText() );
    item->setIcon( QIcon( style.renderPreview() ) );
    mModel->appendRow( item );
  }
}

void QgsFieldConditionalFormatWidget::fieldChanged( const QString &fieldName )
{
  Q_UNUSED( fieldName );
  reloadStyles();
}

void QgsFieldConditionalFormatWidget::viewRules()
{
  pages->setCurrentIndex( 0 );
}

bool QgsFieldConditionalFormatWidget::isCustomSet()
{
  return ( btnBackgroundColor->color().isValid()
           || btnTextColor->color().isValid()
           || mFontButtons->checkedId() != -1 );
}
