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

QgsAttributeSelectionDialog::QgsAttributeSelectionDialog(const QgsVectorLayer* vLayer, const QSet<int>& enabledAttributes, const QMap<int, QString>& aliasMap, \
                                                         QWidget * parent, Qt::WindowFlags f): QDialog(parent, f), mVectorLayer(vLayer)
{
    if( vLayer )
    {
        mGridLayout = new QGridLayout(this);
        QLabel* attributeLabel = new QLabel(QString("<b>") + tr("Attribute") + QString("</b>"), this );
        attributeLabel->setTextFormat(Qt::RichText);
        mGridLayout->addWidget(attributeLabel, 0, 0);
        QLabel* aliasLabel = new QLabel(QString("<b>") + tr("Alias") + QString("</b>"), this );
        aliasLabel->setTextFormat(Qt::RichText);
        mGridLayout->addWidget(aliasLabel, 0, 1);

        QgsFieldMap fieldMap = vLayer->pendingFields();
        QgsFieldMap::const_iterator fieldIt = fieldMap.constBegin();
        int layoutRowCounter = 1;
        for(; fieldIt != fieldMap.constEnd(); ++fieldIt)
        {
            QCheckBox* attributeCheckBox = new QCheckBox(fieldIt.value().name(), this);
            if( enabledAttributes.size() < 1 || enabledAttributes.contains(fieldIt.key()) )
            {
                attributeCheckBox->setCheckState(Qt::Checked);
            }
            else
            {
                attributeCheckBox->setCheckState(Qt::Unchecked);
            }
            mGridLayout->addWidget(attributeCheckBox, layoutRowCounter, 0);

            QLineEdit* attributeLineEdit = new QLineEdit(this);
            QMap<int, QString>::const_iterator aliasIt = aliasMap.find( fieldIt.key() );
            if(aliasIt != aliasMap.constEnd())
            {
                attributeLineEdit->setText(aliasIt.value());
            }
            mGridLayout->addWidget(attributeLineEdit, layoutRowCounter, 1);
            ++layoutRowCounter;
        }

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
        QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
        mGridLayout->addWidget( buttonBox, layoutRowCounter, 0, 3, 1);
    }
}

QgsAttributeSelectionDialog::~QgsAttributeSelectionDialog()
{

}

QSet<int> QgsAttributeSelectionDialog::enabledAttributes() const
{
    QSet<int> result;
    if(!mGridLayout || !mVectorLayer)
    {
        return result;
    }

    for( int i = 1; i < mGridLayout->rowCount(); ++i)
    {
        bool boxChecked = false;
        QLayoutItem* checkBoxItem = mGridLayout->itemAtPosition(i, 0);
        if(checkBoxItem)
        {
            QWidget* checkBoxWidget = checkBoxItem->widget();
            if(checkBoxWidget)
            {
                QCheckBox* checkBox = dynamic_cast<QCheckBox*>(checkBoxWidget);
                if(checkBox)
                {
                    if(checkBox->checkState() == Qt::Checked)
                    {
                        result.insert(mVectorLayer->fieldNameIndex(checkBox->text()));
                    }
                }
            }
        }
    }

    return result;
}

QMap<int, QString> QgsAttributeSelectionDialog::aliasMap() const
{
    QMap<int, QString> result;
    if(!mGridLayout || !mVectorLayer)
    {
        return result;
    }

    for( int i = 1; i < mGridLayout->rowCount(); ++i)
    {
        QLayoutItem* lineEditItem = mGridLayout->itemAtPosition(i, 1);
        QLayoutItem* checkBoxItem = mGridLayout->itemAtPosition(i, 0);
        if(lineEditItem && checkBoxItem)
        {
            QWidget* lineEditWidget = lineEditItem->widget();
            QWidget* checkBoxWidget = checkBoxItem->widget();
            if(lineEditWidget)
            {
                QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(lineEditWidget);
                QCheckBox* checkBox = dynamic_cast<QCheckBox*>(checkBoxWidget);
                if(lineEdit)
                {
                    QString aliasText = lineEdit->text();
                    if(!aliasText.isEmpty())
                    {
                        //insert into map
                        int fieldIndex = mVectorLayer->fieldNameIndex( checkBox->text() );
                        result.insert(fieldIndex, aliasText);
                    }
                }
            }
        }
    }
    return result;
}

