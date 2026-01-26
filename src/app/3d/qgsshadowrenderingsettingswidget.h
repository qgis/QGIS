/***************************************************************************
  qgsshadowrenderingsettingswidget.h
  --------------------------------------
  Date                 : September 2020
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

#ifndef SHADOWRENDERINGSETTINGSWIDGET_H
#define SHADOWRENDERINGSETTINGSWIDGET_H

#include "ui_shadowrenderingsettingswidget.h"

#include "qgsshadowsettings.h"

class QgsShadowRenderingSettingsWidget : public QWidget, private Ui::ShadowRenderingSettingsWidget
{
    Q_OBJECT

  public:
    //! Constructor
    explicit QgsShadowRenderingSettingsWidget( QWidget *parent = nullptr );

    //! Sets the shadow settings in the current widget UI
    void setShadowSettings( const QgsShadowSettings &skyboxSettings );
    //! Returns the shadow settings from the widget UI
    QgsShadowSettings toShadowSettings();
  public slots:
    void onDirectionalLightsCountChanged( int newCount );
};

#endif // SHADOWRENDERINGSETTINGSWIDGET_H
