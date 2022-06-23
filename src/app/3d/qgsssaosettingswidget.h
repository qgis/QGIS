/***************************************************************************
  qgsssaosettingswidget.h
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
#ifndef QGSSSAOSETTINGSWIDGET_H
#define QGSSSAOSETTINGSWIDGET_H

#include "ui_ssaosettingswidget.h"

#include "qgsssaosettings.h"

class QgsSsaoSettingsWidget : public QWidget, private Ui::QgsSsaoSettingsWidget
{
    Q_OBJECT

  public:
    explicit QgsSsaoSettingsWidget( QWidget *parent = nullptr );

    //! Sets the shadow settings in the current widget UI
    void setSsaoSettings( const QgsSsaoSettings &settings );
    //! Returns the shadow settings from the widget UI
    QgsSsaoSettings toSsaoSettings();
};

#endif // QGSSSAOSETTINGSWIDGET_H
