/***************************************************************************
    qgsaipipinstallapprovaldialog.h
    ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAIPIPINSTALLAPPROVALDIALOG_H
#define QGSAIPIPINSTALLAPPROVALDIALOG_H

#include "qgis_app.h"

#include <QDialog>
#include <QStringList>

/**
 * Modal dialog used to confirm an `install_python_package` request from the AI assistant.
 * Shows the verbatim list of pip specs (one per line, monospace) so the user can spot
 * typosquatting (e.g. `requessts` vs `requests`), the optional reason from the model,
 * and a clear notice that QGIS' user-scope site-packages will be modified.
 *
 * The dialog never auto-confirms; the user must explicitly click "Install".
 */
class APP_EXPORT QgsAiPipInstallApprovalDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * \param packages  The pip specs the model wants installed (e.g. "geopy>=2.4").
     * \param reason    Free-form description from the model explaining the intent.
     * \param parent    Parent widget for modal positioning.
     */
    QgsAiPipInstallApprovalDialog( const QStringList &packages, const QString &reason, QWidget *parent = nullptr );
};

#endif // QGSAIPIPINSTALLAPPROVALDIALOG_H
