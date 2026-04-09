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

#include "ui_goochmaterialwidget.h"

#include "qgsmaterialsettingswidget.h"

class QgsGoochMaterialSettings;


//! Widget for configuration of Gooch material settings
class QgsGoochMaterialWidget : public QgsMaterialSettingsWidget, private Ui::GoochMaterialWidget
{
    Q_OBJECT
  public:
    explicit QgsGoochMaterialWidget( QWidget *parent = nullptr );

    static QgsMaterialSettingsWidget *create();
    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) final;
    void setTechnique( Qgis::MaterialRenderingTechnique technique ) final;
    std::unique_ptr< QgsAbstractMaterialSettings > settings() final;

  public slots:
    void setPreviewVisible( bool visible ) final;

  private slots:
    void updatePreview();
};

#endif // QGSGOOCHMATERIALWIDGET_H
