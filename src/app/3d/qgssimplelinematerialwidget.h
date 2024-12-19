/***************************************************************************
  qgssimplelinematerialwidget.h
  --------------------------------------
  Date                 : July 2020
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

#ifndef QGSSIMPLELINEMATERIALWIDGET_H
#define QGSSIMPLELINEMATERIALWIDGET_H

#include "qgsmaterialsettingswidget.h"
#include <ui_simplelinematerialwidgetbase.h>

//! Widget for configuration of simple line material settings
class QgsSimpleLineMaterialWidget : public QgsMaterialSettingsWidget, private Ui::SimpleLineMaterialWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsSimpleLineMaterialWidget( QWidget *parent = nullptr );

    static QgsMaterialSettingsWidget *create();

    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) override;
    QgsAbstractMaterialSettings *settings() override;
};

#endif // QGSSIMPLELINEMATERIALWIDGET_H
