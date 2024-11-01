/***************************************************************************
  qgsgoochmaterialwidget.h
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

#ifndef QGSGOOCHMATERIALWIDGET_H
#define QGSGOOCHMATERIALWIDGET_H

#include "qgsmaterialsettingswidget.h"

#include <ui_goochmaterialwidget.h>

class QgsGoochMaterialSettings;


//! Widget for configuration of Gooch material settings
class QgsGoochMaterialWidget : public QgsMaterialSettingsWidget, private Ui::GoochMaterialWidget
{
    Q_OBJECT
  public:
    explicit QgsGoochMaterialWidget( QWidget *parent = nullptr );

    static QgsMaterialSettingsWidget *create();
    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) override;
    void setTechnique( QgsMaterialSettingsRenderingTechnique technique ) override;
    QgsAbstractMaterialSettings *settings() override;
};

#endif // QGSGOOCHMATERIALWIDGET_H
