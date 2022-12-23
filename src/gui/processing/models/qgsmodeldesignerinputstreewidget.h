/***************************************************************************
                             qgsmodeldesignerinputstreewidget.h
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

#ifndef QGSMODELDESIGNERINPUTSTREEWIDGET_H
#define QGSMODELDESIGNERINPUTSTREEWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include <QTreeWidget>

class QMimeData;

#define SIP_NO_FILE

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief QTreeWidget subclass for use in the model designer as an input list.
 * \warning Not stable API
 * \since QGIS 3.14
 */
class QgsModelDesignerInputsTreeWidget : public QTreeWidget
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsModelDesignerInputsTreeWidget with the specified \a parent widget.
     */
    explicit QgsModelDesignerInputsTreeWidget( QWidget *parent = nullptr );

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMimeData *mimeData( const QList<QTreeWidgetItem *> items ) const override;
#else
    QMimeData *mimeData( const QList<QTreeWidgetItem *> &items ) const override;
#endif
};

///@endcond

#endif // QGSMODELDESIGNERINPUTSTREEWIDGET_H
