/***************************************************************************
    qgsfontoptions.h
    -------------------------
    begin                : June 2022
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
#ifndef QGSFONTOPTIONS_H
#define QGSFONTOPTIONS_H

#include "ui_qgsfontoptionswidgetbase.h"
#include "qgsoptionswidgetfactory.h"
#include "qgscodeeditor.h"

/**
 * \ingroup app
 * \class QgsFontOptionsWidget
 * \brief An options widget showing font options.
 *
 * \since QGIS 3.28
 */
class QgsFontOptionsWidget : public QgsOptionsPageWidget, private Ui::QgsFontOptionsWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFontOptionsWidget with the specified \a parent widget.
     */
    QgsFontOptionsWidget( QWidget *parent );

    void apply() override;

};


class QgsFontOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:

    QgsFontOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;
    QString pagePositionHint() const override;
};


#endif // QGSFONTOPTIONS_H
