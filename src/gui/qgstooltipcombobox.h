/***************************************************************************
                              qgstooltipcombobox.h
                              ------------------------
  begin                : May 25, 2023
  copyright            : (C) 2017 by Mathieu Pellerin
  email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTOOLTIPCOMBOBOX_H
#define QGSTOOLTIPCOMBOBOX_H

#include <QComboBox>

#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgis.h"

class QEvent;

/**
 * \class QgsToolTipComboBox
 * \ingroup gui
 * \brief QComboBox subclass which features a tooltip when mouse hovering the combobox.
 * The tooltip string is taken from the current item's Qt.ToolTipRole data.
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsToolTipComboBox : public QComboBox
{
    Q_OBJECT

  public:
    //! Constructor for QgsToolTipComboBox.
    QgsToolTipComboBox( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    bool event( QEvent *event ) override;
};

#endif // QGSTOOLTIPCOMBOBOX_H
