/***************************************************************************
                             qgslayoutnewitempropertiesdialog.h
                             ----------------------------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTNEWITEMPROPERTIESDIALOG_H
#define QGSLAYOUTNEWITEMPROPERTIESDIALOG_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgslayoutnewitemproperties.h"

#include "qgslayoutsize.h"
#include "qgslayoutpoint.h"
#include "qgslayoutitem.h"

/**
 * \ingroup gui
 * \brief A dialog for configuring properties like the size and position of layout items.
 *
 * This is usually used only when constructing new layout items, allowing users to precisely
 * enter their sizes and positions.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutItemPropertiesDialog : public QDialog, private Ui::QgsLayoutNewItemPropertiesDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutNewItemPropertiesDialog.
     */
    QgsLayoutItemPropertiesDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr );

    /**
     * Sets the item \a position to show in the dialog.
     * \see itemPosition()
     */
    void setItemPosition( QgsLayoutPoint position );

    /**
     * Returns the current item position defined by the dialog.
     * \see setItemPosition()
     */
    QgsLayoutPoint itemPosition() const;

    /**
     * Returns the page number for the new item.
     */
    int page() const;

    /**
     * Sets the item \a size to show in the dialog.
     * \see itemSize()
     */
    void setItemSize( QgsLayoutSize size );

    /**
     * Returns the item size defined by the dialog.
     * \see setItemSize()
     */
    QgsLayoutSize itemSize() const;

    /**
     * Returns the item reference point defined by the dialog.
     * \see setReferencePoint()
     */
    QgsLayoutItem::ReferencePoint referencePoint() const;

    /**
     * Sets the item reference \a point defined to show in the dialog.
     * \see referencePoint()
     */
    void setReferencePoint( QgsLayoutItem::ReferencePoint point );

    /**
     * Sets the \a layout associated with the dialog. This allows the dialog
     * to retrieve properties from the layout and perform tasks like automatic
     * conversion of units.
     */
    void setLayout( QgsLayout *layout );

  private:

    QgsLayout *mLayout = nullptr;

};

#endif // QGSLAYOUTNEWITEMPROPERTIESDIALOG_H
