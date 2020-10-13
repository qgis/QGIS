/***************************************************************************
  qgsskyboxrenderingsettingswidget.h
  --------------------------------------
  Date                 : August 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb uderscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SKYBOXRENDERINGSETTINGSWIDGET_H
#define SKYBOXRENDERINGSETTINGSWIDGET_H

#include "ui_skyboxrenderingsettingswidget.h"

#include "qgsskyboxsettings.h"

class QgsSkyboxRenderingSettingsWidget : public QWidget, private Ui::SkyboxRenderingSettingsWidget
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsSkyboxRenderingSettingsWidget( QWidget *parent = nullptr );

    //! Sets the skybox settings in the current widget UI
    void setSkyboxSettings( const QgsSkyboxSettings &skyboxSettings );
    //! Returns the skybox settings from the widget UI
    QgsSkyboxSettings toSkyboxSettings();

  private slots:
    //! Shows settings of the enabled skybox type
    void showSkyboxSettings( int index );
};

#endif // SKYBOXRENDERINGSETTINGSWIDGET_H
