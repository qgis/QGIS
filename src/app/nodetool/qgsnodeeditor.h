/***************************************************************************
                               qgsnodeeditor.h
                               ---------------
        begin                : Tue Mar 24 2015
        copyright            : (C) 2015 Sandro Mani / Sourcepole AG
        email                : smani@sourcepole.ch

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNODEEDITOR_H
#define QGSNODEEDITOR_H

#include <QDockWidget>

class QgsMapCanvas;
class QgsRubberBand;
class QgsSelectedFeature;
class QgsVectorLayer;
class QTableWidget;

/** A widget to select and edit the vertex coordinates of a geometry numerically*/
class QgsNodeEditor : public QDockWidget
{
    Q_OBJECT
  public:
    QgsNodeEditor( QgsVectorLayer* layer,
                   QgsSelectedFeature* selectedFeature,
                   QgsMapCanvas* canvas );

  public:
    QgsVectorLayer* mLayer;
    QgsSelectedFeature* mSelectedFeature;
    QgsMapCanvas* mCanvas;
    QTableWidget* mTableWidget;

  private slots:
    void rebuildTable();
    void tableValueChanged( int row, int col );
    void updateTableSelection();
    void updateNodeSelection();
};

#endif // QGSNODEEDITOR_H
