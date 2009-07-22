/***************************************************************************
     QgsAttributeTableDelegate.cpp
     --------------------------------------
    Date                 : Feb 2009
    Copyright            : (C) 2009 Vita Cizek
    Email                : weetya (at) gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QItemDelegate>
#include <QLineEdit>
#include <QComboBox>
#include <QPainter>
#include <QCompleter>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "qgsattributetableview.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsattributetabledelegate.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsuniquevaluerenderer.h"
#include "qgssymbol.h"


QWidget * QgsAttributeTableDelegate::createEditor(
  QWidget *parent,
  const QStyleOptionViewItem &option,
  const QModelIndex &index ) const
{
  QWidget *editor;

  const QgsAttributeTableFilterModel* fm = dynamic_cast<const QgsAttributeTableFilterModel*>( index.model() );
  if ( !fm )
  {
      return editor;
  }
  const QgsAttributeTableModel* m = dynamic_cast<const QgsAttributeTableModel*>( fm->sourceModel() );
  if ( !m )
  {
      return editor;
  }

  int col = index.column();
  QVariant::Type type = m->layer()->dataProvider()->fields()[col].type();
  QgsVectorLayer::EditType editType = m->layer()->editType(col);

  //need to created correct edit widget according to edit type of value
  //and fill with data from correct source
  if (editType == QgsVectorLayer::LineEdit ||
      editType == QgsVectorLayer::UniqueValuesEditable ||
      editType == QgsVectorLayer::FileName ||
      editType == QgsVectorLayer::Immutable)
  { //these are all siple edits
    editor = new QLineEdit(parent);
    QLineEdit* le = dynamic_cast<QLineEdit*> ( editor );
    le-> setReadOnly ( false );

    if ( editType == QgsVectorLayer::UniqueValuesEditable )
    { //just this value has completer
      QList<QVariant> values;
      m->layer()->dataProvider()->uniqueValues( col, values );

      QStringList svalues;
      for ( QList<QVariant>::const_iterator it = values.begin(); it != values.end(); it++ )
        svalues << it->toString();

      QCompleter *c = new QCompleter( svalues );
      c->setCompletionMode( QCompleter::PopupCompletion );
      le->setCompleter( c );
    }
    if (editType == QgsVectorLayer::Immutable)
    {
      le->setReadOnly(true);
    }
    //validators if value needs it
    if ( type == QVariant::Int )
    {
      le->setValidator( new QIntValidator( le ) );
    }
    else if ( type == QVariant::Double )
    {
      le->setValidator( new QDoubleValidator( le ) );
    }
  }
  else if (editType == QgsVectorLayer::ValueMap)
  { //simple combobox from data from vector layer
    editor = new QComboBox(parent);
    QComboBox* cb = dynamic_cast<QComboBox*>( editor );
    QMap<QString, QVariant> &map = m->layer()->valueMap(col);
    QMap<QString, QVariant>::iterator it = map.begin();
    for ( ; it != map.end(); it ++)
    {
      cb->addItem( it.key() ,it.value());
    }
  }
  else if (editType == QgsVectorLayer::SliderRange)
  { //horizontal slider
    editor = new QSlider(Qt::Horizontal, parent);
    QSlider* s = dynamic_cast<QSlider*>(editor );
    QgsVectorLayer::RangeData &range = m->layer()->range(col);
    s->setMinimum( range.mMin.toInt() );
    s->setMaximum( range.mMax.toInt() );
    s->setPageStep( range.mStep.toInt() );
  }
  else if (editType == QgsVectorLayer::Classification)
  {
    // should be prepared probably to not change it always
    int classificationField = -1;
    QMap<QString, QString> classes;
    const QgsUniqueValueRenderer *uvr = dynamic_cast<const QgsUniqueValueRenderer *>( m->layer()->renderer() );
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
    editor = new QComboBox(parent);
    QComboBox* cb = dynamic_cast<QComboBox*>( editor );
    for ( QMap<QString, QString>::const_iterator it = classes.begin(); it != classes.end(); it++ )
    {
      cb->addItem( it.value(), it.key() );
    }
  }
  else if (editType == QgsVectorLayer::UniqueValues)
  {
    QList<QVariant> values;
    m->layer()->dataProvider()->uniqueValues( col, values );

    editor = new QComboBox(parent);
    QComboBox* cb = dynamic_cast<QComboBox*>( editor );
    cb->setEditable( true );

    for ( QList<QVariant>::iterator it = values.begin(); it != values.end(); it++ )
      cb->addItem( it->toString() );

  }
  else if (editType == QgsVectorLayer::Enumeration)
  {
    QStringList enumValues;
    m->layer()->dataProvider()->enumValues( col, enumValues );

    editor = new QComboBox(parent);
    QComboBox* cb = dynamic_cast<QComboBox*>( editor );
    QStringList::const_iterator s_it = enumValues.constBegin();
    for ( ; s_it != enumValues.constEnd(); ++s_it )
    {
      cb->addItem( *s_it );
    }
  }
  else if (editType == QgsVectorLayer::EditRange)
  {
    if ( type == QVariant::Int )
    {
      int min = m->layer()->range( col ).mMin.toInt();
      int max = m->layer()->range( col ).mMax.toInt();
      int step = m->layer()->range( col ).mStep.toInt();
      editor = new QSpinBox(parent);
      QSpinBox* sb = dynamic_cast<QSpinBox*>( editor );

      sb->setRange( min, max );
      sb->setSingleStep( step );

    }
    else if ( type == QVariant::Double )
    {
      double min = m->layer()->range( col ).mMin.toDouble();
      double max = m->layer()->range( col ).mMax.toDouble();
      double step = m->layer()->range( col ).mStep.toDouble();
      editor = new QDoubleSpinBox(parent);
      QDoubleSpinBox* dsb = dynamic_cast<QDoubleSpinBox*>( editor );

      dsb->setRange( min, max );
      dsb->setSingleStep( step );
    }
  }


  return editor;
}


void QgsAttributeTableDelegate::paint( QPainter * painter,
                                       const QStyleOptionViewItem & option,
                                       const QModelIndex & index ) const
{
  QItemDelegate::paint( painter, option, index );

  if ( option.state & QStyle::State_HasFocus )
  {
    QRect r = option.rect.adjusted( 1, 1, -1, -1 );
    QPen p( QBrush( QColor( 0, 255, 127 ) ), 2 );
    painter->save();
    painter->setPen( p );
    painter->drawRect( r );
    painter->restore();
  }
}


void QgsAttributeTableDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{


  const QgsAttributeTableFilterModel* fm = dynamic_cast<const QgsAttributeTableFilterModel*>( index.model() );
  if ( !fm )
  {
      return;
  }
  const QgsAttributeTableModel* m = dynamic_cast<const QgsAttributeTableModel*>( fm->sourceModel() );
  if ( !m )
  {
      return;
  }
  int col = index.column();
  QVariant::Type type = m->layer()->dataProvider()->fields()[col].type();
  QgsVectorLayer::EditType editType = m->layer()->editType(col);
  if (editType == QgsVectorLayer::LineEdit ||
      editType == QgsVectorLayer::UniqueValuesEditable ||
      editType == QgsVectorLayer::FileName ||
      editType == QgsVectorLayer::Immutable)
  {
    QString qs = index.model()->data(index, Qt::DisplayRole).toString();
    
    QLineEdit* le = dynamic_cast<QLineEdit*> ( editor );
    le->setText( qs );
  }
  else if (editType == QgsVectorLayer::ValueMap ||
           editType == QgsVectorLayer::Classification ||
           editType == QgsVectorLayer::UniqueValues ||
           editType == QgsVectorLayer::Enumeration)
  {
    QComboBox* cb = dynamic_cast<QComboBox*>( editor );
    QVariant qs = index.model()->data(index, Qt::EditRole);
    int cbIndex = cb->findData(qs);
    if (cbIndex > -1)
    {
      cb->setCurrentIndex(cbIndex);
    }
  }
  else if (editType == QgsVectorLayer::SliderRange)
  {
     int value = index.model()->data(index, Qt::EditRole).toInt();
     QSlider* s = dynamic_cast<QSlider*>( editor );
     s->setValue( value );
  }
  else if (editType == QgsVectorLayer::EditRange)
  {
    if ( type == QVariant::Int )
    {
      QSpinBox* sb = dynamic_cast<QSpinBox*>( editor );
      int value = index.model()->data(index, Qt::EditRole).toInt();
      sb->setValue( value );
    }
    else if ( type == QVariant::Double )
    {
      QDoubleSpinBox* sb = dynamic_cast<QDoubleSpinBox*>( editor );
      double value = index.model()->data(index, Qt::EditRole).toDouble();
      sb->setValue( value );
    }
  }
}



void QgsAttributeTableDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  const QgsAttributeTableFilterModel* fm = dynamic_cast<const QgsAttributeTableFilterModel*>( index.model() );
  if ( !fm )
  {
      return;
  }
  const QgsAttributeTableModel* m = dynamic_cast<const QgsAttributeTableModel*>( fm->sourceModel() );
  if ( !m )
  {
      return;
  }

  int col = index.column();
  QVariant::Type type = m->layer()->dataProvider()->fields()[col].type();
  QgsVectorLayer::EditType editType = m->layer()->editType(col);
  if (editType == QgsVectorLayer::LineEdit ||
      editType == QgsVectorLayer::UniqueValuesEditable ||
      editType == QgsVectorLayer::FileName ||
      editType == QgsVectorLayer::Immutable)
  {
    QLineEdit* le = dynamic_cast<QLineEdit*> ( editor );
    QString text = le->text();
    QVariant value = QVariant (text);
    model->setData( index, value );
  }
  else if (editType == QgsVectorLayer::ValueMap ||
           editType == QgsVectorLayer::Classification ||
           editType == QgsVectorLayer::UniqueValues ||
           editType == QgsVectorLayer::Enumeration)
  {
    QComboBox* cb = dynamic_cast<QComboBox*>( editor );
    model->setData(index, cb->itemData(cb->currentIndex()));
  }
  else if (editType == QgsVectorLayer::SliderRange)
  {
    QSlider* s = dynamic_cast<QSlider*>( editor );
    model->setData( index, s->value() );
  }
  else if (editType == QgsVectorLayer::EditRange)
  {
    if ( type == QVariant::Int )
    {
      QSpinBox* sb = dynamic_cast<QSpinBox*>( editor );
      model->setData( index, sb->value() );
    }
    else if ( type == QVariant::Double )
    {
      QDoubleSpinBox* sb = dynamic_cast<QDoubleSpinBox*>( editor );
      model->setData( index, sb->value() );
    }
  }
}







