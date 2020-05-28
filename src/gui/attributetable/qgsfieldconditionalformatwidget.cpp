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
#include "qgsexpressioncontextutils.h"
#include "qgsguiutils.h"

//
// QgsFieldConditionalFormatWidget
//

QgsFieldConditionalFormatWidget::QgsFieldConditionalFormatWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );
  setPanelTitle( tr( "Conditional Styles" ) );
  connect( mFieldCombo, &QgsFieldComboBox::fieldChanged, this, &QgsFieldConditionalFormatWidget::fieldChanged );
  connect( fieldRadio, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::reloadStyles );
  connect( rowRadio, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::reloadStyles );
  connect( mNewButton, &QAbstractButton::clicked, this, &QgsFieldConditionalFormatWidget::addNewRule );
  connect( listView, &QAbstractItemView::clicked, this, &QgsFieldConditionalFormatWidget::ruleClicked );
  mModel = new QStandardItemModel( listView );
  listView->setModel( mModel );

  connect( fieldRadio, &QRadioButton::toggled, mFieldCombo, &QWidget::setEnabled );

  mPresets = defaultPresets();
}

void QgsFieldConditionalFormatWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
  mFieldCombo->setLayer( layer );
  mFieldCombo->setCurrentIndex( 0 );
  fieldChanged( mFieldCombo->currentField() );
}

void QgsFieldConditionalFormatWidget::ruleClicked( const QModelIndex &index )
{
  QList<QgsConditionalStyle> styles = getStyles();
  QgsConditionalStyle style = styles.at( index.row() );
  editStyle( index.row(), style );
}

void QgsFieldConditionalFormatWidget::editStyle( int editIndex, const QgsConditionalStyle &style )
{
  mEditIndex = editIndex;
  mEditing = editIndex >= 0;
  mPanelHandled = false;

  QgsEditConditionalFormatRuleWidget *ruleWidget = new QgsEditConditionalFormatRuleWidget();
  ruleWidget->setLayer( mLayer );
  ruleWidget->setPresets( mPresets );
  ruleWidget->loadStyle( style );
  ruleWidget->setDockMode( true );

  if ( fieldRadio->isChecked() && style.rule().isEmpty() )
  {
    ruleWidget->setRule( QStringLiteral( "@value " ) );
  }

  connect( ruleWidget, &QgsEditConditionalFormatRuleWidget::panelAccepted, this, [ = ]
  {
    if ( mPanelHandled )
    {
      // already handled the result of the panel, and the panel is being dismissed as a result
      // of an already dealt with action
      return;
    }

    QList<QgsConditionalStyle> styles = getStyles();
    if ( mEditing )
    {
      styles.replace( mEditIndex, ruleWidget->currentStyle() );
    }
    else
    {
      styles.append( ruleWidget->currentStyle() );
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
    reloadStyles();
    emit rulesUpdated( fieldName );
  } );

  connect( ruleWidget, &QgsEditConditionalFormatRuleWidget::ruleSaved, this, [ = ]
  {
    ruleWidget->acceptPanel();
  } );

  connect( ruleWidget, &QgsEditConditionalFormatRuleWidget::canceled, this, [ = ]
  {
    mPanelHandled = true;
    ruleWidget->acceptPanel();
  } );

  connect( ruleWidget, &QgsEditConditionalFormatRuleWidget::ruleDeleted, this, [ = ]
  {
    deleteCurrentRule();
    mPanelHandled = true;
    ruleWidget->acceptPanel();
  } );
  showPanel( ruleWidget );
}

void QgsFieldConditionalFormatWidget::loadStyle( const QgsConditionalStyle & )
{
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

void QgsFieldConditionalFormatWidget::addNewRule()
{
  editStyle( -1, QgsConditionalStyle() );
}

void QgsFieldConditionalFormatWidget::reset()
{
}

void QgsFieldConditionalFormatWidget::setPresets( const QList<QgsConditionalStyle> &styles )
{
  mPresets = styles;
}

QList<QgsConditionalStyle> QgsFieldConditionalFormatWidget::defaultPresets()
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

void QgsFieldConditionalFormatWidget::reloadStyles()
{
  mModel->clear();

  const auto constGetStyles = getStyles();

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
  const QSize size( Qgis::UI_SCALE_FACTOR * fontMetrics().width( 'X' ) * 10, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 2 );
#else
  const QSize size( Qgis::UI_SCALE_FACTOR * fontMetrics().horizontalAdvance( 'X' ) * 10, Qgis::UI_SCALE_FACTOR * fontMetrics().height() * 2 );
#endif

  listView->setIconSize( size );

  for ( const QgsConditionalStyle &style : constGetStyles )
  {
    QStandardItem *item = new QStandardItem( style.displayText() );
    item->setIcon( QIcon( style.renderPreview( size ) ) );
    mModel->appendRow( item );
  }
}

void QgsFieldConditionalFormatWidget::fieldChanged( const QString &fieldName )
{
  Q_UNUSED( fieldName )
  reloadStyles();
}

void QgsFieldConditionalFormatWidget::deleteCurrentRule()
{
  if ( !mEditing )
    return;

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

  reloadStyles();
  emit rulesUpdated( fieldName );
}

void QgsFieldConditionalFormatWidget::viewRules()
{
}


//
// QgsEditConditionalFormatRuleWidget
//

QgsEditConditionalFormatRuleWidget::QgsEditConditionalFormatRuleWidget( QWidget *parent )
  : QgsPanelWidget( parent )
{
  setupUi( this );

  setPanelTitle( tr( "Edit Rule" ) );

  btnBackgroundColor->setColor( QColor() );
  btnTextColor->setColor( QColor() );
  checkIcon->setChecked( false );
  btnChangeIcon->setIcon( QIcon() );
  btnBackgroundColor->setToNoColor();
  btnTextColor->setToNoColor();

  mFontBoldBtn->setChecked( false );
  mFontItalicBtn->setChecked( false );
  mFontStrikethroughBtn->setChecked( false );
  mFontUnderlineBtn->setChecked( false );

  const int buttonSize = QgsGuiUtils::scaleIconSize( 24 );
  mFontUnderlineBtn->setMinimumSize( buttonSize, buttonSize );
  mFontUnderlineBtn->setMaximumSize( buttonSize, buttonSize );
  mFontStrikethroughBtn->setMinimumSize( buttonSize, buttonSize );
  mFontStrikethroughBtn->setMaximumSize( buttonSize, buttonSize );
  mFontBoldBtn->setMinimumSize( buttonSize, buttonSize );
  mFontBoldBtn->setMaximumSize( buttonSize, buttonSize );
  mFontItalicBtn->setMinimumSize( buttonSize, buttonSize );
  mFontItalicBtn->setMaximumSize( buttonSize, buttonSize );

  connect( mSaveRule, &QAbstractButton::clicked, this, &QgsEditConditionalFormatRuleWidget::ruleSaved );
  connect( mCancelButton, &QAbstractButton::clicked, this, &QgsEditConditionalFormatRuleWidget::canceled );
  connect( mDeleteButton, &QAbstractButton::clicked, this, &QgsEditConditionalFormatRuleWidget::ruleDeleted );

  connect( btnBuildExpression, &QAbstractButton::clicked, this, &QgsEditConditionalFormatRuleWidget::setExpression );
  connect( mPresetsList, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsEditConditionalFormatRuleWidget::presetSet );

  btnBackgroundColor->setAllowOpacity( true );
  btnBackgroundColor->setShowNoColor( true );
  btnTextColor->setAllowOpacity( true );
  btnTextColor->setShowNoColor( true );
  mPresetsModel = new QStandardItemModel( mPresetsList );
  mPresetsList->setModel( mPresetsModel );

  btnChangeIcon->setSymbolType( QgsSymbol::Marker );
  btnChangeIcon->setSymbol( QgsSymbol::defaultSymbol( QgsWkbTypes::PointGeometry ) );
  connect( checkIcon, &QCheckBox::toggled, btnChangeIcon, &QWidget::setEnabled );
}

void QgsEditConditionalFormatRuleWidget::setLayer( QgsVectorLayer *layer )
{
  mLayer = layer;
}

void QgsEditConditionalFormatRuleWidget::loadStyle( const QgsConditionalStyle &style )
{
  mRuleEdit->setText( style.rule() );
  mNameEdit->setText( style.name() );
  setFormattingFromStyle( style );
}

QgsConditionalStyle QgsEditConditionalFormatRuleWidget::currentStyle() const
{
  QgsConditionalStyle style;

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
  return style;
}

void QgsEditConditionalFormatRuleWidget::setExpression()
{
  QgsExpressionContext context( QgsExpressionContextUtils::globalProjectLayerScopes( mLayer ) );
  context.lastScope()->addVariable( QgsExpressionContextScope::StaticVariable( QStringLiteral( "value" ), 0, true ) );
  context.setHighlightedVariables( QStringList() << QStringLiteral( "value" ) );

  QgsExpressionBuilderDialog dlg( mLayer, mRuleEdit->text(), this, QStringLiteral( "generic" ), context );
  dlg.setWindowTitle( tr( "Conditional Style Rule Expression" ) );

  if ( dlg.exec() )
  {
    QString expression = dlg.expressionBuilder()->expressionText();
    mRuleEdit->setText( expression );
  }
}

void QgsEditConditionalFormatRuleWidget::presetSet( int index )
{
  if ( index == -1 || mPresets.isEmpty() )
    return;

  const int styleIndex = mPresetsList->currentData( Qt::UserRole + 1 ).toInt();
  QgsConditionalStyle style = mPresets.at( styleIndex );
  setFormattingFromStyle( style );
}

void QgsEditConditionalFormatRuleWidget::setFormattingFromStyle( const QgsConditionalStyle &style )
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
  mFontFamilyCmbBx->setCurrentFont( font );
}

void QgsEditConditionalFormatRuleWidget::setPresets( const QList<QgsConditionalStyle> &styles )
{
  mPresets.clear();
  mPresetsModel->clear();
  QStandardItem *item = new QStandardItem( QString() );
  mPresetsModel->appendRow( item );
  int i = 0;
  for ( const QgsConditionalStyle &style : styles )
  {
    if ( style.isValid() )
    {
      QStandardItem *item = new QStandardItem( QStringLiteral( "abc - 123" ) );
      if ( style.validBackgroundColor() )
        item->setBackground( style.backgroundColor() );
      if ( style.validTextColor() )
        item->setForeground( style.textColor() );
      if ( style.symbol() )
        item->setIcon( style.icon() );
      item->setFont( style.font() );
      item->setData( i, Qt::UserRole + 1 );
      mPresetsModel->appendRow( item );
      mPresets.append( style );
      i++;
    }
  }
  mPresetsList->setCurrentIndex( 0 );
}

void QgsEditConditionalFormatRuleWidget::setRule( const QString &rule )
{
  mRuleEdit->setText( rule );
}

bool QgsEditConditionalFormatRuleWidget::isCustomSet()
{
  return ( btnBackgroundColor->color().isValid()
           || btnTextColor->color().isValid()
           || mFontButtons->checkedId() != -1 );
}
