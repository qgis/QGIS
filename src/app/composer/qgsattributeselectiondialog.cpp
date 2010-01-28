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
    QWidget * parent, Qt::WindowFlags f ): QDialog( parent, f ), mVectorLayer( vLayer )
{
  if ( vLayer )
  {
    QScrollArea* attributeScrollArea = new QScrollArea( this );
    QWidget* attributeWidget = new QWidget();

    mAttributeGridLayout = new QGridLayout( attributeWidget );
    QLabel* attributeLabel = new QLabel( QString( "<b>" ) + tr( "Attribute" ) + QString( "</b>" ), this );
    attributeLabel->setTextFormat( Qt::RichText );
    mAttributeGridLayout->addWidget( attributeLabel, 0, 0 );
    QLabel* aliasLabel = new QLabel( QString( "<b>" ) + tr( "Alias" ) + QString( "</b>" ), this );
    aliasLabel->setTextFormat( Qt::RichText );
    mAttributeGridLayout->addWidget( aliasLabel, 0, 1 );

    QgsFieldMap fieldMap = vLayer->pendingFields();
    QgsFieldMap::const_iterator fieldIt = fieldMap.constBegin();
    int layoutRowCounter = 1;
    for ( ; fieldIt != fieldMap.constEnd(); ++fieldIt )
    {
      QCheckBox* attributeCheckBox = new QCheckBox( fieldIt.value().name(), this );
      if ( enabledAttributes.size() < 1 || enabledAttributes.contains( fieldIt.key() ) )
      {
        attributeCheckBox->setCheckState( Qt::Checked );
      }
      else
      {
        attributeCheckBox->setCheckState( Qt::Unchecked );
      }
      mAttributeGridLayout->addWidget( attributeCheckBox, layoutRowCounter, 0 );

      QLineEdit* attributeLineEdit = new QLineEdit( this );
      QMap<int, QString>::const_iterator aliasIt = aliasMap.find( fieldIt.key() );
      if ( aliasIt != aliasMap.constEnd() )
      {
        attributeLineEdit->setText( aliasIt.value() );
      }
      mAttributeGridLayout->addWidget( attributeLineEdit, layoutRowCounter, 1 );
      ++layoutRowCounter;
    }

    attributeScrollArea->setWidget( attributeWidget );

    QVBoxLayout* verticalLayout = new QVBoxLayout( this );
    verticalLayout->addWidget( attributeScrollArea );

    QHBoxLayout* selectClearLayout = new QHBoxLayout( this );
    QPushButton* mSelectAllButton = new QPushButton( tr( "Select all" ), this );
    QObject::connect( mSelectAllButton, SIGNAL( clicked() ), this, SLOT( selectAllAttributes() ) );
    QPushButton* mClearButton = new QPushButton( tr( "Clear" ), this );
    QObject::connect( mClearButton, SIGNAL( clicked() ), this, SLOT( clearAttributes() ) );
    selectClearLayout->addWidget( mSelectAllButton );
    selectClearLayout->addWidget( mClearButton );
    verticalLayout->addLayout( selectClearLayout );


    QDialogButtonBox* buttonBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
    QObject::connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
    QObject::connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
    verticalLayout->addWidget( buttonBox );
  }
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

void QgsAttributeSelectionDialog::selectAllAttributes()
{
  setAllEnabled( true );
}

void QgsAttributeSelectionDialog::clearAttributes()
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

