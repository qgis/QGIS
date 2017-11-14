/***************************************************************************
                         qgscomposerpolylinewidget.h
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

#ifndef QGSCOMPOSERPOLYLINEWIDGET_H
#define QGSCOMPOSERPOLYLINEWIDGET_H

#include "ui_qgscomposerpolylinewidgetbase.h"
#include "qgscomposeritemwidget.h"

class QgsComposerPolyline;

/**
 * Input widget for QgsComposerPolyline
 * \since QGIS 2.16
 */
class QgsComposerPolylineWidget: public QgsComposerItemBaseWidget, private Ui::QgsComposerPolylineWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsComposerPolylineWidget( QgsComposerPolyline *composerPolyline );

  private:
    QgsComposerPolyline *mComposerPolyline = nullptr;

    void updatePolylineStyle();

  private slots:
    void mLineStyleButton_clicked();

    //! Sets the GUI elements to the currentValues of mComposerShape
    void setGuiElementValues();

    void updateStyleFromWidget();
    void cleanUpStyleSelector( QgsPanelWidget *container );
};

#endif // QGSCOMPOSERPOLYLINEWIDGET_H
