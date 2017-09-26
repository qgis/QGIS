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
#include <QScrollArea>

class GUI_EXPORT QgsVScrollArea : public QScrollArea
{
  public:
    QgsVScrollArea( QWidget *parent = 0 );
    bool eventFilter( QObject *o, QEvent *e ) override;
};

#endif // QGSVSCROLLAREA_H
