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
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsFeatureSelectionModel;
class QPainter;
class QgsVectorLayer;
class QgsAttributeTableModel;
class QToolButton;

/**
 * \ingroup gui
 * \brief A delegate item class for QgsAttributeTable (see Qt documentation for
 * QItemDelegate).
 */

class GUI_EXPORT QgsAttributeTableDelegate : public QItemDelegate
{
    Q_OBJECT

    static QgsVectorLayer *layer( const QAbstractItemModel *model );
    static const QgsAttributeTableModel *masterModel( const QAbstractItemModel *model );

  public:
    /**
     * Constructor
     * \param parent parent object
     */
    QgsAttributeTableDelegate( QObject *parent SIP_TRANSFERTHIS = nullptr )
      : QItemDelegate( parent )
    {
    }

    /**
     * Used to create an editor for when the user tries to
     * change the contents of a cell
     */
    QWidget *createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    /**
     * Overloads the paint method form the QItemDelegate base class
     */
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const override;

    /**
     * Sets data from editor back to model. Overloads default method
     * \param editor editor which was created by create editor function in this class
     * \param model model where data should be updated
     * \param index index of field which is to be modified
     */
    void setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const override;

    /**
     * Sets data from model into the editor. Overloads default method
     * \param editor editor which was created by create editor function in this class
     * \param index index of field which is to be retrieved
     */
    void setEditorData( QWidget *editor, const QModelIndex &index ) const override;

    void setFeatureSelectionModel( QgsFeatureSelectionModel *featureSelectionModel );

  signals:

    /**
     * Emitted when an action column item is painted.
     * The consumer of this signal can initialize the index widget.
     *
     * \note This signal is emitted repeatedly whenever the item is being painted.
     *       It is the consumers responsibility to check if initialization has already
     *       happened before.
     */
    void actionColumnItemPainted( const QModelIndex &index ) const;

  private:
    QgsFeatureSelectionModel *mFeatureSelectionModel = nullptr;
};

#endif //QGSATTRIBUTETABLEDELEGATE_H
