/***************************************************************************
                          qgsprojecttrustdialog.h
                             -------------------
    begin                : October 2025
    copyright            : (C) 2025 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTTRUSTDIALOG_H
#define QGSPROJECTTRUSTDIALOG_H

#include "ui_qgsprojecttrustdialog.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"

class QgsProject;


/**
 * \ingroup gui
 * \brief A dialog to handle granting of trust to projects containing python code.
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsProjectTrustDialog : public QDialog, private Ui::QgsProjectTrustDialog
{
    Q_OBJECT
  public:
    QgsProjectTrustDialog( QgsProject *project, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    bool applyTrustToProjectFolder() const;

  private slots:
    void buttonBoxClicked( QAbstractButton *button );
    void showHelp();

  private:
};

#endif
