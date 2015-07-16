#include "qgsfieldconditionalformatwidget.h"

#include "qgsfielduiproperties.h"

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
  mModel = new QStandardItemModel();
  listView->setModel( mModel );
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
  // TODO Match the colors
  mDeleteButton->show();
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
}

void QgsFieldConditionalFormatWidget::addNewRule()
{
  pages->setCurrentIndex( 1 );
  mRuleEdit->clear();
  mDefault1->toggle();
  mDeleteButton->hide();
}

void QgsFieldConditionalFormatWidget::saveRule()
{
  QgsFieldUIProperties props = mLayer->fieldUIProperties( mFieldCombo->currentField() );
  QList<QgsConditionalStyle> styles = props.getConditionalStyles();
  QgsConditionalStyle style = QgsConditionalStyle();
  QAbstractButton* button = mDefaultButtons->checkedButton();
  QColor backColor = button->property( "backColor" ).value<QColor>();
  QColor fontColor = button->property( "fontColor" ).value<QColor>();
  style.setRule( mRuleEdit->text() );
  style.setBackgroundColor( backColor );
  style.setTextColor( fontColor );
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
  reloadStyles();
}

void QgsFieldConditionalFormatWidget::viewRules()
{
  pages->setCurrentIndex( 0 );
}
