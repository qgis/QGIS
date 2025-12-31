/***************************************************************************
                             qgsmetadataurlitemdelegate.cpp
                             ------------------
  begin                : June 21, 2021
  copyright            : (C) 2021 by Etienne Trimaille
  email                : etrimaille at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmetadataurlitemdelegate.h"

#include <QComboBox>
#include <QStringListModel>

#include "moc_qgsmetadataurlitemdelegate.cpp"

///@cond PRIVATE

MetadataUrlItemDelegate::MetadataUrlItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

QWidget *MetadataUrlItemDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  if ( index.column() == 1 )
  {
    // Type
    QComboBox *typeEditor = new QComboBox( parent );
    QStringList types;
    types << QString() << u"FGDC"_s << u"TC211"_s;
    QStringListModel *model = new QStringListModel( parent );
    model->setStringList( types );
    typeEditor->setModel( model );
    return typeEditor;
  }
  else if ( index.column() == 2 )
  {
    // Format
    QComboBox *typeFormat = new QComboBox( parent );
    QStringList formats;
    formats << QString() << u"text/plain"_s << u"text/xml"_s;
    QStringListModel *model = new QStringListModel( parent );
    model->setStringList( formats );
    typeFormat->setModel( model );
    return typeFormat;
  }

  return QStyledItemDelegate::createEditor( parent, option, index );
}
///@endcond
