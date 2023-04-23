/***************************************************************************
  qgssettingstreewidget.h
  --------------------------------------
  Date                 : April 2023
  Copyright            : (C) 2023 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSETTINGSTREEWIDGET_H
#define QGSSETTINGSTREEWIDGET_H

#include "qgis_gui.h"

#include "qwidget.h"

class QTreeView;

class QgsSettingsTreeModel;

/**
 * \ingroup gui
 * \class QgsSettingsTreeWidget
 * \brief QgsSettingsTreeWidget is a widget with the settings tree to visualize, search and edit settings
 *
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsSettingsTreeWidget : public QWidget
{

  public:
    //! Constructor
    explicit QgsSettingsTreeWidget( QWidget *parent = nullptr );


    // Apply changes to settings value
    void applyChanges() const;

  private:
    QgsSettingsTreeModel *mTreeModel = nullptr;
    QTreeView *mTreeView = nullptr;

};

#endif // QGSSETTINGSTREEWIDGET_H
