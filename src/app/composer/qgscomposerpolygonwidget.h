/***************************************************************************
                         qgscomposerpolygonwidget.h
    begin                : March 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERPOLYGONWIDGET_H
#define QGSCOMPOSERPOLYGONWIDGET_H

#include "ui_qgscomposerpolygonwidgetbase.h"
#include "qgscomposeritemwidget.h"

class QgsComposerPolygon;

/**
 * Input widget for QgsComposerPolygon
 * @note added in QGIS 2.16
 */
class QgsComposerPolygonWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerPolygonWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsComposerPolygonWidget( QgsComposerPolygon* composerPolygon );
    ~QgsComposerPolygonWidget();

  private:
    QgsComposerPolygon* mComposerPolygon;

    void updatePolygonStyle();

  private slots:
    void on_mPolygonStyleButton_clicked();

    /** Sets the GUI elements to the currentValues of mComposerShape*/
    void setGuiElementValues();
};

#endif // QGSCOMPOSERPOLYGONWIDGET_H
