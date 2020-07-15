/***************************************************************************
  qgsphongmaterialwidget.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPHONGMATERIALWIDGET_H
#define QGSPHONGMATERIALWIDGET_H

#include "qgsmaterialsettingswidget.h"

#include <ui_phongmaterialwidget.h>

class QgsPhongMaterialSettings;


//! Widget for configuration of Phong material settings
class QgsPhongMaterialWidget : public QgsMaterialSettingsWidget, private Ui::PhongMaterialWidget
{
    Q_OBJECT
  public:
    explicit QgsPhongMaterialWidget( QWidget *parent = nullptr );

    static QgsMaterialSettingsWidget *create();

    void setDiffuseVisible( bool visible );
    bool isDiffuseVisible() const;

    void setSettings( const QgsAbstractMaterialSettings *settings, QgsVectorLayer *layer ) override;
    QgsAbstractMaterialSettings *settings() override;

    //! activates the texturing UI (to make sure texturing UI isn't visible when the user doesn't need it like in the 3D configuration window)
    void activateTexturingUI( bool activated );

};

#endif // QGSPHONGMATERIALWIDGET_H
