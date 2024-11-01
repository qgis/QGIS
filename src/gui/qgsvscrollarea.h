/***************************************************************************
                              qgsvscrollarea.h
                              ------------------------
  begin                : September 2017
  copyright            : (C) 2017 Sandro Mani
  email                : manisandro at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVSCROLLAREA_H
#define QGSVSCROLLAREA_H

#include "qgis_gui.h"
#include "qgsscrollarea.h"

/**
 * \ingroup gui
 * \brief QgsVScrollArea is a QScrollArea subclass which only displays a vertical
 * scrollbar and fits the width to the contents.
 *
 */
class GUI_EXPORT QgsVScrollArea : public QgsScrollArea
{
    Q_OBJECT

  public:
    /**
     * QgsVScrollArea
     * \param parent The parent widget
     */
    QgsVScrollArea( QWidget *parent = nullptr );

    bool eventFilter( QObject *o, QEvent *e ) override;
};

#endif // QGSVSCROLLAREA_H
