/***************************************************************************
                             qgsmodeldesignerdialog.h
                             ------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMODELDESIGNERDIALOG_H
#define QGSMODELDESIGNERDIALOG_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsmodeldesignerdialogbase.h"

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief Model designer dialog base class
 * \warning Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelDesignerDialog : public QMainWindow, public Ui::QgsModelDesignerDialogBase
{
  public:

    QgsModelDesignerDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr );

  protected:

    QToolBar *toolbar() { return mToolbar; }

};

///@endcond

#endif // QGSMODELDESIGNERDIALOG_H
