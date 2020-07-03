/***************************************************************************
  qgsmap3dexportwidget.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAP3DEXPORTWIDGET_H
#define QGSMAP3DEXPORTWIDGET_H

#include <QWidget>

namespace Ui
{
  class Map3DExportWidget;
}

class Qgs3DMapScene;

class QgsMap3DExportWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit QgsMap3DExportWidget( Qgs3DMapScene *scene, QWidget *parent = nullptr );
    ~QgsMap3DExportWidget();

    void exportScene();
  private:
    Ui::Map3DExportWidget *ui;
  private:
    Qgs3DMapScene *mScene = nullptr;
};

#endif // QGSMAP3DEXPORTWIDGET_H
