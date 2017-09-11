/***************************************************************************
    qgsnewhttpconnection.cpp -  selector for a new HTTP server for WMS, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNEWHTTPCONNECTION_H
#define QGSNEWHTTPCONNECTION_H

#include "qgis_sip.h"
#include "ui_qgsnewhttpconnectionbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"

class QgsAuthConfigSelect;

/** \ingroup gui
 * \brief Dialog to allow the user to configure and save connection
 * information for an HTTP Server for WMS, etc.
 */
class GUI_EXPORT QgsNewHttpConnection : public QDialog, private Ui::QgsNewHttpConnectionBase
{
    Q_OBJECT

  public:

    /**
     * Available connection types for configuring in the dialog.
     * \since QGIS 3.0
     */
    enum ConnectionType
    {
      ConnectionWfs = 1 << 1, //!< WFS connection
      ConnectionWms = 1 << 2, //!< WMS connection
      ConnectionWcs = 1 << 3, //!< WCS connection
      ConnectionOther = 1 << 4, //!< Other connection type
    };
    Q_DECLARE_FLAGS( ConnectionTypes, ConnectionType )

    /**
     * Constructor for QgsNewHttpConnection.
     *
     * The \a types argument dictates which connection type settings should be
     * shown in the dialog.
     *
     */
    QgsNewHttpConnection( QWidget *parent SIP_TRANSFERTHIS = nullptr,
                          QgsNewHttpConnection::ConnectionTypes types = ConnectionWms,
                          const QString &baseKey = "qgis/connections-wms/",
                          const QString &connectionName = QString(),
                          Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

  public slots:
    // Saves the connection to ~/.qt/qgisrc
    void accept() override;
    void on_txtName_textChanged( const QString & );
    void on_txtUrl_textChanged( const QString & );

  private:

    ConnectionTypes mTypes = ConnectionWms;

    QString mBaseKey;
    QString mCredentialsBaseKey;
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    QgsAuthConfigSelect *mAuthConfigSelect = nullptr;
    void showHelp();

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsNewHttpConnection::ConnectionTypes )

#endif //  QGSNEWHTTPCONNECTION_H
