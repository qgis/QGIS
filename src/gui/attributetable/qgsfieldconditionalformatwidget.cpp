#include "qgsfieldconditionalformatwidget.h"

#include "qgsexpressionbuilderdialog.h"
#include "qgsfielduiproperties.h"
#include "qgssymbolv2.h"
#include "qgssymbolv2selectordialog.h"
#include "qgssymbollayerv2utils.h"
#include "qgsstylev2.h"

QgsFieldConditionalFormatWidget::QgsFieldConditionalFormatWidget( QWidget *parent ) :
    QWidget( parent )
    , mEditing( false )
{
  setupUi( this );
  mDeleteButton->hide();
  connect( mFieldCombo, SIGNAL( fieldChanged( QString ) ), SLOT( fieldChanged( QString ) ) );
  connect( mNewButton, SIGNAL( clicked() ), SLOT( addNewRule() ) );
  connect( mSaveRule, SIGNAL( clicked() ), SLOT( saveRule() ) );
  connect( mCancelButton, SIGNAL( clicked() ), SLOT( cancelRule() ) );
  connect( mDeleteButton, SIGNAL( clicked() ), SLOT( deleteRule() ) );
  connect( listView, SIGNAL( clicked( QModelIndex ) ), SLOT( ruleClicked( QModelIndex ) ) );
  connect( mDefaultButtons , SIGNAL( buttonPressed( QAbstractButton* ) ), SLOT( defaultPressed( QAbstractButton* ) ) );
  connect( btnChangeIcon , SIGNAL( clicked() ), SLOT( updateIcon() ) );
  connect( btnBuildExpression , SIGNAL( clicked() ), SLOT( setExpression() ) );
  mModel = new QStandardItemModel();
  listView->setModel( mModel );
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
  QgsExpressionBuilderDialog dlg( mLayer, mRuleEdit->text(), this );
  dlg.setWindowTitle( tr( "Conditional style rule expression" ) );

  if ( dlg.exec() )
  {
    QString expression =  dlg.expressionBuilder()->expressionText();
    mRuleEdit->setText( expression );
  }
}

void QgsFieldConditionalFormatWidget::defaultPressed( QAbstractButton *button )
{
  QColor backColor = button->property( "backColor" ).value<QColor>();
  QColor fontColor = button->property( "fontColor" ).value<QColor>();
  btnBackgroundColor->setColor( backColor );
  btnTextColor->setColor( fontColor );
}

void QgsFieldConditionalFormatWidget::setLayer( QgsVectorLayer *theLayer )
{
  mLayer = theLayer;
  mFieldCombo->setLayer( theLayer );
  mFieldCombo->setCurrentIndex( 0 );
}

void QgsFieldConditionalFormatWidget::ruleClicked( QModelIndex index )
{
  QgsFieldUIProperties props = mLayer->fieldUIProperties( mFieldCombo->currentField() );
  QList<QgsConditionalStyle> styles = props.getConditionalStyles();
  QgsConditionalStyle style = styles.at( index.row() );
  editStyle( index.row(), style );
}

void QgsFieldConditionalFormatWidget::editStyle( int editIndex, QgsConditionalStyle style )
{
  pages->setCurrentIndex( 1 );
  mEditIndex = editIndex;
  mEditing = true;

  mRuleEdit->setText( style.rule() );
  mDeleteButton->show();
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

void QgsFieldConditionalFormatWidget::deleteRule()
{
  QgsFieldUIProperties props = mLayer->fieldUIProperties( mFieldCombo->currentField() );
  QList<QgsConditionalStyle> styles = props.getConditionalStyles();
  styles.removeAt( mEditIndex );
  props.setConditionalStyles( styles );
  mLayer->setFieldUIProperties( mFieldCombo->currentField(), props );
  pages->setCurrentIndex( 0 );
  reloadStyles();
  emit rulesUpdates();
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
  mRuleEdit->clear();
  btnBackgroundColor->setColor( QColor() );
  btnTextColor->setColor( QColor() );
  mDefault1->toggle();
  defaultPressed( mDefault1 );
  mDeleteButton->hide();
  mEditing = false;
  checkIcon->setChecked( false );
  btnChangeIcon->setIcon( QIcon() );

  mFontBoldBtn->setChecked( false );
  mFontItalicBtn->setChecked( false );
  mFontStrikethroughBtn->setChecked( false );
  mFontUnderlineBtn->setChecked( false );
}

void QgsFieldConditionalFormatWidget::saveRule()
{
  QgsFieldUIProperties props = mLayer->fieldUIProperties( mFieldCombo->currentField() );
  QList<QgsConditionalStyle> styles = props.getConditionalStyles();
  QgsConditionalStyle style = QgsConditionalStyle();

  style.setRule( mRuleEdit->text() );

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
  props.setConditionalStyles( styles );
  mLayer->setFieldUIProperties( mFieldCombo->currentField(), props );
  pages->setCurrentIndex( 0 );
  reloadStyles();
  emit rulesUpdates();
  reset();
}

void QgsFieldConditionalFormatWidget::reloadStyles()
{
  mModel->clear();
  QgsFieldUIProperties props = mLayer->fieldUIProperties( mFieldCombo->currentField() );
  QList<QgsConditionalStyle> styles = props.getConditionalStyles();
  foreach ( QgsConditionalStyle style, styles )
  {
    QStandardItem* item = new QStandardItem( style.rule() );
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
