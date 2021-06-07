/***************************************************************************
                             qgsmodeldesignerinputstreewidget.cpp
                             ------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodeldesignerinputstreewidget.h"

#include <QMimeData>

///@cond NOT_STABLE

QgsModelDesignerInputsTreeWidget::QgsModelDesignerInputsTreeWidget( QWidget *parent )
  : QTreeWidget( parent )
{

}

QMimeData *QgsModelDesignerInputsTreeWidget::mimeData( const QList<QTreeWidgetItem *> items ) const
{
  if ( items.empty() )
    return nullptr;

  std::unique_ptr< QMimeData > res = qgis::make_unique< QMimeData >();
  const QString text = items.value( 0 )->data( 0, Qt::UserRole ).toString();
  res->setText( text );
  return res.release();
}

///@endcond

