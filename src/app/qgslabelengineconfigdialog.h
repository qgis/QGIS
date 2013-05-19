/***************************************************************************
    qgslabelengineconfigdialog.h
    ---------------------
    begin                : May 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLABELENGINECONFIGDIALOG_H
#define QGSLABELENGINECONFIGDIALOG_H

#include <QDialog>

#include "ui_qgsengineconfigdialog.h"

class QgsPalLabeling;

class QgsLabelEngineConfigDialog : public QDialog, private Ui::QgsEngineConfigDialog
{
    Q_OBJECT
  public:
    QgsLabelEngineConfigDialog( QgsPalLabeling* lbl, QWidget* parent = NULL );

  public slots:
    void onOK();
    /** @note Added in QGIS 1.9 */
    void setDefaults();

  protected:
    QgsPalLabeling* mLBL;
};

#endif // QGSLABELENGINECONFIGDIALOG_H
