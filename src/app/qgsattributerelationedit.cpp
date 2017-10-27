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

void QgsAttributeRelationEdit::setCardinalityCombo( const QString &cardinalityComboItem )
{
  coCardinality->addItem( cardinalityComboItem );
}

void QgsAttributeRelationEdit::setCardinality( const QString &cardinality )
{
  int idx = coCardinality->findText( cardinality );

  if ( idx != -1 )
    coCardinality->setCurrentIndex( idx );
}

QString QgsAttributeRelationEdit::cardinality()
{
  return coCardinality->currentText();
}
