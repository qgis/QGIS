#include "qgsfieldconditionalformatwidget.h"

#include "qgsfield.h"

QgsFieldConditionalFormatWidget::QgsFieldConditionalFormatWidget(QWidget *parent) :
  QWidget(parent)
{
  setupUi( this );
  connect( mFieldCombo, SIGNAL( fieldChanged(QString) ), SLOT( fieldChanged(QString) ) );
  connect( mNewButton, SIGNAL( clicked() ), SLOT( addNewRule() ) );
  connect( mSaveRule, SIGNAL( clicked() ), SLOT( saveRule() ) );
  connect( mCancelButton, SIGNAL( clicked() ), SLOT( cancelRule() ) );
  connect( listView, SIGNAL( clicked(QModelIndex)), SLOT( ruleClicked(QModelIndex) ));
  mModel = new QStandardItemModel();
  listView->setModel(mModel);
}

void QgsFieldConditionalFormatWidget::setLayer(QgsVectorLayer *theLayer)
{
  mLayer = theLayer;
  mFieldCombo->setLayer(theLayer);
  mFieldCombo->setCurrentIndex(0);
}

void QgsFieldConditionalFormatWidget::ruleClicked(QModelIndex index)
{
  QgsFieldUIProperties props = mLayer->fieldUIProperties(mFieldCombo->currentField());
  QList<QgsConditionalStyle> styles = props.getConditionalStyles();
  QgsConditionalStyle style = styles.at(index.row());
  pages->setCurrentIndex(1);
  mRuleEdit->setText(style.rule);
}
void QgsFieldConditionalFormatWidget::cancelRule()
{
  pages->setCurrentIndex(0);
  reloadStyles();
}

void QgsFieldConditionalFormatWidget::addNewRule()
{
  pages->setCurrentIndex(1);
  mRuleEdit->clear();
  mDefault1->toggle();
}

void QgsFieldConditionalFormatWidget::saveRule()
{
  QgsFieldUIProperties props = mLayer->fieldUIProperties(mFieldCombo->currentField());
  QList<QgsConditionalStyle> styles = props.getConditionalStyles();
  QgsConditionalStyle style = QgsConditionalStyle();
  style.rule = mRuleEdit->text();
  QAbstractButton* button = mDefaultButtons->checkedButton();
  QColor backColor = button->property("backColor").value<QColor>();
  QColor fontColor = button->property("fontColor").value<QColor>();
  style.backColor = backColor;
  style.textColor = fontColor;
  styles.append(style);
  props.setConditionalStyles(styles);
  mLayer->setFieldUIProperties(mFieldCombo->currentField(), props);
  pages->setCurrentIndex(0);
  reloadStyles();
}

void QgsFieldConditionalFormatWidget::reloadStyles()
{
  mModel->clear();
  QgsFieldUIProperties props = mLayer->fieldUIProperties(mFieldCombo->currentField());
  QList<QgsConditionalStyle> styles = props.getConditionalStyles();
  foreach(QgsConditionalStyle style, styles)
    {
      QStandardItem* item = new QStandardItem(style.rule);
      mModel->appendRow(item);
    }
}

void QgsFieldConditionalFormatWidget::fieldChanged(QString fieldName)
{
  reloadStyles();
}

void QgsFieldConditionalFormatWidget::viewRules()
{
  pages->setCurrentIndex(0);
}
