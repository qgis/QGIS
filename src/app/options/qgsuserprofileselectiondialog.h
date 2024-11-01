/***************************************************************************
  qgsuserprofileselectiondialog.h
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

#ifndef QGSUSERPROFILESELECTIONDIALOG_H
#define QGSUSERPROFILESELECTIONDIALOG_H

#include <QDialog>
#include <qgis_app.h>

#include "ui_qgsuserprofileselectiondialog.h"


// Forward declarations
class QgsUserProfileManager;
class QEvent;

/**
 * \class QgsUserProfileSelectionDialog
 * \brief A dialog shown at startup to select the user profile
 * \since QGIS 3.32
 */
class APP_EXPORT QgsUserProfileSelectionDialog : public QDialog, private Ui::QgsUserProfileSelectionDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsUserProfileSelectionDialog.
     * \param manager QgsUserProfileManager manager that will be used to fill the list of profiles
     * \param parent parent widget
     */
    explicit QgsUserProfileSelectionDialog( QgsUserProfileManager *manager, QWidget *parent = nullptr );
    virtual ~QgsUserProfileSelectionDialog();


    /* Return the selected profile name */
    QString selectedProfileName() const;

  public slots:
    void accept() override;

  private slots:
    void onAddProfile();

  private:
    QgsUserProfileManager *mManager = nullptr;
};

#endif // QGSUSERPROFILESELECTIONDIALOG_H
