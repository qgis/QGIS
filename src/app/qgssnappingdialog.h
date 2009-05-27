/***************************************************************************
                              qgssnappingdialog.h
                              --------------------------
  begin                : June 11, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSNAPPINGDIALOG_H
#define QGSSNAPPINGDIALOG_H

#include "ui_qgssnappingdialogbase.h"

class QgsMapCanvas;

struct LayerEntry
{
  bool checked;
  int snapTo; //0 = to vertex, 1 = to segment, 2 = to vertex and to segment
  QString layerName;
  double tolerance;
  int toleranceUnit; // 0 = map units, 1 = pixels
};

/**A dialog to enter advanced editing properties, e.g. topological editing, snapping settings
for the individual layers*/
class QgsSnappingDialog: public QDialog, private Ui::QgsSnappingDialogBase
{
    Q_OBJECT

  public:
    /**Constructor
     @param canvas pointer to the map canvas (for detecting which vector layers are loaded
    @param settings existing snapping layer settings*/
    QgsSnappingDialog( QgsMapCanvas* canvas, const QMap<QString, LayerEntry >& settings );
    ~QgsSnappingDialog();

    /**Returns the snapping settings per layer. Key of the map is the layer id and value the
     corresponding layer entry*/
    void layerSettings( QMap<QString, LayerEntry>& settings ) const;

  private:
    /**Default constructor forbidden*/
    QgsSnappingDialog();
    /**Used to query the loaded layers*/
    QgsMapCanvas* mMapCanvas;
    /**Stores the layer ids from top to bottom*/
    QStringList mLayerIds;
};

#endif
