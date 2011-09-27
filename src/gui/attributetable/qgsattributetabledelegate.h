/***************************************************************************
     QgsAttributeTableDelegate.h
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

#ifndef QGSATTRIBUTETABLEDELEGATE_H
#define QGSATTRIBUTETABLEDELEGATE_H

#include <QItemDelegate>
#include "qgsfeature.h"

class QPainter;
class QgsVectorLayer;

/** \ingroup app
 * A delegate item class for QgsAttributeTable (see Qt documentation for
 * QItemDelegate).
 */

class QgsAttributeTableDelegate : public QItemDelegate
{
    Q_OBJECT;

    QgsVectorLayer *layer( const QAbstractItemModel *model ) const;
    int fieldIdx( const QModelIndex &index ) const;
    QgsFeatureId featureId( const QModelIndex &index ) const;

  public:
    /** Constructor
     * @param parent parent object
     */
    QgsAttributeTableDelegate( QObject* parent = NULL ) :
        QItemDelegate( parent ) {};
    /** Used to create an editor for when the user tries to
     * change the contents of a cell */
    QWidget * createEditor(
      QWidget *parent,
      const QStyleOptionViewItem &option,
      const QModelIndex &index ) const;

    /** Overloads the paint method form the QItemDelegate bas class */
    void paint(
      QPainter * painter,
      const QStyleOptionViewItem & option,
      const QModelIndex & index ) const;

    /**
     * Sets data from editor back to model. Overloads default method
     * @param editor editor which was created by create editor function in this class
     * @param model model where data should be updated
     * @param index index of field which is to be modified
     */
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const;

    /**
     * Sets data from model into the editor. Overloads default method
     * @param editor editor which was created by create editor function in this class
     * @param index index of field which is to be retrieved
     */
    void setEditorData( QWidget *editor, const QModelIndex &index ) const;
};

#endif //QGSATTRIBUTETABLEDELEGATE_H
