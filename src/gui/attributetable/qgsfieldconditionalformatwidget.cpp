#include "qgsfieldconditionalformatwidget.h"

#include "qgsexpressionbuilderdialog.h"
#include "qgssymbolv2.h"
#include "qgssymbolv2selectordialog.h"
#include "qgssymbollayerv2utils.h"
#include "qgsstylev2.h"

QgsFieldConditionalFormatWidget::QgsFieldConditionalFormatWidget( QWidget *parent )
    : QWidget( parent )
    , mLayer( 0 )
    , mEditIndex( 0 )
    , mEditing( false )
    , mSymbol( 0 )
{
  setupUi( this );
  mDeleteButton->hide();
  connect( mFieldCombo, SIGNAL( fieldChanged( QString ) ), SLOT( fieldChanged( QString ) ) );
  connect( fieldRadio, SIGNAL( clicked() ), SLOT( reloadStyles() ) );
  connect( rowRadio, SIGNAL( clicked() ), SLOT( reloadStyles() ) );
  connect( mNewButton, SIGNAL( clicked() ), SLOT( addNewRule() ) );
  connect( mSaveRule, SIGNAL( clicked() ), SLOT( saveRule() ) );
  connect( mCancelButton, SIGNAL( clicked() ), SLOT( cancelRule() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( deleteRule() ) );
  connect( listView, SIGNAL( clicked( QModelIndex ) ), SLOT( ruleClicked( QModelIndex ) ) );
  connect( btnChangeIcon , SIGNAL( clicked() ), SLOT( updateIcon() ) );
  connect( btnBuildExpression , SIGNAL( clicked() ), SLOT( setExpression() ) );
  connect( mPresetsList , SIGNAL( currentIndexChanged( int ) ), SLOT( presetSet( int ) ) );
  btnBackgroundColor->setAllowAlpha( true );
  btnBackgroundColor->setShowNoColor( true );
  btnTextColor->setAllowAlpha( true );
  btnTextColor->setShowNoColor( true );
  mPresetsModel = new QStandardItemModel( listView );
  mModel = new QStandardItemModel( listView );
  listView->setModel( mModel );
  mPresetsList->setModel( mPresetsModel );

  setPresets( defaultPresets() );
}

QgsFieldConditionalFormatWidget::~QgsFieldConditionalFormatWidget()
{
  delete mSymbol;
}

void QgsFieldConditionalFormatWidget::updateIcon()
{
  mSymbol = QgsSymbolV2::defaultSymbol( QGis::Point );

  QgsSymbolV2SelectorDialog dlg( mSymbol, QgsStyleV2::defaultStyle(), 0, this );
  if ( !dlg.exec() )
  {
    return;
  }

  QIcon icon = QgsSymbolLayerV2Utils::symbolPreviewIcon( mSymbol, btnChangeIcon->iconSize() );
  btnChangeIcon->setIcon( icon );
}

void QgsFieldConditionalFormatWidget::setExpression()
{
  QgsExpressionContext context;
  context << QgsExpressionContextUtils::globalScope()
  << QgsExpressionContextUtils::projectScope()
  << QgsExpressionContextUtils::layerScope( mLayer );
  context.lastScope()->setVariable( "value", 0 );
  context.setHighlightedVariables( QStringList() << "value" );

  QgsExpressionBuilderDialog dlg( mLayer, mRuleEdit->text(), this, "generic", context );
  dlg.setWindowTitle( tr( "Conditional style rule expression" ) );

  if ( dlg.exec() )
  {
    QString expression =  dlg.expressionBuilder()->expressionText();
    mRuleEdit->setText( expression );
  }
}

void QgsFieldConditionalFormatWidget::presetSet( int index )
{
  if ( index == -1 || mPresets.count() == 0 )
    return;

  QgsConditionalStyle style = mPresets.at( index );
  setFormattingFromStyle( style );
}

void QgsFieldConditionalFormatWidget::setLayer( QgsVectorLayer *theLayer )
{
  mLayer = theLayer;
  mFieldCombo->setLayer( theLayer );
  mFieldCombo->setCurrentIndex( 0 );
}

void QgsFieldConditionalFormatWidget::ruleClicked( QModelIndex index )
{
  QList<QgsConditionalStyle> styles = getStyles();
  QgsConditionalStyle style = styles.at( index.row() );
  editStyle( index.row(), style );
}

void QgsFieldConditionalFormatWidget::editStyle( int editIndex, QgsConditionalStyle style )
{
  pages->setCurrentIndex( 1 );
  mEditIndex = editIndex;
  mEditing = true;
  mDeleteButton->show();
  loadStyle( style );
}

void QgsFieldConditionalFormatWidget::loadStyle( QgsConditionalStyle style )
{
  mRuleEdit->setText( style.rule() );
  mNameEdit->setText( style.name() );
  setFormattingFromStyle( style );
}
void QgsFieldConditionalFormatWidget::setFormattingFromStyle( QgsConditionalStyle style )
{
  btnBackgroundColor->setColor( style.backgroundColor() );
  btnTextColor->setColor( style.textColor() );
  if ( !style.icon().isNull() )
  {
    checkIcon->setChecked( true );
    QIcon icon( style.icon() );
    btnChangeIcon->setIcon( icon );
  }
  else
  {
    checkIcon->setChecked( false );
    btnChangeIcon->setIcon( QIcon() );
  }
  if ( style.symbol() )
  {
    mSymbol = style.symbol()->clone();
  }
  else
  {
    mSymbol = 0;
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
    fieldName =  mFieldCombo->currentField();
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
  mSymbol = 0;
  mNameEdit->clear();
  mRuleEdit->clear();
  if ( fieldRadio->isChecked() )
  {
    mRuleEdit->setText( "@value " );
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


void QgsFieldConditionalFormatWidget::setPresets( QList<QgsConditionalStyle> styles )
{
  mPresets.clear();
  mPresetsModel->clear();
  Q_FOREACH ( const QgsConditionalStyle& style, styles )
  {
    if ( style.isValid() )
    {
      QStandardItem* item = new QStandardItem( "abc - 123" );
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
  if ( mSymbol && checkIcon->isChecked() )
  {
    style.setSymbol( mSymbol );
  }
  else
  {
    style.setSymbol( 0 );
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
    fieldName =  mFieldCombo->currentField();
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

  Q_FOREACH ( const QgsConditionalStyle& style, getStyles() )
  {
    QStandardItem* item = new QStandardItem( style.displayText() );
    item->setIcon( QIcon( style.renderPreview() ) );
    mModel->appendRow( item );
  }
}

void QgsFieldConditionalFormatWidget::fieldChanged( QString fieldName )
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
