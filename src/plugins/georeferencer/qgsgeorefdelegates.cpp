/***************************************************************************
     qgsgeorefdelegates.cpp
     --------------------------------------
    Date                 : 14-Feb-2010
    Copyright            : (C) 2010 by Jack R, Maxim Dubinin (GIS-Lab)
    Email                : sim@gis-lab.info
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#include <limits>

#include <QDoubleSpinBox>
#include <QLineEdit>

#include "qgsgeorefvalidators.h"
#include "qgsgeorefdelegates.h"

// ------------------------ QgsNonEditableDelegate ------------------------- //
QgsNonEditableDelegate::QgsNonEditableDelegate(QWidget *parent)
  : QStyledItemDelegate(parent)
{
}

// ------------------------- QgsDmsAndDdDelegate --------------------------- //
QgsDmsAndDdDelegate::QgsDmsAndDdDelegate(QWidget *parent)
  : QStyledItemDelegate(parent)
{
}

QWidget *QgsDmsAndDdDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                                const QModelIndex &/*index*/) const
{
  QLineEdit *editor = new QLineEdit(parent);
  QgsDMSAndDDValidator *validator = new QgsDMSAndDDValidator(editor);
  editor->setValidator(validator);

  return editor;
}

void QgsDmsAndDdDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  QString value = index.model()->data(index, Qt::EditRole).toString();

  QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
  lineEdit->setText(value);
}

void QgsDmsAndDdDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                            const QModelIndex &index) const
{
  QLineEdit *lineEdit = static_cast<QLineEdit *>(editor);
  QString value = lineEdit->text();
  if (value.contains(' '))
    value = dmsToDD(value);

  model->setData(index, value, Qt::EditRole);
}

void QgsDmsAndDdDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                                    const QModelIndex &/*index*/) const
{
  editor->setGeometry(option.rect);
}

QString QgsDmsAndDdDelegate::dmsToDD(QString dms) const
{
  QStringList list = dms.split(' ');
  QString tmpStr = list.at(0);
  double res = qAbs(tmpStr.toDouble());

  tmpStr = list.value(1);
  if (!tmpStr.isEmpty())
    res += tmpStr.toDouble() / 60;

  tmpStr = list.value(2);
  if (!tmpStr.isEmpty())
    res += tmpStr.toDouble() / 3600;

  if (dms.startsWith('-'))
    return QString::number(-res, 'f', 7);
  else
    return QString::number(res, 'f', 7);
}

// ---------------------------- QgsCoordDelegate --------------------------- //
QgsCoordDelegate::QgsCoordDelegate(QWidget *parent)
  : QStyledItemDelegate(parent)
{
}

QWidget *QgsCoordDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/*option*/,
                                                const QModelIndex &/*index*/) const
{
  QLineEdit *editor = new QLineEdit(parent);
  QRegExp re("-?\\d*(\\.\\d+)?");
  QRegExpValidator *validator = new QRegExpValidator(re, editor);
  editor->setValidator(validator);

  return editor;
}
