/***************************************************************************
    qgselevationoptions.h
    -------------------------
    begin                : September 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSELEVATIONOPTIONS_H
#define QGSELEVATIONOPTIONS_H

#include "ui_qgselevationoptionswidgetbase.h"
#include "qgsoptionswidgetfactory.h"

/**
 * \ingroup app
 * \class QgsElevationOptionsWidget
 * \brief An options widget showing elevation related options.
 *
 * \since QGIS 3.34
 */
class QgsElevationOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsElevationOptionsWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsElevationOptionsWidget with the specified \a parent widget.
     */
    QgsElevationOptionsWidget( QWidget *parent );
    QString helpKey() const override;
    void apply() override;
};


class QgsElevationOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:
    QgsElevationOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QString pagePositionHint() const override;
};


#endif // QGSELEVATIONOPTIONS_H
