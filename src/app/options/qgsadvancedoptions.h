/***************************************************************************
    qgsadvancedoptions.h
    -------------------------
    begin                : September 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSADVANCEDOPTIONS_H
#define QGSADVANCEDOPTIONS_H

#include "ui_qgsadvancedsettingswidget.h"
#include "qgsoptionswidgetfactory.h"
#include "qgssettings.h"

/**
 * \ingroup app
 * \class QgsAdvancedSettingsWidget
 * \brief An options widget showing an advanced settings editor.
 *
 * \since QGIS 3.16
 */
class QgsAdvancedSettingsWidget : public QgsOptionsPageWidget, private Ui::QgsAdvancedSettingsWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAdvancedSettingsWidget with the specified \a parent widget.
     */
    QgsAdvancedSettingsWidget( QWidget *parent );
    ~QgsAdvancedSettingsWidget() override;
    void apply() override;

    QgsSettingsTree *settingsTree();

  private:

    QgsSettings mSettings;

};


class QgsAdvancedSettingsOptionsFactory : public QgsOptionsWidgetFactory
{
    Q_OBJECT

  public:

    QgsAdvancedSettingsOptionsFactory();

    QIcon icon() const override;
    QgsOptionsPageWidget *createWidget( QWidget *parent = nullptr ) const override;

};


#endif // QGSADVANCEDOPTIONS_H
