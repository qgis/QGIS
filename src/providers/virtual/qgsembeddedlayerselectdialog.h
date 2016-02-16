/***************************************************************************
      Virtual layer embedded layer selection widget

begin                : Feb 2016
copyright            : (C) 2016 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUAL_EMBEDDED_LAYER_SELECT_DIALOG_H
#define QGSVIRTUAL_EMBEDDED_LAYER_SELECT_DIALOG_H

#include "ui_qgsembeddedlayerselect.h"

#include <QDialog>

class QgsVectorLayer;
class QMainWindow;
class QgsLayerTreeView;

class QgsEmbeddedLayerSelectDialog : public QDialog, private Ui::QgsEmbeddedLayerSelectDialog
{
    Q_OBJECT

  public:
    QgsEmbeddedLayerSelectDialog( QWidget * parent, QgsLayerTreeView* tv );

    /** Returns the list of layer ids selected */
    QStringList layers() const;
};

#endif
