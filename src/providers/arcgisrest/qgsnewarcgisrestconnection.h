/***************************************************************************
    qgsnewarcgisrestconnection.cpp
                             -------------------
    begin                : December 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNEWARCGISRESTCONNECTION_H
#define QGSNEWARCGISRESTCONNECTION_H

#include "qgis_sip.h"
#include "ui_qgsnewarcgisrestconnectionbase.h"
#include "qgsguiutils.h"

class QgsAuthSettingsWidget;

/**
 * \ingroup gui
 * \brief Dialog to allow the user to configure and save connection
 * information for an HTTP Server for WMS, etc.
 */
class QgsNewArcGisRestConnectionDialog : public QDialog, private Ui::QgsNewArcGisRestConnectionBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsNewArcGisRestConnectionDialog.
     */
    QgsNewArcGisRestConnectionDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, const QString &connectionName = QString(), Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    /**
     * Returns the current connection name.
     */
    QString name() const;

    /**
     * Returns the current connection url.
     */
    QString url() const;

  public slots:

    void accept() override;

  private slots:

    void nameChanged( const QString & );
    void urlChanged( const QString & );
    void updateOkButtonState();

  protected:
    /**
     * Returns TRUE if dialog settings are valid, or FALSE if current
     * settings are not valid and the dialog should not be acceptable.
     */
    virtual bool validate();

    /**
     * Returns the url.
     */
    QUrl urlTrimmed() const SIP_SKIP;

  private:
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    void showHelp();
};

#endif //  QGSNEWARCGISRESTCONNECTION_H
