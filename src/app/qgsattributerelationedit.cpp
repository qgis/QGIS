#include "qgsattributerelationedit.h"
#include "ui_qgsattributerelationedit.h"

QgsAttributeRelationEdit::QgsAttributeRelationEdit(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::QgsAttributeRelationEdit)
{
  ui->setupUi(this);
}

QgsAttributeRelationEdit::~QgsAttributeRelationEdit()
{
  delete ui;
}

void QgsAttributeRelationEdit::setCardinality( const QString &cardinality )
{
  leCardinality->setText( cardinality );
}

QString QgsAttributeRelationEdit::cardinality()
{
  return leCardinality->text();
}
