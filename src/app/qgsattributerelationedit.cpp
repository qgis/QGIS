#include "qgsattributerelationedit.h"
#include "ui_qgsattributerelationedit.h"

QgsAttributeRelationEdit::QgsAttributeRelationEdit(const QString &relationid, QWidget *parent) :
  QWidget(parent),
  mRelationId( relationid )
{
  setupUi(this);
}

QgsAttributeRelationEdit::~QgsAttributeRelationEdit()
{
}

void QgsAttributeRelationEdit::setCardinality( const QString &cardinality )
{
  leCardinality->setText( cardinality );
}

QString QgsAttributeRelationEdit::cardinality()
{
  return leCardinality->text();
}
