/***************************************************************************
  qgsclothmaterialwidget.h
  --------------------------------------
  Date                 : May 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCLOTHMATERIALWIDGET_H
#define QGSCLOTHMATERIALWIDGET_H

#include "ui_clothmaterialwidget.h"

#include "qgsmaterialsettingswidget.h"

class QgsAbstractMaterialSettings;


//! Widget for configuration of cloth material settings
class QgsClothMaterialWidget : public QgsMaterialSettingsWidget, private Ui::ClothMaterialWidget
{
    Q_OBJECT

  public:
    explicit QgsClothMaterialWidget( QWidget *parent = nullptr, bool hasOpacity = true );

    static QgsMaterialSettingsWidget *create();

    void setTechnique( Qgis::MaterialRenderingTechnique technique ) final;
    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) final;
    std::unique_ptr< QgsAbstractMaterialSettings > settings() final;
  public slots:
    void setPreviewVisible( bool visible ) final;
  private slots:

    void updateWidgetState();
    void updatePreview();
};

#endif // QGSCLOTHMATERIALWIDGET_H
