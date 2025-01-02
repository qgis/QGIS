/***************************************************************************
                             qgsmodeldesignerinputstreewidget.cpp
                             ------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodeldesignerinputstreewidget.h"
#include "moc_qgsmodeldesignerinputstreewidget.cpp"

#include <QMimeData>

///@cond NOT_STABLE

QgsModelDesignerInputsTreeWidget::QgsModelDesignerInputsTreeWidget( QWidget *parent )
  : QTreeWidget( parent )
{
}

#if QT_VERSION < QT_VERSION_CHECK( 6, 0, 0 )
QMimeData *QgsModelDesignerInputsTreeWidget::mimeData( const QList<QTreeWidgetItem *> items ) const
#else
QMimeData *QgsModelDesignerInputsTreeWidget::mimeData( const QList<QTreeWidgetItem *> &items ) const
#endif
{
  if ( items.empty() )
    return nullptr;

  std::unique_ptr<QMimeData> res = std::make_unique<QMimeData>();
  const QString text = items.value( 0 )->data( 0, Qt::UserRole ).toString();
  res->setText( text );
  return res.release();
}

///@endcond
