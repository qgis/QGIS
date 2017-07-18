/***************************************************************************
                             qgslayoutunitscombobox.h
                             ------------------------
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
#ifndef QGSLAYOUTUNITSCOMBOBOX_H
#define QGSLAYOUTUNITSCOMBOBOX_H

#include <QComboBox>
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsunittypes.h"

/**
 * \ingroup gui
 * A custom combo box for selecting units for layout settings.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutUnitsComboBox : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( QgsUnitTypes::LayoutUnit unit READ unit WRITE setUnit NOTIFY changed )

  public:

    /**
     * Constructor for QgsLayoutUnitsComboBox.
     */
    QgsLayoutUnitsComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the unit currently selected in the combo box.
     * \see setUnit()
     */
    QgsUnitTypes::LayoutUnit unit() const;

    /**
     * Sets the \a unit currently selected in the combo box.
     * \see unit()
     */
    void setUnit( QgsUnitTypes::LayoutUnit unit );

  signals:

    /**
     * Emitted when the \a unit is changed.
     */
    void changed( QgsUnitTypes::LayoutUnit unit );

};

#endif // QGSLAYOUTUNITSCOMBOBOX_H
