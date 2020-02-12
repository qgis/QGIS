/***************************************************************************
    qgsattributerelationedit.cpp
    ---------------------
    begin                : October 2017
    copyright            : (C) 2017 by David Signer
    email                : david at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsattributerelationedit.h"
#include "ui_qgsattributerelationedit.h"

QgsAttributeRelationEdit::QgsAttributeRelationEdit( const QString &relationid, QWidget *parent )
  : QWidget( parent )
  , mRelationId( relationid )
{
  setupUi( this );
  coCardinality->setToolTip( tr( "For a many to many (N:M) relation, the direct link has to be selected. The in-between table will be hidden." ) );
}

void QgsAttributeRelationEdit::setCardinalityCombo( const QString &cardinalityComboItem, const QVariant &auserData )
{
  coCardinality->addItem( cardinalityComboItem, auserData );
}

void QgsAttributeRelationEdit::setCardinality( const QVariant &auserData )
{
  int idx = coCardinality->findData( auserData );

  if ( idx != -1 )
    coCardinality->setCurrentIndex( idx );
}

QVariant  QgsAttributeRelationEdit::cardinality()
{
  return coCardinality->currentData();
}
