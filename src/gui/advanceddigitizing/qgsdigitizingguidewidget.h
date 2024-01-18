/***************************************************************************
    qgsdigitizingguidewidget.h
    ----------------------
    begin                : January 2024
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIGITIZINGGUIDEWIDGET_H
#define QGSDIGITIZINGGUIDEWIDGET_H

#include <QWidget>

#include "ui_qgsdigitizingguidewidget.h"

#include "qgis_gui.h"

class QgsDigitizingGuideMapTool;
class QgsDigitizingGuideLayer;
class QgsMapCanvas;

/**
 * \ingroup core
 * @brief The QgsDigitizingGuideLayer class holds map guides information saved in the project file.
 *
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsDigitizingGuideWidget : public QWidget, private Ui::QgsDigitizingGuideWidget
{
  Q_OBJECT
public:
  explicit QgsDigitizingGuideWidget(QgsMapCanvas *canvas, QWidget *parent = nullptr);

signals:

private:
  void enableGuideMapTool();

  void guideSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

  void removeButtonClicked();

  QgsMapCanvas* mCanvas = nullptr;
  QgsDigitizingGuideLayer* mGuideLayer = nullptr;

  QMap<QToolButton *, QgsDigitizingGuideMapTool *> mGuidesMapTools;

};

#endif // QGSDIGITIZINGGUIDEWIDGET_H
