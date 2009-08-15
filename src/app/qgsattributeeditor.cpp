/***************************************************************************
                         qgsattributeeditor.cpp  -  description
                             -------------------
    begin                : July 2009
    copyright            : (C) 2009 by Jürgen E. Fischer
    email                : jef@norbit.de
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

#include "qgsattributeeditor.h"
#include <qgsvectorlayer.h>
#include <qgsvectordataprovider.h>
#include <qgsuniquevaluerenderer.h>
#include <qgssymbol.h>

#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCompleter>
#include <QHBoxLayout>

void QgsAttributeEditor::selectFileName( void )
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

QWidget *QgsAttributeEditor::createAttributeEditor( QWidget *parent, QgsVectorLayer *vl, int idx, const QVariant &value )
{
  if ( !vl )
    return NULL;

  QWidget *myWidget;
  QgsVectorLayer::EditType editType = vl->editType( idx );
  const QgsField &field = vl->pendingFields()[idx];
  QVariant::Type myFieldType = field.type();

  switch ( editType )
  {
    case QgsVectorLayer::UniqueValues:
    {
      QList<QVariant> values;
      vl->dataProvider()->uniqueValues( idx, values );

      QComboBox *cb = new QComboBox( parent );
      cb->setEditable( true );

      for ( QList<QVariant>::iterator it = values.begin(); it != values.end(); it++ )
        cb->addItem( it->toString() );

      myWidget = cb;
    }
    break;

    case QgsVectorLayer::Enumeration:
    {
      QStringList enumValues;
      vl->dataProvider()->enumValues( idx, enumValues );

      QComboBox *cb = new QComboBox( parent );
      QStringList::const_iterator s_it = enumValues.constBegin();
      for ( ; s_it != enumValues.constEnd(); ++s_it )
      {
        cb->addItem( *s_it );
      }

      myWidget = cb;
    }
    break;

    case QgsVectorLayer::ValueMap:
    {
      const QMap<QString, QVariant> &map = vl->valueMap( idx );

      QComboBox *cb = new QComboBox( parent );

      for ( QMap<QString, QVariant>::const_iterator it = map.begin(); it != map.end(); it++ )
      {
        cb->addItem( it.key(), it.value() );
      }

      myWidget = cb;
    }
    break;

    case QgsVectorLayer::Classification:
    {
      int classificationField = -1;
      QMap<QString, QString> classes;

      const QgsUniqueValueRenderer *uvr = dynamic_cast<const QgsUniqueValueRenderer *>( vl->renderer() );
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

      QComboBox *cb = new QComboBox( parent );
      for ( QMap<QString, QString>::const_iterator it = classes.begin(); it != classes.end(); it++ )
      {
        cb->addItem( it.value(), it.key() );
      }

      myWidget = cb;
    }
    break;

    case QgsVectorLayer::SliderRange:
    case QgsVectorLayer::EditRange:
    {
      if ( myFieldType == QVariant::Int )
      {
        int min = vl->range( idx ).mMin.toInt();
        int max = vl->range( idx ).mMax.toInt();
        int step = vl->range( idx ).mStep.toInt();

        if ( editType == QgsVectorLayer::EditRange )
        {
          QSpinBox *sb = new QSpinBox( parent );

          sb->setRange( min, max );
          sb->setSingleStep( step );

          myWidget = sb;
        }
        else
        {
          QSlider *sl = new QSlider( Qt::Horizontal, parent );

          sl->setRange( min, max );
          sl->setSingleStep( step );

          myWidget = sl;
        }
        break;
      }
      else if ( myFieldType == QVariant::Double )
      {
        double min = vl->range( idx ).mMin.toDouble();
        double max = vl->range( idx ).mMax.toDouble();
        double step = vl->range( idx ).mStep.toDouble();
        QDoubleSpinBox *dsb = new QDoubleSpinBox( parent );

        dsb->setRange( min, max );
        dsb->setSingleStep( step );

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
      QLineEdit *le = new QLineEdit( parent );

      if ( editType == QgsVectorLayer::Immutable )
      {
        le->setEnabled( false );
      }

      if ( editType == QgsVectorLayer::UniqueValuesEditable )
      {
        QList<QVariant> values;
        vl->dataProvider()->uniqueValues( idx, values );

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
      QLineEdit *le = new QLineEdit();

      QPushButton *pb = new QPushButton( tr( "..." ) );
      connect( pb, SIGNAL( clicked() ), new QgsAttributeEditor( pb ), SLOT( selectFileName() ) );

      QHBoxLayout *hbl = new QHBoxLayout();
      hbl->addWidget( le );
      hbl->addWidget( pb );

      myWidget = new QWidget( parent );
      myWidget->setLayout( hbl );
    }
    break;
  }

  setValue( myWidget, vl, idx, value );

  return myWidget;
}

bool QgsAttributeEditor::retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value )
{
  const QgsField &theField = vl->pendingFields()[idx];
  QgsVectorLayer::EditType editType = vl->editType( idx );
  bool modified = false;
  QString text;

  QLineEdit *le = dynamic_cast<QLineEdit *>( widget );
  if ( le )
  {
    text = le->text();
    modified = le->isModified();
    if ( text == "NULL" )
    {
      text = QString::null;
    }
  }

  QComboBox *cb = dynamic_cast<QComboBox *>( widget );
  if ( cb )
  {
    if ( editType == QgsVectorLayer::UniqueValues ||
         editType == QgsVectorLayer::ValueMap ||
         editType == QgsVectorLayer::Classification )
    {
      text = cb->itemData( cb->currentIndex() ).toString();
    }
    else
    {
      text = cb->currentText();
    }
  }

  QSpinBox *sb = dynamic_cast<QSpinBox *>( widget );
  if ( sb )
  {
    text = QString::number( sb->value() );
  }

  QSlider *slider = dynamic_cast<QSlider *>( widget );
  if ( slider )
  {
    text = QString::number( slider->value() );
  }
  QDoubleSpinBox *dsb = dynamic_cast<QDoubleSpinBox *>( widget );
  if ( dsb )
  {
    text = QString::number( dsb->value() );
  }

  le = widget->findChild<QLineEdit *>();
  if ( le )
  {
    text = le->text();
  }

  switch ( theField.type() )
  {
    case QVariant::Int:
    {
      bool ok;
      int myIntValue = text.toInt( &ok );
      if ( ok && !text.isEmpty() )
      {
        value = QVariant( myIntValue );
        modified = true;
      }
      else if ( modified )
      {
        value = QVariant( theField.type() );
      }
    }
    break;
    case QVariant::Double:
    {
      bool ok;
      double myDblValue = text.toDouble( &ok );
      if ( ok && !text.isEmpty() )
      {
        value = QVariant( myDblValue );
        modified = true;
      }
      else if ( modified )
      {
        value = QVariant( theField.type() );
      }
    }
    break;
    default: //string
      modified = true;
      value = QVariant( text );
      break;
  }

  return modified;
}

bool QgsAttributeEditor::setValue( QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value )
{
  QgsVectorLayer::EditType editType = vl->editType( idx );
  const QgsField &field = vl->pendingFields()[idx];
  QVariant::Type myFieldType = field.type();

  switch ( editType )
  {
    case QgsVectorLayer::UniqueValues:
    case QgsVectorLayer::Enumeration:
    case QgsVectorLayer::ValueMap:
    {
      QComboBox *cb = dynamic_cast<QComboBox *>( editor );
      if ( cb == NULL )
        return false;
      int idx = cb->findText( value.toString() );
      if ( idx < 0 )
        return false;

      cb->setCurrentIndex( idx );
    }
    break;

    case QgsVectorLayer::SliderRange:
    case QgsVectorLayer::EditRange:
    {
      if ( myFieldType == QVariant::Int )
      {
        if ( editType == QgsVectorLayer::EditRange )
        {
          QSpinBox *sb = dynamic_cast<QSpinBox*>( editor );
          if ( sb == NULL )
            return false;
          sb->setValue( value.toInt() );
        }
        else
        {
          QSlider *sl = dynamic_cast<QSlider *>( editor );
          if ( sl == NULL )
            return false;
          sl->setValue( value.toInt() );
        }
        break;
      }
      else if ( myFieldType == QVariant::Double )
      {
        QDoubleSpinBox *dsb = dynamic_cast<QDoubleSpinBox*>( editor );
        if ( dsb == NULL )
          return false;
        dsb->setValue( value.toDouble() );
      }
    }

    // fall-through

    case QgsVectorLayer::LineEdit:
    case QgsVectorLayer::UniqueValuesEditable:
    case QgsVectorLayer::Immutable:
    default:
    {
      QLineEdit *le = dynamic_cast<QLineEdit*>( editor );
      if ( le == NULL )
        return false;

      QString text;
      if ( value.isNull() )
        if ( myFieldType == QVariant::Int || myFieldType == QVariant::Double )
          text = "";
        else
          text = "NULL";
      else
        text = value.toString();

      le->setText( text );
    }
    break;

    case QgsVectorLayer::FileName:
    {
      QLineEdit *le = editor->findChild<QLineEdit *>();
      if ( le == NULL )
        return false;
      le->setText( value.toString() );
    }
    break;
  }

  return true;
}
