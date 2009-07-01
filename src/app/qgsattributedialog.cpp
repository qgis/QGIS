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
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>

QgsAttributeDialog::QgsAttributeDialog( QgsVectorLayer *vl, QgsFeature *thepFeature )
    : QDialog(),
    mSettingsPath( "/Windows/AttributeDialog/" ),
    mLayer( vl ),
    mpFeature( thepFeature )
{
  setupUi( this );
  if ( mpFeature == NULL || vl->dataProvider() == NULL )
    return;

  const QgsFieldMap &theFieldMap = vl->pendingFields();

  if ( theFieldMap.isEmpty() ) return;

  QgsAttributeMap myAttributes = mpFeature->attributeMap();
  //
  //Set up dynamic inside a scroll box
  //
  QVBoxLayout * mypOuterLayout = new QVBoxLayout();
  mypOuterLayout->setContentsMargins( 0, 0, 0, 0 );
  //transfers layout ownership so no need to call delete
  mFrame->setLayout( mypOuterLayout );
  QScrollArea * mypScrollArea = new QScrollArea();
  //transfers scroll area ownership so no need to call delete
  mypOuterLayout->addWidget( mypScrollArea );
  QFrame * mypInnerFrame = new QFrame();
  mypInnerFrame->setFrameShape( QFrame::NoFrame );
  mypInnerFrame->setFrameShadow( QFrame::Plain );
  //transfers frame ownership so no need to call delete
  mypScrollArea->setWidget( mypInnerFrame );
  mypScrollArea->setWidgetResizable( true );
  QGridLayout * mypInnerLayout = new QGridLayout( mypInnerFrame );


  int classificationField = -1;
  QMap<QString, QString> classes;

  const QgsUniqueValueRenderer *uvr = dynamic_cast<const QgsUniqueValueRenderer *>( mLayer->renderer() );
  if ( uvr )
  {
    classificationField = uvr->classificationField();

    const QList<QgsSymbol *> symbols = uvr->symbols();

    for ( int i = 0; i < symbols.size(); i++ )
    {
      QString label = symbols[i]->label();
      QString name = symbols[i]->lowerValue();

      if ( label == "" )
        label = name;

      classes.insert( name, label );
    }
  }

  int index = 0;
  for ( QgsAttributeMap::const_iterator it = myAttributes.begin();
        it != myAttributes.end();
        ++it )
  {
    const QgsField &field = theFieldMap[it.key()];

    //show attribute alias if available
    QString myFieldName = vl->attributeDisplayName( it.key() );
    int myFieldType = field.type();
    QLabel * mypLabel = new QLabel();
    mypInnerLayout->addWidget( mypLabel, index, 0 );
    QVariant myFieldValue = it.value();

    QWidget *myWidget;

    QgsVectorLayer::EditType editType = vl->editType( it.key() );

    switch ( editType )
    {
      case QgsVectorLayer::UniqueValues:
      {
        QList<QVariant> values;
        mLayer->dataProvider()->uniqueValues( it.key(), values );

        QComboBox *cb = new QComboBox();
        cb->setEditable( true );

        for ( QList<QVariant>::iterator it = values.begin(); it != values.end(); it++ )
          cb->addItem( it->toString() );

        int idx = cb->findText( myFieldValue.toString() );
        if ( idx >= 0 )
          cb->setCurrentIndex( idx );

        myWidget = cb;
      }
      break;

      case QgsVectorLayer::Enumeration:
      {
        QStringList enumValues;
        mLayer->dataProvider()->enumValues( it.key(), enumValues );

        QComboBox *cb = new QComboBox();
        QStringList::const_iterator s_it = enumValues.constBegin();
        for ( ; s_it != enumValues.constEnd(); ++s_it )
        {
          cb->addItem( *s_it );
        }
        int idx = cb->findText( myFieldValue.toString() );
        if ( idx >= 0 )
        {
          cb->setCurrentIndex( idx );
        }
        myWidget = cb;
      }
      break;

      case QgsVectorLayer::ValueMap:
      {
        const QMap<QString, QVariant> &map = vl->valueMap( it.key() );

        QComboBox *cb = new QComboBox();

        for ( QMap<QString, QVariant>::const_iterator it = map.begin(); it != map.end(); it++ )
        {
          cb->addItem( it.key(), it.value() );
        }

        int idx = cb->findData( myFieldValue );
        if ( idx >= 0 )
          cb->setCurrentIndex( idx );

        myWidget = cb;
      }
      break;

      case QgsVectorLayer::Classification:
      {
        QComboBox *cb = new QComboBox();
        for ( QMap<QString, QString>::const_iterator it = classes.begin(); it != classes.end(); it++ )
        {
          cb->addItem( it.value(), it.key() );
        }

        int idx = cb->findData( myFieldValue );
        if ( idx >= 0 )
          cb->setCurrentIndex( idx );

        myWidget = cb;
      }
      break;

      case QgsVectorLayer::SliderRange:
      case QgsVectorLayer::EditRange:
      {
        if ( myFieldType == QVariant::Int )
        {
          int min = vl->range( it.key() ).mMin.toInt();
          int max = vl->range( it.key() ).mMax.toInt();
          int step = vl->range( it.key() ).mStep.toInt();

          if ( editType == QgsVectorLayer::EditRange )
          {
            QSpinBox *sb = new QSpinBox();

            sb->setRange( min, max );
            sb->setSingleStep( step );
            sb->setValue( it.value().toInt() );

            myWidget = sb;
          }
          else
          {
            QSlider *sl = new QSlider( Qt::Horizontal );

            sl->setRange( min, max );
            sl->setSingleStep( step );
            sl->setValue( it.value().toInt() );

            myWidget = sl;
          }
          break;
        }
        else if ( myFieldType == QVariant::Double )
        {
          double min = vl->range( it.key() ).mMin.toDouble();
          double max = vl->range( it.key() ).mMax.toDouble();
          double step = vl->range( it.key() ).mStep.toDouble();
          QDoubleSpinBox *dsb = new QDoubleSpinBox();

          dsb->setRange( min, max );
          dsb->setSingleStep( step );
          dsb->setValue( it.value().toDouble() );

          myWidget = dsb;
          break;
        }
      }

      // fall-through

      case QgsVectorLayer::LineEdit:
      case QgsVectorLayer::UniqueValuesEditable:
      case QgsVectorLayer::Immutable:
      default:
      {
        QLineEdit *le = new QLineEdit( myFieldValue.toString() );

        if ( editType == QgsVectorLayer::Immutable )
        {
          le->setEnabled( false );
        }
        if ( editType == QgsVectorLayer::UniqueValuesEditable )
        {
          QList<QVariant> values;
          mLayer->dataProvider()->uniqueValues( it.key(), values );

          QStringList svalues;
          for ( QList<QVariant>::const_iterator it = values.begin(); it != values.end(); it++ )
            svalues << it->toString();

          QCompleter *c = new QCompleter( svalues );
          c->setCompletionMode( QCompleter::PopupCompletion );
          le->setCompleter( c );
        }

        if ( myFieldType == QVariant::Int )
        {
          le->setValidator( new QIntValidator( le ) );
        }
        else if ( myFieldType == QVariant::Double )
        {
          le->setValidator( new QDoubleValidator( le ) );
        }

        myWidget = le;
      }
      break;

      case QgsVectorLayer::FileName:
      {
        QLineEdit *le = new QLineEdit( myFieldValue.toString() );

        QPushButton *pb = new QPushButton( tr( "..." ) );
        connect( pb, SIGNAL( clicked() ), this, SLOT( selectFileName() ) );

        QHBoxLayout *hbl = new QHBoxLayout();
        hbl->addWidget( le );
        hbl->addWidget( pb );

        myWidget = new QWidget;
        myWidget->setLayout( hbl );
      }
      break;
    }

    if ( myFieldType == QVariant::Int )
    {
      mypLabel->setText( myFieldName + tr( " (int)" ) );
    }
    else if ( myFieldType == QVariant::Double )
    {
      mypLabel->setText( myFieldName + tr( " (dbl)" ) );
    }
    else //string
    {
      //any special behaviour for string goes here
      mypLabel->setText( myFieldName + tr( " (txt)" ) );
    }

    mypInnerLayout->addWidget( myWidget, index, 1 );
    mpWidgets << myWidget;
    ++index;
  }
  // Set focus to first widget in list, to help entering data without moving the mouse.
  if ( mpWidgets.size() > 0 )
  {
    mpWidgets.first()->setFocus( Qt::OtherFocusReason );
  }
  restoreGeometry();
}


QgsAttributeDialog::~QgsAttributeDialog()
{
  saveGeometry();
}

void QgsAttributeDialog::selectFileName()
{
  QPushButton *pb = dynamic_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  QWidget *hbox = dynamic_cast<QWidget *>( pb->parent() );
  if ( !hbox )
    return;

  QLineEdit *le = hbox->findChild<QLineEdit *>();
  if ( !le )
    return;

  QString fileName = QFileDialog::getOpenFileName( 0 , tr( "Select a file" ) );
  if ( fileName.isNull() )
    return;

  le->setText( fileName );
}

void QgsAttributeDialog::accept()
{
  //write the new values back to the feature
  QgsAttributeMap myAttributes = mpFeature->attributeMap();
  int myIndex = 0;
  for ( QgsAttributeMap::const_iterator it = myAttributes.begin();
        it != myAttributes.end();
        ++it )
  {
    const QgsField &theField = mLayer->pendingFields()[it.key()];
    QgsVectorLayer::EditType editType = mLayer->editType( it.key() );
    QString myFieldName = theField.name();
    bool myFlag = false;
    QString myFieldValue;
    bool modified = true;

    QLineEdit *le = dynamic_cast<QLineEdit *>( mpWidgets.value( myIndex ) );
    if ( le )
    {
      myFieldValue = le->text();
      modified = le->isModified();
    }

    QComboBox *cb = dynamic_cast<QComboBox *>( mpWidgets.value( myIndex ) );
    if ( cb )
    {
      if ( editType == QgsVectorLayer::UniqueValues ||
           editType == QgsVectorLayer::ValueMap ||
           editType == QgsVectorLayer::Classification )
      {
        myFieldValue = cb->itemData( cb->currentIndex() ).toString();
      }
      else
      {
        myFieldValue = cb->currentText();
      }
    }

    QSpinBox *sb = dynamic_cast<QSpinBox *>( mpWidgets.value( myIndex ) );
    if ( sb )
    {
      myFieldValue = QString::number( sb->value() );
    }

    QSlider *slider = dynamic_cast<QSlider *>( mpWidgets.value( myIndex ) );
    if ( slider )
    {
      myFieldValue = QString::number( slider->value() );
    }

    QDoubleSpinBox *dsb = dynamic_cast<QDoubleSpinBox *>( mpWidgets.value( myIndex ) );
    if ( dsb )
    {
      myFieldValue = QString::number( dsb->value() );
    }

    le = mpWidgets.value( myIndex )->findChild<QLineEdit *>();
    if ( le )
    {
      myFieldValue = le->text();
    }

    switch ( theField.type() )
    {
      case QVariant::Int:
      {
        int myIntValue = myFieldValue.toInt( &myFlag );
        if ( myFlag && ! myFieldValue.isEmpty() )
        {
          mpFeature->changeAttribute( it.key(), QVariant( myIntValue ) );
        }
        else if ( modified )
        {
          mpFeature->changeAttribute( it.key(), QVariant( QString::null ) );
        }
        else
        {
          mpFeature->changeAttribute( it.key(), myFieldValue );
        }
      }
      break;
      case QVariant::Double:
      {
        double myDblValue = myFieldValue.toDouble( &myFlag );
        if ( myFlag && ! myFieldValue.isEmpty() )
        {
          mpFeature->changeAttribute( it.key(), QVariant( myDblValue ) );
        }
        else if ( modified )
        {
          mpFeature->changeAttribute( it.key(), QVariant( QString::null ) );
        }
        else
        {
          mpFeature->changeAttribute( it.key(), myFieldValue );
        }
      }
      break;
      default: //string
        mpFeature->changeAttribute( it.key(), QVariant( myFieldValue ) );
        break;
    }
    ++myIndex;
  }
  QDialog::accept();
}

void QgsAttributeDialog::saveGeometry()
{
  QSettings settings;
  settings.setValue( mSettingsPath + "geometry", QDialog::saveGeometry() );
}

void QgsAttributeDialog::restoreGeometry()
{
  QSettings settings;
  QDialog::restoreGeometry( settings.value( mSettingsPath + "geometry" ).toByteArray() );
}
