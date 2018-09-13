/***************************************************************************
  qgsvectorlayerloadsavestyledialog.h
  --------------------------------------
  Date                 : September 2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERLOADSAVESTYLEDIALOG_H
#define QGSVECTORLAYERLOADSAVESTYLEDIALOG_H

#include <QDialog>
#include "ui_qgsvectorlayerloadsavestyledialog.h"

class QgsVectorLayer;


class QgsVectorLayerLoadSaveStyleDialog : public QDialog, private Ui::QgsVectorLayerLoadSaveStyleDialog
{
    Q_OBJECT

  public:
    enum Mode
    {
      Save,
      Load
    };

    enum StyleType
    {
      GenericFile,
      QML,
      SLD,
      DB,
    };

    explicit QgsVectorLayerLoadSaveStyleDialog( QgsVectorLayer *layer, Mode mode, QWidget *parent = nullptr );
    ~QgsVectorLayerLoadSaveStyleDialog() = default;

  public slots:
    void accept();

  private:
    QgsVectorLayer *mLayer;
    Mode mMode;
};

#endif // QGSVECTORLAYERLOADSAVESTYLE_H
