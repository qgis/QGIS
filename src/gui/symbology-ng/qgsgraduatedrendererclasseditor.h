/***************************************************************************
                         qgsgraduatedrendererv2classeditor.h  -  description
                             -------------------
    begin                : September 2014
    copyright            : (C) 2014 by Chris Crook
    email                : ccrook@linz.govt.nz
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRADUATEDRENDERERCLASSEDITOR_H
#define QGSGRADUATEDRENDERERCLASSEDITOR_H

#include "qgsgraduatedsymbolrendererv2.h"
#include "qgsvectorlayer.h"

#include "ui_qgsgraduatedrendererclasseditorbase.h"
#include "qgisgui.h"


class GUI_EXPORT QgsGraduatedRendererClassEditor: public QDialog, private Ui::QgsGraduatedRendererClassEditorBase
{
    Q_OBJECT
  public:
    QgsGraduatedRendererClassEditor( QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsGraduatedRendererClassEditor();
    void accept();
    void setRendererAndLayer( QgsGraduatedSymbolRendererV2 *renderer, QgsVectorLayer *layer );

  private:
    QString classesToString();
    // Returns an error message if failed, else a blank string
    QString stringToClasses( QString breaks, QgsRangeList &ranges );
    void applyRangesToRenderer( QgsRangeList &ranges );

    QgsGraduatedSymbolRendererV2 *mRenderer;
    QgsVectorLayer *mLayer;
    double mMinValue;
    double mMaxValue;
};

#endif
