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
#include <qgscategorizedsymbolrendererv2.h>
#include <qgssymbol.h>
#include <qgslonglongvalidator.h>
#include <qgsfieldvalidator.h>

#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QCompleter>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QDial>
#include <QCalendarWidget>
#include <QDialogButtonBox>
#include <QSettings>
#include <QDir>

void QgsAttributeEditor::selectFileName()
{
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  QWidget *hbox = qobject_cast<QWidget *>( pb->parent() );
  if ( !hbox )
    return;

  QLineEdit *le = hbox->findChild<QLineEdit *>();
  if ( !le )
    return;

  QString fileName = QFileDialog::getOpenFileName( 0 , tr( "Select a file" ) );
  if ( fileName.isNull() )
    return;

  //le->setText( fileName );
  le->setText( QDir::toNativeSeparators( fileName ) );
}

void QgsAttributeEditor::selectDate()
{
  QPushButton *pb = qobject_cast<QPushButton *>( sender() );
  if ( !pb )
    return;

  QWidget *hbox = qobject_cast<QWidget *>( pb->parent() );
  if ( !hbox )
    return;

  QLineEdit *le = hbox->findChild<QLineEdit *>();
  if ( !le )
    return;

  QDialog *dlg = new QDialog();
  dlg->setWindowTitle( tr( "Select a date" ) );
  QVBoxLayout *vl = new QVBoxLayout( dlg );

  QCalendarWidget *cw = new QCalendarWidget( dlg );
  cw->setSelectedDate( QDate::fromString( le->text(), Qt::ISODate ) );
  vl->addWidget( cw );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( dlg );
  buttonBox->addButton( QDialogButtonBox::Ok );
  buttonBox->addButton( QDialogButtonBox::Cancel );
  vl->addWidget( buttonBox );

  connect( buttonBox, SIGNAL( accepted() ), dlg, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), dlg, SLOT( reject() ) );

  if ( dlg->exec() == QDialog::Accepted )
  {
    le->setText( cw->selectedDate().toString( Qt::ISODate ) );
  }
}

QComboBox *QgsAttributeEditor::comboBox( QWidget *editor, QWidget *parent )
{
  QComboBox *cb = NULL;
  if ( editor )
    cb = qobject_cast<QComboBox *>( editor );
  else
    cb = new QComboBox( parent );

  return cb;
}

QWidget *QgsAttributeEditor::createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value )
{
  if ( !vl )
    return NULL;

  QWidget *myWidget = NULL;
  QgsVectorLayer::EditType editType = vl->editType( idx );
  const QgsField &field = vl->pendingFields()[idx];
  QVariant::Type myFieldType = field.type();

  switch ( editType )
  {
    case QgsVectorLayer::UniqueValues:
    {
      QList<QVariant> values;
      vl->dataProvider()->uniqueValues( idx, values );

      QComboBox *cb = comboBox( editor, parent );
      if ( cb )
      {
        cb->setEditable( false );

        for ( QList<QVariant>::iterator it = values.begin(); it != values.end(); it++ )
          cb->addItem( it->toString(), it->toString() );

        myWidget = cb;
      }
    }
    break;

    case QgsVectorLayer::Enumeration:
    {
      QStringList enumValues;
      vl->dataProvider()->enumValues( idx, enumValues );

      QComboBox *cb = comboBox( editor, parent );
      if ( cb )
      {
        QStringList::const_iterator s_it = enumValues.constBegin();
        for ( ; s_it != enumValues.constEnd(); ++s_it )
        {
          cb->addItem( *s_it, *s_it );
        }

        myWidget = cb;
      }
    }
    break;

    case QgsVectorLayer::ValueMap:
    {
      const QMap<QString, QVariant> &map = vl->valueMap( idx );

      QComboBox *cb = comboBox( editor, parent );
      if ( cb )
      {
        for ( QMap<QString, QVariant>::const_iterator it = map.begin(); it != map.end(); it++ )
        {
          cb->addItem( it.key(), it.value() );
        }

        myWidget = cb;
      }
    }
    break;

    case QgsVectorLayer::Classification:
    {
      QMap<QString, QString> classes;

      const QgsUniqueValueRenderer *uvr = dynamic_cast<const QgsUniqueValueRenderer *>( vl->renderer() );
      if ( uvr )
      {
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

      const QgsCategorizedSymbolRendererV2 *csr = dynamic_cast<const QgsCategorizedSymbolRendererV2 *>( vl->rendererV2() );
      if ( csr )
      {
        const QgsCategoryList &categories = (( QgsCategorizedSymbolRendererV2 * )csr )->categories(); // FIXME: QgsCategorizedSymbolRendererV2::categories() should be const
        for ( int i = 0; i < categories.size(); i++ )
        {
          QString label = categories[i].label();
          QString value = categories[i].value().toString();
          if ( label.isEmpty() )
            label = value;
          classes.insert( value, label );
        }
      }

      QComboBox *cb = comboBox( editor, parent );
      if ( cb )
      {
        for ( QMap<QString, QString>::const_iterator it = classes.begin(); it != classes.end(); it++ )
        {
          cb->addItem( it.value(), it.key() );
        }

        myWidget = cb;
      }
    }
    break;


    case QgsVectorLayer::DialRange:
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
          QSpinBox *sb = NULL;

          if ( editor )
            sb = qobject_cast<QSpinBox *>( editor );
          else
            sb = new QSpinBox( parent );

          if ( sb )
          {
            sb->setRange( min, max );
            sb->setSingleStep( step );

            myWidget = sb;
          }
        }
        else
        {
          QAbstractSlider *sl = NULL;

          if ( editor )
          {
            sl = qobject_cast<QAbstractSlider*>( editor );
          }
          else if ( editType == QgsVectorLayer::DialRange )
          {
            sl = new QDial( parent );
          }
          else
          {
            sl = new QSlider( Qt::Horizontal, parent );
          }

          if ( sl )
          {
            sl->setRange( min, max );
            sl->setSingleStep( step );

            myWidget = sl;
          }
        }
        break;
      }
      else if ( myFieldType == QVariant::Double )
      {
        QDoubleSpinBox *dsb = NULL;
        if ( editor )
          dsb = qobject_cast<QDoubleSpinBox*>( editor );
        else
          dsb = new QDoubleSpinBox( parent );

        if ( dsb )
        {
          double min = vl->range( idx ).mMin.toDouble();
          double max = vl->range( idx ).mMax.toDouble();
          double step = vl->range( idx ).mStep.toDouble();

          dsb->setRange( min, max );
          dsb->setSingleStep( step );

          myWidget = dsb;
        }
        break;
      }
    }

    case QgsVectorLayer::CheckBox:
    {
      QCheckBox *cb = NULL;
      if ( editor )
        cb = qobject_cast<QCheckBox*>( editor );
      else
        cb = new QCheckBox( parent );

      if ( cb )
      {
        myWidget = cb;
        break;
      }
    }

    // fall-through

    case QgsVectorLayer::LineEdit:
    case QgsVectorLayer::TextEdit:
    case QgsVectorLayer::UniqueValuesEditable:
    case QgsVectorLayer::Immutable:
    {
      QLineEdit *le = NULL;
      QTextEdit *te = NULL;
      QPlainTextEdit *pte = NULL;

      if ( editor )
      {
        le = qobject_cast<QLineEdit *>( editor );
        te = qobject_cast<QTextEdit *>( editor );
        pte = qobject_cast<QPlainTextEdit *>( editor );
      }
      else if ( editType == QgsVectorLayer::TextEdit )
      {
        pte = new QPlainTextEdit( parent );
      }
      else
      {
        le = new QLineEdit( parent );
      }

      if ( le )
      {

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

        le->setValidator( new QgsFieldValidator( le, field ) );
        myWidget = le;
      }

      if ( te )
      {
        te->setAcceptRichText( true );
        myWidget = te;
      }

      if ( pte )
      {
        myWidget = pte;
      }

      if ( myWidget )
      {
        myWidget->setDisabled( editType == QgsVectorLayer::Immutable );
      }
    }
    break;

    case QgsVectorLayer::Hidden:
      myWidget = NULL;
      break;

    case QgsVectorLayer::FileName:
    case QgsVectorLayer::Calendar:
    {
      QPushButton *pb = NULL;
      QLineEdit *le = qobject_cast<QLineEdit *>( editor );
      if ( le )
      {
        if ( le )
          myWidget = le;

        if ( editor->parent() )
        {
          pb = editor->parent()->findChild<QPushButton *>();
        }
      }
      else
      {
        le = new QLineEdit();

        pb = new QPushButton( tr( "..." ) );

        QHBoxLayout *hbl = new QHBoxLayout();
        hbl->addWidget( le );
        hbl->addWidget( pb );

        myWidget = new QWidget( parent );
        myWidget->setBackgroundRole( QPalette::Window );
        myWidget->setAutoFillBackground( true );
        myWidget->setLayout( hbl );
      }

      if ( pb )
      {
        if ( editType == QgsVectorLayer::FileName )
          connect( pb, SIGNAL( clicked() ), new QgsAttributeEditor( pb ), SLOT( selectFileName() ) );
        if ( editType == QgsVectorLayer::Calendar )
          connect( pb, SIGNAL( clicked() ), new QgsAttributeEditor( pb ), SLOT( selectDate() ) );
      }
    }
    break;
  }

  setValue( myWidget, vl, idx, value );

  return myWidget;
}

bool QgsAttributeEditor::retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value )
{
  if ( !widget )
    return false;

  const QgsField &theField = vl->pendingFields()[idx];
  QgsVectorLayer::EditType editType = vl->editType( idx );
  bool modified = false;
  QString text;

  QSettings settings;
  QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();

  QLineEdit *le = qobject_cast<QLineEdit *>( widget );
  if ( le )
  {
    text = le->text();
    modified = le->isModified();
    if ( text == nullValue )
    {
      text = QString::null;
    }
  }

  QTextEdit *te = qobject_cast<QTextEdit *>( widget );
  if ( te )
  {
    text = te->toHtml();
    modified = te->document()->isModified();
    if ( text == nullValue )
    {
      text = QString::null;
    }
  }

  QPlainTextEdit *pte = qobject_cast<QPlainTextEdit *>( widget );
  if ( pte )
  {
    text = pte->toPlainText();
    modified = pte->document()->isModified();
    if ( text == nullValue )
    {
      text = QString::null;
    }
  }

  QComboBox *cb = qobject_cast<QComboBox *>( widget );
  if ( cb )
  {
    if ( editType == QgsVectorLayer::UniqueValues ||
         editType == QgsVectorLayer::ValueMap ||
         editType == QgsVectorLayer::Classification )
    {
      text = cb->itemData( cb->currentIndex() ).toString();
      if ( text == nullValue )
      {
        text = QString::null;
      }
    }
    else
    {
      text = cb->currentText();
    }
    modified = true;
  }

  QSpinBox *sb = qobject_cast<QSpinBox *>( widget );
  if ( sb )
  {
    text = QString::number( sb->value() );
  }

  QAbstractSlider *slider = qobject_cast<QAbstractSlider *>( widget );
  if ( slider )
  {
    text = QString::number( slider->value() );
  }

  QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox *>( widget );
  if ( dsb )
  {
    text = QString::number( dsb->value() );
  }

  QCheckBox *ckb = qobject_cast<QCheckBox *>( widget );
  if ( ckb )
  {
    QPair<QString, QString> states = vl->checkedState( idx );
    text = ckb->isChecked() ? states.first : states.second;
  }

  QCalendarWidget *cw = qobject_cast<QCalendarWidget *>( widget );
  if ( cw )
  {
    text = cw->selectedDate().toString();
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
        value = QVariant();
      }
    }
    break;
    case QVariant::LongLong:
    {
      bool ok;
      qlonglong myLongValue = text.toLong( &ok );
      if ( ok && !text.isEmpty() )
      {
        value = QVariant( myLongValue );
        modified = true;
      }
      else if ( modified )
      {
        value = QVariant();
      }
    }
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
        value = QVariant();
      }
    }
    break;
    case QVariant::Date:
    {
      QDate myDateValue = QDate::fromString( text, Qt::ISODate );
      if ( myDateValue.isValid() && !text.isEmpty() )
      {
        value = myDateValue;
        modified = true;
      }
      else if ( modified )
      {
        value = QVariant();
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
  if ( !editor )
    return false;

  QgsVectorLayer::EditType editType = vl->editType( idx );
  const QgsField &field = vl->pendingFields()[idx];
  QVariant::Type myFieldType = field.type();

  QSettings settings;
  QString nullValue = settings.value( "qgis/nullValue", "NULL" ).toString();

  switch ( editType )
  {
    case QgsVectorLayer::Classification:
    case QgsVectorLayer::UniqueValues:
    case QgsVectorLayer::Enumeration:
    case QgsVectorLayer::ValueMap:
    {
      QVariant v = value;
      QComboBox *cb = qobject_cast<QComboBox *>( editor );
      if ( cb == NULL )
        return false;

      if ( v.isNull() )
      {
        v = nullValue;
      }

      int idx = cb->findData( v );
      if ( idx < 0 )
        return false;

      cb->setCurrentIndex( idx );
    }
    break;

    case QgsVectorLayer::DialRange:
    case QgsVectorLayer::SliderRange:
    case QgsVectorLayer::EditRange:
    {
      if ( myFieldType == QVariant::Int )
      {
        if ( editType == QgsVectorLayer::EditRange )
        {
          QSpinBox *sb = qobject_cast<QSpinBox *>( editor );
          if ( sb == NULL )
            return false;
          sb->setValue( value.toInt() );
        }
        else
        {
          QAbstractSlider *sl = qobject_cast<QAbstractSlider *>( editor );
          if ( sl == NULL )
            return false;
          sl->setValue( value.toInt() );
        }
        break;
      }
      else if ( myFieldType == QVariant::Double )
      {
        QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox *>( editor );
        if ( dsb == NULL )
          return false;
        dsb->setValue( value.toDouble() );
      }
    }

    case QgsVectorLayer::CheckBox:
    {
      QCheckBox *cb = qobject_cast<QCheckBox *>( editor );
      if ( cb )
      {
        QPair<QString, QString> states = vl->checkedState( idx );
        cb->setChecked( value == states.first );
        break;
      }
    }

    // fall-through

    case QgsVectorLayer::LineEdit:
    case QgsVectorLayer::UniqueValuesEditable:
    case QgsVectorLayer::Immutable:
    default:
    {
      QLineEdit *le = qobject_cast<QLineEdit *>( editor );
      QTextEdit *te = qobject_cast<QTextEdit *>( editor );
      QPlainTextEdit *pte = qobject_cast<QPlainTextEdit *>( editor );
      if ( !le && !te && !pte )
        return false;

      QString text;
      if ( value.isNull() )
        if ( myFieldType == QVariant::Int || myFieldType == QVariant::Double || myFieldType == QVariant::LongLong )
          text = "";
        else
          text = nullValue;
      else
        text = value.toString();

      if ( le )
        le->setText( text );
      if ( te )
        te->setHtml( text );
      if ( pte )
        pte->setPlainText( text );
    }
    break;

    case QgsVectorLayer::FileName:
    case QgsVectorLayer::Calendar:
    {
      QLineEdit* le = qobject_cast<QLineEdit*>( editor );
      if( !le )
      {
        le = editor->findChild<QLineEdit *>();
      }
      if ( !le )
      {
        return false;
      }
      le->setText( value.toString() );
    }
    break;
  }

  return true;
}
