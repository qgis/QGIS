/***************************************************************************
    testrenderergui.h
    ---------------------
    begin                : January 2012
    copyright            : (C) 2012 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef TESTRENDERERV2GUI_H
#define TESTRENDERERV2GUI_H

#include <QMainWindow>

class QgsMapCanvas;

class TestRendererGUI : public QMainWindow
{
    Q_OBJECT
  public:
    explicit TestRendererGUI( QWidget *parent = nullptr );
    void loadLayers();

  signals:

  public slots:
    void setRenderer();

  protected:
    QgsMapCanvas *mMapCanvas = nullptr;
};

#endif // TESTRENDERERV2GUI_H
