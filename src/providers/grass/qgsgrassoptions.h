/***************************************************************************
                              qgsgrassoptions.h
                             -------------------
    begin                : May, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGRASSOPTIONS_H
#define QGSGRASSOPTIONS_H

#include <QDialog>

#include "qgsoptionsdialogbase.h"

#include "ui_qgsgrassoptionsbase.h"

class GRASS_LIB_EXPORT QgsGrassOptions : public QgsOptionsDialogBase, private Ui::QgsGrassOptionsBase
{
    Q_OBJECT

  public:
    explicit QgsGrassOptions( QWidget *parent = 0 );
    ~QgsGrassOptions();

  private slots:
    void on_mModulesConfigBrowseButton_clicked();
    void saveOptions();

  private:
    QString mImportSettingsPath;
    QString mModulesSettingsPath;
};

#endif // QGSGRASSOPTIONS_H
