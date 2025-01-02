/***************************************************************************
                          qgsaddtaborgroup.h
        Add a tab or a group for the tab and group display of fields
                             -------------------
    begin                : 2012-07-30
    copyright            : (C) 2012 by Denis Rouzaud
    email                : denis dot rouzaud at gmail dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsvectorlayer.h"
#include "qgsaddtaborgroup.h"
#include "moc_qgsaddtaborgroup.cpp"
#include "qgssettings.h"
#include "qgshelp.h"

#include <QTreeWidgetItem>
#include <QComboBox>
#include <QRadioButton>

QgsAddAttributeFormContainerDialog::QgsAddAttributeFormContainerDialog( QgsVectorLayer *lyr, const QList<ContainerPair> &existingContainerList, QTreeWidgetItem *currentTab, QWidget *parent )
  : QDialog( parent )
  , mLayer( lyr )
  , mExistingContainers( existingContainerList )
{
  setupUi( this );

  mTypeCombo->addItem( tr( "Tab" ), QVariant::fromValue( Qgis::AttributeEditorContainerType::Tab ) );
  mTypeCombo->addItem( tr( "Group Box" ), QVariant::fromValue( Qgis::AttributeEditorContainerType::GroupBox ) );
  mTypeCombo->addItem( tr( "Row" ), QVariant::fromValue( Qgis::AttributeEditorContainerType::Row ) );

  mTypeCombo->setCurrentIndex( mTypeCombo->findData( QVariant::fromValue( Qgis::AttributeEditorContainerType::Tab ) ) );

  mParentCombo->addItem( QString() );
  if ( !mExistingContainers.isEmpty() )
  {
    int i = 0;
    for ( const ContainerPair &container : std::as_const( mExistingContainers ) )
    {
      mParentCombo->addItem( container.first, i );
      if ( container.second == currentTab )
      {
        mParentCombo->setCurrentIndex( i );
        mTypeCombo->setCurrentIndex( mTypeCombo->findData( QVariant::fromValue( Qgis::AttributeEditorContainerType::GroupBox ) ) );
      }
      ++i;
    }
  }

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsAddAttributeFormContainerDialog::showHelp );

  mColumnCountSpinBox->setValue( QgsSettings().value( QStringLiteral( "/qgis/attributeForm/defaultTabColumnCount" ), 1 ).toInt() );

  setWindowTitle( tr( "Add Container for %1" ).arg( mLayer->name() ) );

  connect( mTypeCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, &QgsAddAttributeFormContainerDialog::containerTypeChanged );
  containerTypeChanged();
}

QString QgsAddAttributeFormContainerDialog::name()
{
  return mName->text();
}

QTreeWidgetItem *QgsAddAttributeFormContainerDialog::parentContainerItem()
{
  if ( containerType() == Qgis::AttributeEditorContainerType::Tab )
    return nullptr;

  if ( !mParentCombo->currentData().isValid() )
    return nullptr;

  const ContainerPair tab = mExistingContainers.at( mParentCombo->currentData().toInt() );
  return tab.second;
}

int QgsAddAttributeFormContainerDialog::columnCount() const
{
  return mColumnCountSpinBox->value();
}

Qgis::AttributeEditorContainerType QgsAddAttributeFormContainerDialog::containerType() const
{
  return mTypeCombo->currentData().value<Qgis::AttributeEditorContainerType>();
}

void QgsAddAttributeFormContainerDialog::accept()
{
  if ( mColumnCountSpinBox->value() > 0 )
  {
    switch ( containerType() )
    {
      case Qgis::AttributeEditorContainerType::GroupBox:
        QgsSettings().setValue( QStringLiteral( "/qgis/attributeForm/defaultGroupColumnCount" ), mColumnCountSpinBox->value() );
        break;
      case Qgis::AttributeEditorContainerType::Tab:
        QgsSettings().setValue( QStringLiteral( "/qgis/attributeForm/defaultTabColumnCount" ), mColumnCountSpinBox->value() );
        break;
      case Qgis::AttributeEditorContainerType::Row:
        break;
    }
  }

  QDialog::accept();
}

void QgsAddAttributeFormContainerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_vector/vector_properties.html#the-drag-and-drop-designer" ) );
}

void QgsAddAttributeFormContainerDialog::containerTypeChanged()
{
  const Qgis::AttributeEditorContainerType type = mTypeCombo->currentData().value<Qgis::AttributeEditorContainerType>();
  switch ( type )
  {
    case Qgis::AttributeEditorContainerType::GroupBox:
      mParentCombo->show();
      mLabelParent->show();
      mColumnsLabel->show();
      mColumnCountSpinBox->show();
      mColumnCountSpinBox->setValue( QgsSettings().value( QStringLiteral( "/qgis/attributeForm/defaultGroupColumnCount" ), 1 ).toInt() );
      break;
    case Qgis::AttributeEditorContainerType::Tab:
      mParentCombo->hide();
      mLabelParent->hide();
      mColumnsLabel->show();
      mColumnCountSpinBox->show();
      mColumnCountSpinBox->setValue( QgsSettings().value( QStringLiteral( "/qgis/attributeForm/defaultTabColumnCount" ), 1 ).toInt() );
      break;
    case Qgis::AttributeEditorContainerType::Row:
      mParentCombo->show();
      mLabelParent->show();
      mColumnsLabel->hide();
      mColumnCountSpinBox->hide();
      break;
  }
}
