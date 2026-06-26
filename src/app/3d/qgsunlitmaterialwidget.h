/***************************************************************************
  qgsunlitmaterialwidget.h
  --------------------------------------
  Date                 : June 2026
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

#ifndef QGSUNLIITMATERIALWIDGET_H
#define QGSUNLIITMATERIALWIDGET_H

#include "ui_unlitmaterialwidgetbase.h"

#include "qgsmaterialsettingswidget.h"

//! Widget for configuration of unlit material settings
class QgsUnlitMaterialWidget : public QgsMaterialSettingsWidget, private Ui::UnlitMaterialWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsUnlitMaterialWidget( QWidget *parent = nullptr );

    static QgsMaterialSettingsWidget *create();

    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) final;
    std::unique_ptr< QgsAbstractMaterialSettings > settings() final;
  public slots:
    void setPreviewVisible( bool visible ) final;

  private slots:
    void updatePreview();
};

#endif // QGSUNLIITMATERIALWIDGET_H
