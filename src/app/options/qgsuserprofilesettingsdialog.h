/***************************************************************************
  qgsuserprofilesettingsdialog.h
  --------------------------------------
  Date                 : February 2023
  Copyright            : (C) 2023 by Yoann Quenach de Quivillic
  Email                : yoann dot quenach at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSUSERPROFILESETTINGSDIALOG_H
#define QGSUSERPROFILESETTINGSDIALOG_H

#include <QDialog>
#include "ui_qgsuserprofilesettingsdialog.h"

/**
 * \class QgsUserProfileSettingsDialog
 * \brief A dialog displaying user profile policy settings (open last closed profile, open specific, let user choose)
 * \since QGIS 3.32
 */
class QgsUserProfileSettingsDialog : public QDialog, private Ui::QgsUserProfileSettingsDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsUserProfileSettingsDialog.
     * \param parent parent widget
     */
    explicit QgsUserProfileSettingsDialog( QWidget *parent = nullptr );
    virtual ~QgsUserProfileSettingsDialog() = default;

  public slots:

    /**
    * Apply changes
    */
    void apply();


};

#endif // QGSUSERPROFILESETTINGSDIALOG_H
