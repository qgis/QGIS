/***************************************************************************
  qgsmaterialwidget.h
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

#ifndef QGSMATERIALWIDGET_H
#define QGSMATERIALWIDGET_H

#include <QWidget>
#include <memory>
#include <ui_materialwidget.h>

class QgsAbstractMaterialSettings;
enum class QgsMaterialSettingsRenderingTechnique;
class QgsVectorLayer;

//! Widget for configuration of material settings
class QgsMaterialWidget : public QWidget, private Ui::MaterialWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsMaterialWidget( QWidget *parent = nullptr );

    /**
     * Sets the required rendering \a technique which the material must support.
     *
     * This is used to filter the available material choices in the widget.
     */
    void setTechnique( QgsMaterialSettingsRenderingTechnique technique );

    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer );
    QgsAbstractMaterialSettings *settings();

    void setType( const QString &type );

  signals:

    void changed();

  private slots:
    void materialTypeChanged();
    void materialWidgetChanged();

  private:
    void updateMaterialWidget();
    QgsVectorLayer *mLayer = nullptr;

    std::unique_ptr<QgsAbstractMaterialSettings> mCurrentSettings;
    QgsMaterialSettingsRenderingTechnique mTechnique;
};

#endif // QGSMATERIALWIDGET_H
