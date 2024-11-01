/***************************************************************************
                             qgshistorywidgetcontext.h
                             ------------------
    Date                 : April 2023
    Copyright            : (C) 2023 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHISTORYWIDGETCONTEXT_H
#define QGSHISTORYWIDGETCONTEXT_H

#include "qgis.h"
#include "qgis_gui.h"

class QgsMessageBar;

/**
 * \ingroup gui
 * \class QgsHistoryWidgetContext
 * \brief Contains settings which reflect the context in which a history widget is shown, e.g., an associated message bar.
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsHistoryWidgetContext
{
  public:
    QgsHistoryWidgetContext() = default;

    /**
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the appropriate message bar.
     * \see messageBar()
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Returns the message bar associated with the widget.
     * \see setMessageBar()
     */
    QgsMessageBar *messageBar() const;

  private:
    QgsMessageBar *mMessageBar = nullptr;
};

#endif // QGSHISTORYWIDGETCONTEXT_H
