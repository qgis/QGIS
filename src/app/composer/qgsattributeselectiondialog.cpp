/***************************************************************************
                         qgsattributeselectiondialog.cpp
                         -------------------------------
    begin                : January 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco at hugis dot net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsattributeselectiondialog.h"
#include "qgsvectorlayer.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>

QgsAttributeSelectionDialog::QgsAttributeSelectionDialog( const QgsVectorLayer* vLayer, const QSet<int>& enabledAttributes, const QMap<int, QString>& aliasMap,
    const QList< QPair<int, bool> >& sortColumns, QWidget* parent, Qt::WindowFlags f ): QDialog( parent, f ), mVectorLayer( vLayer )
{
  setupUi( this );
  if ( vLayer )
  {
    QgsFieldMap fieldMap = vLayer->pendingFields();
    QgsFieldMap::const_iterator fieldIt = fieldMap.constBegin();
    int layoutRowCounter = 1;
    for ( ; fieldIt != fieldMap.constEnd(); ++fieldIt )
    {
      //insert field into sorting combo first
      mSortColumnComboBox->addItem( fieldIt.value().name(), QVariant( fieldIt.key() ) );

      //and into enabled / alias list
      QCheckBox* attributeCheckBox = new QCheckBox( fieldIt.value().name(), this );
      if ( enabledAttributes.size() < 1 || enabledAttributes.contains( fieldIt.key() ) )
      {
        attributeCheckBox->setCheckState( Qt::Checked );
      }
      else
      {
        attributeCheckBox->setCheckState( Qt::Unchecked );
      }
      mAttributeGridLayout->addWidget(( QWidget* )attributeCheckBox, layoutRowCounter, 0, 1, 1 );

      QLineEdit* attributeLineEdit = new QLineEdit( this );
      QMap<int, QString>::const_iterator aliasIt = aliasMap.find( fieldIt.key() );
      if ( aliasIt != aliasMap.constEnd() )
      {
        attributeLineEdit->setText( aliasIt.value() );
      }
      mAttributeGridLayout->addWidget(( QWidget* )attributeLineEdit, layoutRowCounter, 1, 1, 1 );
      ++layoutRowCounter;
    }

    //sort columns
    QList< QPair<int, bool> >::const_iterator sortIt = sortColumns.constBegin();
    for ( ; sortIt != sortColumns.constEnd(); ++sortIt )
    {
      QTreeWidgetItem* item = new QTreeWidgetItem();
      item->setText( 0, fieldMap[sortIt->first].name() );
      item->setData( 0, Qt::UserRole, sortIt->first );
      item->setText( 1, sortIt->second ? tr( "Ascending" ) : tr( "Descending" ) );
      mSortColumnTreeWidget->addTopLevelItem( item );
    }
  }

  mOrderComboBox->insertItem( 0, tr( "Ascending" ) );
  mOrderComboBox->insertItem( 0, tr( "Descending" ) );
}

QgsAttributeSelectionDialog::~QgsAttributeSelectionDialog()
{

}

QSet<int> QgsAttributeSelectionDialog::enabledAttributes() const
{
  QSet<int> result;
  if ( !mAttributeGridLayout || !mVectorLayer )
  {
    return result;
  }

  for ( int i = 1; i < mAttributeGridLayout->rowCount(); ++i )
  {
    QLayoutItem *checkBoxItem = mAttributeGridLayout->itemAtPosition( i, 0 );
    if ( checkBoxItem )
    {
      QCheckBox *checkBox = qobject_cast< QCheckBox * >( checkBoxItem->widget() );
      if ( checkBox && checkBox->checkState() == Qt::Checked )
      {
        result.insert( mVectorLayer->fieldNameIndex( checkBox->text() ) );
      }
    }
  }

  return result;
}

QMap<int, QString> QgsAttributeSelectionDialog::aliasMap() const
{
  QMap<int, QString> result;
  if ( !mAttributeGridLayout || !mVectorLayer )
  {
    return result;
  }

  for ( int i = 1; i < mAttributeGridLayout->rowCount(); ++i )
  {
    QLayoutItem* lineEditItem = mAttributeGridLayout->itemAtPosition( i, 1 );
    QLayoutItem* checkBoxItem = mAttributeGridLayout->itemAtPosition( i, 0 );
    if ( lineEditItem && checkBoxItem )
    {
      QLineEdit *lineEdit = qobject_cast<QLineEdit*>( lineEditItem->widget() );
      QCheckBox *checkBox = qobject_cast<QCheckBox*>( checkBoxItem->widget() );
      if ( lineEdit )
      {
        QString aliasText = lineEdit->text();
        if ( !aliasText.isEmpty() && checkBox )
        {
          //insert into map
          int fieldIndex = mVectorLayer->fieldNameIndex( checkBox->text() );
          result.insert( fieldIndex, aliasText );
        }
      }
    }
  }
  return result;
}

QList< QPair<int, bool> > QgsAttributeSelectionDialog::attributeSorting() const
{
  QList< QPair<int, bool> > sortingList;

  for ( int i = 0; i < mSortColumnTreeWidget->topLevelItemCount(); ++i )
  {
    QTreeWidgetItem* item = mSortColumnTreeWidget->topLevelItem( i );
    if ( item )
    {
      sortingList.push_back( qMakePair( item->data( 0, Qt::UserRole ).toInt(), item->text( 1 ) == tr( "Ascending" ) ) );
    }
  }

  return sortingList;
}

void QgsAttributeSelectionDialog::on_mSelectAllButton_clicked()
{
  setAllEnabled( true );
}

void QgsAttributeSelectionDialog::on_mClearButton_clicked()
{
  setAllEnabled( false );
}

void QgsAttributeSelectionDialog::setAllEnabled( bool enabled )
{
  if ( mAttributeGridLayout )
  {
    int nRows = mAttributeGridLayout->rowCount();
    for ( int i = 0; i < nRows; ++i )
    {
      QLayoutItem* checkBoxItem = mAttributeGridLayout->itemAtPosition( i, 0 );
      if ( checkBoxItem )
      {
        QWidget* checkBoxWidget = checkBoxItem->widget();
        if ( checkBoxWidget )
        {
          QCheckBox* checkBox = dynamic_cast<QCheckBox*>( checkBoxWidget );
          if ( checkBox )
          {
            checkBox->setCheckState( enabled ? Qt::Checked : Qt::Unchecked );
          }
        }
      }
    }
  }
}

void QgsAttributeSelectionDialog::on_mAddPushButton_clicked()
{
  QTreeWidgetItem* item = new QTreeWidgetItem();
  item->setText( 0, mSortColumnComboBox->currentText() );
  item->setData( 0, Qt::UserRole, mSortColumnComboBox->itemData( mSortColumnComboBox->currentIndex() ) );
  item->setText( 1, mOrderComboBox->currentText() );
  mSortColumnTreeWidget->addTopLevelItem( item );
}

void QgsAttributeSelectionDialog::on_mRemovePushButton_clicked()
{
  int currentIndex = mSortColumnTreeWidget->indexOfTopLevelItem( mSortColumnTreeWidget->currentItem() );
  if ( currentIndex != -1 )
  {
    delete( mSortColumnTreeWidget->takeTopLevelItem( currentIndex ) );
  }
}

void QgsAttributeSelectionDialog::on_mUpPushButton_clicked()
{
  int currentIndex = mSortColumnTreeWidget->indexOfTopLevelItem( mSortColumnTreeWidget->currentItem() );
  if ( currentIndex != -1 )
  {
    if ( currentIndex > 0 )
    {
      QTreeWidgetItem* item = mSortColumnTreeWidget->takeTopLevelItem( currentIndex );
      mSortColumnTreeWidget->insertTopLevelItem( currentIndex - 1, item );
      mSortColumnTreeWidget->setCurrentItem( item );
    }
  }
}

void QgsAttributeSelectionDialog::on_mDownPushButton_clicked()
{
  int currentIndex = mSortColumnTreeWidget->indexOfTopLevelItem( mSortColumnTreeWidget->currentItem() );
  if ( currentIndex != -1 )
  {
    if ( currentIndex < ( mSortColumnTreeWidget->topLevelItemCount() - 1 ) )
    {
      QTreeWidgetItem* item = mSortColumnTreeWidget->takeTopLevelItem( currentIndex );
      mSortColumnTreeWidget->insertTopLevelItem( currentIndex + 1, item );
      mSortColumnTreeWidget->setCurrentItem( item );
    }
  }
}
