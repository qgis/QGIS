/***************************************************************************
                         qgsattributedialog.cpp  -  description
                             -------------------
    begin                : October 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include "qgsattributedialog.h"
#include "qgsfield.h"
#include "qgslogger.h"

#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsuniquevaluerenderer.h"
#include "qgssymbol.h"

#include <QTableWidgetItem>
#include <QSettings>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QCompleter>
#include <QSpinBox>
#include <QDoubleSpinBox>

QgsAttributeDialog::QgsAttributeDialog(QgsVectorLayer *vl, QgsFeature *thepFeature)
  : QDialog(),
    mSettingsPath("/Windows/AttributeDialog/"),
    mpFeature(thepFeature),
    mLayer(vl)
{
  setupUi(this);
  if (mpFeature==NULL || vl->getDataProvider()==NULL )
    return;

  const QgsFieldMap &theFieldMap = vl->getDataProvider()->fields();

  if (theFieldMap.isEmpty()) return;

  QgsAttributeMap myAttributes = mpFeature->attributeMap();
  //
  //Set up dynamic inside a scroll box
  //
  QVBoxLayout * mypOuterLayout = new QVBoxLayout();
  mypOuterLayout->setContentsMargins(0,0,0,0);
  //transfers layout ownership so no need to call delete
  mFrame->setLayout(mypOuterLayout);
  QScrollArea * mypScrollArea = new QScrollArea();
  //transfers scroll area ownership so no need to call delete
  mypOuterLayout->addWidget(mypScrollArea);
  QFrame * mypInnerFrame = new QFrame();
  mypInnerFrame->setFrameShape(QFrame::NoFrame);
  mypInnerFrame->setFrameShadow(QFrame::Plain);
  //transfers frame ownership so no need to call delete
  mypScrollArea->setWidget(mypInnerFrame);
  mypScrollArea->setWidgetResizable( true );
  QGridLayout * mypInnerLayout = new QGridLayout(mypInnerFrame);


  int classificationField = -1;
  QMap<QString,QString> classes;

  const QgsUniqueValueRenderer *uvr = dynamic_cast<const QgsUniqueValueRenderer *>( mLayer->renderer() );
  if( uvr )
  {
    classificationField = uvr->classificationField();

    const QList<QgsSymbol *> symbols = uvr->symbols();

    for(int i=0; i<symbols.size(); i++)
    {
      QString label = symbols[i]->label();
      QString name = symbols[i]->lowerValue();

      if(label=="")
        label=name;

      classes.insert(name, label);
    }
  }

  int index=0;
  for (QgsAttributeMap::const_iterator it = myAttributes.begin(); 
      it != myAttributes.end(); 
      ++it)
  {
    const QgsField &field = theFieldMap[it.key()];
    QString myFieldName = field.name();
    int myFieldType = field.type();
    QLabel * mypLabel = new QLabel();
    mypInnerLayout->addWidget(mypLabel,index,0);
    QVariant myFieldValue = it.value();
   
    QWidget *myWidget;

    QgsVectorLayer::EditType editType = vl->editType( it.key() );

    switch( editType )
    {
    case QgsVectorLayer::Range:
      {
        if( myFieldType==QVariant::Int )
        {
          int min = vl->range( it.key() ).mMin.toInt();
          int max = vl->range( it.key() ).mMax.toInt();
          int step = vl->range( it.key() ).mStep.toInt();

          QSpinBox *sb = new QSpinBox();
          sb->setMinimum(min);
          sb->setMaximum(max);
          sb->setSingleStep(step);
          sb->setValue( it.value().toInt() );

          myWidget = sb;
          break;
        } else if( myFieldType==QVariant::Double ) {
          double min = vl->range( it.key() ).mMin.toDouble();
          double max = vl->range( it.key() ).mMax.toDouble();
          double step = vl->range( it.key() ).mStep.toDouble();
          QDoubleSpinBox *dsb = new QDoubleSpinBox();
          
          dsb->setMinimum(min);
          dsb->setMaximum(max);
          dsb->setSingleStep(step);
          dsb->setValue( it.value().toDouble() );

          myWidget = dsb;

          break;
        }
        
      }

      // fall-through


    case QgsVectorLayer::LineEdit:
    case QgsVectorLayer::UniqueValuesEditable:
      {
        QLineEdit *le = new QLineEdit( myFieldValue.toString() );

        if( editType == QgsVectorLayer::UniqueValuesEditable )
        {
          QStringList values;
          mLayer->getDataProvider()->getUniqueValues(it.key(), values);

          QCompleter *c = new QCompleter(values);
          c->setCompletionMode(QCompleter::PopupCompletion);
          le->setCompleter(c);
        }

        if( myFieldType==QVariant::Int )
        {
          le->setValidator( new QIntValidator(le) );
        }
        else if( myFieldType==QVariant::Double )
        {
          le->setValidator( new QIntValidator(le) );
        }

        myWidget = le;
      }
      break;

    case QgsVectorLayer::UniqueValues:
      {
        QStringList values;
        mLayer->getDataProvider()->getUniqueValues(it.key(), values);

        QComboBox *cb = new QComboBox();
        cb->setEditable(true);
        cb->addItems(values);

        int idx = cb->findText( myFieldValue.toString() );
        if( idx>= 0 )
          cb->setCurrentIndex( idx );

        myWidget = cb;
      }
      break;

    case QgsVectorLayer::ValueMap:
      {
        const QMap<QString,QVariant> &map = vl->valueMap( it.key() );

        QComboBox *cb = new QComboBox();

        for(QMap<QString,QVariant>::const_iterator it=map.begin(); it!=map.end(); it++)
        {
          cb->addItem( it.key(), it.value() );
        }

        int idx = cb->findData( myFieldValue );
        if( idx>= 0 )
          cb->setCurrentIndex( idx );

        myWidget = cb;
      }
      break;

    case QgsVectorLayer::Classification:
      {
        QComboBox *cb = new QComboBox();
        for(QMap<QString,QString>::const_iterator it=classes.begin(); it!=classes.end(); it++)
        {
          cb->addItem( it.value(), it.key() );
        }

        int idx = cb->findData( myFieldValue );
        if( idx>=0 )
          cb->setCurrentIndex( idx );

        myWidget = cb;
      }
      break;
    }

    if( myFieldType==QVariant::Int )
    {
      mypLabel->setText(myFieldName + tr(" (int)"));
    }
    else if( myFieldType==QVariant::Double )
    {
      mypLabel->setText(myFieldName + tr(" (dbl)"));
    }
    else //string
    {
      //any special behaviour for string goes here
      mypLabel->setText(myFieldName + tr(" (txt)"));
    }

    mypInnerLayout->addWidget(myWidget, index, 1);
    mpWidgets << myWidget;
    ++index;
  }
  restoreGeometry();
}


QgsAttributeDialog::~QgsAttributeDialog()
{
  saveGeometry();
}
  
void QgsAttributeDialog::accept()
{
  //write the new values back to the feature
  QgsAttributeMap myAttributes = mpFeature->attributeMap();
  int myIndex=0;
  for (QgsAttributeMap::const_iterator it = myAttributes.begin(); 
      it != myAttributes.end(); 
      ++it)
  {
    const QgsField &theField = mLayer->pendingFields()[it.key()];
    QgsVectorLayer::EditType editType = mLayer->editType( it.key() );
    QString myFieldName = theField.name();
    bool myFlag=false;
    QString myFieldValue;

    QLineEdit *le = dynamic_cast<QLineEdit *>(mpWidgets.value(myIndex));
    if(le)
    {
      myFieldValue = le->text();
    }

    QComboBox *cb = dynamic_cast<QComboBox *>(mpWidgets.value(myIndex));
    if(cb)
    {
      if( editType==QgsVectorLayer::UniqueValues ||
          editType==QgsVectorLayer::ValueMap ||
          editType==QgsVectorLayer::Classification)
      {
        myFieldValue = cb->itemData( cb->currentIndex() ).toString();
      }
      else
      {
        myFieldValue = cb->currentText();
      }
    }


    QSpinBox *sb = dynamic_cast<QSpinBox *>(mpWidgets.value(myIndex));
    if(sb)
    {
      myFieldValue = QString::number(sb->value());
    }

    QDoubleSpinBox *dsb = dynamic_cast<QDoubleSpinBox *>(mpWidgets.value(myIndex));
    if(dsb)
    {
      myFieldValue = QString::number(dsb->value());
    }

    switch( theField.type() )
    {
    case QVariant::Int:
      {
        int myIntValue = myFieldValue.toInt(&myFlag);
        if (myFlag && ! myFieldValue.isEmpty())
        {
          mpFeature->changeAttribute( it.key(), QVariant(myIntValue) );
        }
        else
        {
          mpFeature->changeAttribute( it.key(), QVariant(QString::null) );
        }
      }
      break;
    case QVariant::Double:
      {
        double myDblValue = myFieldValue.toDouble(&myFlag);
        if (myFlag && ! myFieldValue.isEmpty())
        {
          mpFeature->changeAttribute( it.key(), QVariant(myDblValue) );
        }
        else
        {
          mpFeature->changeAttribute( it.key(), QVariant(QString::null) );
        }
      }
      break;
    default: //string
      mpFeature->changeAttribute(it.key(),QVariant( myFieldValue ) );
      break;
    }
    ++myIndex;
  }
  QDialog::accept();
}

void QgsAttributeDialog::saveGeometry()
{
  QSettings settings;
  settings.setValue(mSettingsPath+"geometry", QDialog::saveGeometry());
}

void QgsAttributeDialog::restoreGeometry()
{
  QSettings settings;
  QDialog::restoreGeometry(settings.value(mSettingsPath+"geometry").toByteArray());
}
