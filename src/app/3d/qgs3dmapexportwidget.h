/***************************************************************************
  qgs3dmapexportwidget.h
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

#ifndef QGS3DMAPEXPORTWIDGET_H
#define QGS3DMAPEXPORTWIDGET_H

#include <QWidget>

namespace Ui
{
  class Map3DExportWidget;
}

class Qgs3DMapScene;
class Qgs3DMapExportSettings;

class Qgs3DMapExportWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit Qgs3DMapExportWidget( Qgs3DMapScene *scene, Qgs3DMapExportSettings *exportSettings, QWidget *parent = nullptr );
    ~Qgs3DMapExportWidget();

    void loadSettings();
    bool exportScene();
  private slots:
    void settingsChanged();
    void exportFormatChanged();

  private:
    Ui::Map3DExportWidget *ui;

  private:
    Qgs3DMapScene *mScene = nullptr;
    Qgs3DMapExportSettings *mExportSettings = nullptr;
};

#endif // QGS3DMAPEXPORTWIDGET_H
