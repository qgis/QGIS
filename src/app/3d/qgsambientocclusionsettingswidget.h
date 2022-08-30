/***************************************************************************
  qgsambientocclusionsettingswidget.h
  --------------------------------------
  Date                 : Juin 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAMBIENTOCCLUSIONSETTINGSWIDGET_H
#define QGSAMBIENTOCCLUSIONSETTINGSWIDGET_H

#include "ui_ambientocclusionsettingswidget.h"

#include "qgsambientocclusionsettings.h"

class QgsAmbientOcclusionSettingsWidget : public QWidget, private Ui::QgsAmbientOcclusionSettingsWidget
{
    Q_OBJECT

  public:
    explicit QgsAmbientOcclusionSettingsWidget( QWidget *parent = nullptr );

    //! Sets the ambient occlusion settings in the current widget UI
    void setAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &settings );
    //! Returns the ambient occlusion settings from the widget UI
    QgsAmbientOcclusionSettings toAmbientOcclusionSettings();
};

#endif // QGSAMBIENTOCCLUSIONSETTINGSWIDGET_H
