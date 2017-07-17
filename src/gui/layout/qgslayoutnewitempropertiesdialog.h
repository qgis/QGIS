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

/**
 * \ingroup gui
 * \brief A dialog for configuring properties like the size and position of new layout items.
 */
class GUI_EXPORT QgsLayoutNewItemPropertiesDialog : public QDialog, private Ui::QgsLayoutNewItemPropertiesDialog
{
    Q_OBJECT

  public:

    QgsLayoutNewItemPropertiesDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = 0 );


    void setInitialItemPosition( QPointF position );

    QgsLayoutPoint itemPosition() const;

    QgsLayoutSize itemSize() const;

};

#endif // QGSLAYOUTNEWITEMPROPERTIESDIALOG_H
