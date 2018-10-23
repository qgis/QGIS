/***************************************************************************
  qgsrelationadddlg.cpp - QgsRelationAddDlg
  ---------------------------------

 begin                : 4.10.2013
 copyright            : (C) 2013 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrelationadddlg.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerproxymodel.h"

#include <QPushButton>

QgsRelationAddDlg::QgsRelationAddDlg( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  connect( mCbxReferencingLayer, &QgsMapLayerComboBox::layerChanged, mCbxReferencingField, &QgsFieldComboBox::setLayer );
  connect( mCbxReferencedLayer, &QgsMapLayerComboBox::layerChanged, mCbxReferencedField, &QgsFieldComboBox::setLayer );

  mCbxReferencingLayer->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mCbxReferencingField->setLayer( mCbxReferencingLayer->currentLayer() );
  mCbxReferencedLayer->setFilters( QgsMapLayerProxyModel::VectorLayer );
  mCbxReferencedField->setLayer( mCbxReferencedLayer->currentLayer() );

  mCbxRelationStrength->addItem( "Association", QVariant::fromValue( QgsRelation::RelationStrength::Association ) );
  mCbxRelationStrength->addItem( "Composition", QVariant::fromValue( QgsRelation::RelationStrength::Composition ) );
  mCbxRelationStrength->setToolTip( QStringLiteral( "On composition, the child features will be duplicated too.\nDuplications are made by the feature duplication action.\nThe default actions are activated in the Action section of the layer properties." ) );

  mTxtRelationId->setPlaceholderText( tr( "[Generated automatically]" ) );
  checkDefinitionValid();

  connect( mCbxReferencingLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
  connect( mCbxReferencingField, &QgsFieldComboBox::fieldChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
  connect( mCbxReferencedLayer, &QgsMapLayerComboBox::layerChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
  connect( mCbxReferencedField, &QgsFieldComboBox::fieldChanged, this, &QgsRelationAddDlg::checkDefinitionValid );
}

QString QgsRelationAddDlg::referencingLayerId()
{
  return mCbxReferencingLayer->currentLayer()->id();
}

QString QgsRelationAddDlg::referencedLayerId()
{
  return mCbxReferencedLayer->currentLayer()->id();
}

QList< QPair< QString, QString > > QgsRelationAddDlg::references()
{
  QList< QPair< QString, QString > > references;

  QString referencingField = mCbxReferencingField->currentField();
  QString referencedField = mCbxReferencedField->currentField();

  references.append( QPair<QString, QString> ( referencingField, referencedField ) );

  return references;
}

QString QgsRelationAddDlg::relationId()
{
  return mTxtRelationId->text();
}

QString QgsRelationAddDlg::relationName()
{
  return mTxtRelationName->text();
}

QgsRelation::RelationStrength QgsRelationAddDlg::relationStrength()
{
#if QT_VERSION <= 0x050601
  // in Qt 5.6.1 and former, QVariant does not correctly convert enum using value
  // see https://bugreports.qt.io/browse/QTBUG-53384
  return static_cast<QgsRelation::RelationStrength>( mCbxRelationStrength->currentData() );
#else
  return mCbxRelationStrength->currentData().value<QgsRelation::RelationStrength>();
#endif
}

void QgsRelationAddDlg::checkDefinitionValid()
{
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( mCbxReferencedLayer->currentIndex() != -1
      && mCbxReferencedField->currentIndex() != -1
      && mCbxReferencingLayer->currentIndex() != -1
      && mCbxReferencingField->currentIndex() != -1 );
}
