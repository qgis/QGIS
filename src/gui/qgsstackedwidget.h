/***************************************************************************
    qgsstackedwidget.h
    ------------------
    begin                : January 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACKEDWIDGET_H
#define QGSSTACKEDWIDGET_H

#define SIP_NO_FILE

#include <QStackedWidget>
#include "qgis_gui.h"

class QSize;

/**
 * \class QgsStackedWidget
 * \ingroup gui
 * \brief A QStackedWidget that can be shrunk to its current widget's size.
 *
 * A regular QStackedWidget can be shrunk down the size of its
 * largest page widget. A QgsStackedWidget only takes the current
 * page widget into account when resizing.
 *
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsStackedWidget : public QStackedWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsStackedWidget.
     */
    explicit QgsStackedWidget( QWidget *parent = nullptr );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

};

#endif // QGSSTACKEDWIDGET_H
