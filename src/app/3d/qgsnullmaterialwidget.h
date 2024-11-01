/***************************************************************************
  qgsnullmaterialwidget.h
  --------------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNULLMATERIALWIDGET_H
#define QGSNULLMATERIALWIDGET_H

#include "qgsmaterialsettingswidget.h"

#include <ui_nullmaterialwidget.h>

class QgsNullMaterialSettings;


//! Widget for configuration of null material settings
class QgsNullMaterialWidget : public QgsMaterialSettingsWidget, private Ui::NullMaterialWidget
{
    Q_OBJECT
  public:
    explicit QgsNullMaterialWidget( QWidget *parent = nullptr );

    static QgsMaterialSettingsWidget *create();
    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) override;
    QgsAbstractMaterialSettings *settings() override;
};

#endif // QGSNULLMATERIALWIDGET_H
