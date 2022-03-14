/***************************************************************************
    qgstableview.h
    ---------------
    begin                : March 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSTABLEVIEW_H
#define QGSTABLEVIEW_H

#include <QTableView>
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \class QgsTableView
 * \ingroup gui
 * \brief A QTableView subclass with QGIS specific tweaks and improvements.
 *
 * QgsTableView should be used instead of QTableView widgets.
 * In most cases the use is identical, however QgsTableView
 * adds extra functionality and UI tweaks for improved user
 * experience in QGIS.
 *
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsTableView : public QTableView
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsTableView.
     */
    explicit QgsTableView( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    void wheelEvent( QWheelEvent *event ) override;
};

#endif // QGSTABLEVIEW_H
