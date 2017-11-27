#include "qgsattributerelationedit.h"
#include "ui_qgsattributerelationedit.h"

QgsAttributeRelationEdit::QgsAttributeRelationEdit( const QString &relationid, QWidget *parent ) :
  QWidget( parent ),
  mRelationId( relationid )
{
  setupUi( this );
}

void QgsAttributeRelationEdit::setCardinalityCombo( const QString &cardinalityComboItem, const QVariant &auserData )
{
  coCardinality->addItem( cardinalityComboItem, auserData );
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
