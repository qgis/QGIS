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
#include "qgssettingstree.h"

class QgsAuthSettingsWidget;
class QgsSettingsEntryBool;

/**
 * \ingroup gui
 * \brief Dialog to allow the user to configure and save connection
 * information for an HTTP Server for WMS, etc.
 */
class GUI_EXPORT QgsNewHttpConnection : public QDialog, private Ui::QgsNewHttpConnectionBase
{
    Q_OBJECT

  public:
#ifndef SIP_RUN
    static inline QgsSettingsTreeNode *sTreeHttpConnectionDialog = QgsSettingsTree::sTreeConnections->createChildNode( QStringLiteral( "http-connection-dialog" ) );

    static const QgsSettingsEntryBool *settingsIgnoreReportedLayerExtentsDefault;
#endif

    /**
     * Available connection types for configuring in the dialog.
     */
    enum ConnectionType SIP_ENUM_BASETYPE( IntFlag )
    {
      ConnectionWfs = 1 << 1,   //!< WFS connection
      ConnectionWms = 1 << 2,   //!< WMS connection
      ConnectionWcs = 1 << 3,   //!< WCS connection
      ConnectionOther = 1 << 4, //!< Other connection type
    };
    Q_DECLARE_FLAGS( ConnectionTypes, ConnectionType )

    /**
     * Flags controlling dialog behavior.
     */
    enum Flag SIP_ENUM_BASETYPE( IntFlag )
    {
      FlagShowTestConnection = 1 << 1,      //!< Display the 'test connection' button
      FlagHideAuthenticationGroup = 1 << 2, //!< Hide the Authentication group
      FlagShowHttpSettings = 1 << 3,        //!< Display the 'http' group
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * Constructor for QgsNewHttpConnection.
     *
     * The \a types argument dictates which connection type settings should be
     * shown in the dialog.
     *
     * The \a flags argument allows specifying flags which control the dialog behavior
     * and appearance.
     */
    QgsNewHttpConnection( QWidget *parent SIP_TRANSFERTHIS = nullptr, QgsNewHttpConnection::ConnectionTypes types = ConnectionWms,
                          const QString &serviceName SIP_PYARGRENAME( settingsKey ) = "WMS", // TODO QGIS 4 remove arg rename
                          const QString &connectionName = QString(), QgsNewHttpConnection::Flags flags = QgsNewHttpConnection::Flags(), Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

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
    void wfsVersionCurrentIndexChanged( int index );
    void wfsFeaturePagingCurrentIndexChanged( int index );

  protected:
    //! Index of wfsVersionComboBox
    enum WfsVersionIndex
    {
      WFS_VERSION_MAX = 0,
      WFS_VERSION_1_0 = 1,
      WFS_VERSION_1_1 = 2,
      WFS_VERSION_2_0 = 3,
      WFS_VERSION_API_FEATURES_1_0 = 4,
    };

#ifndef SIP_RUN
    //! Index of wfsFeaturePaging
    enum class WfsFeaturePagingIndex
    {
      DEFAULT = 0,
      ENABLED = 1,
      DISABLED = 2,
    };
#endif

    /**
     * Returns TRUE if dialog settings are valid, or FALSE if current
     * settings are not valid and the dialog should not be acceptable.
     */
    virtual bool validate();

    /**
     * Returns the "test connection" button.
     */
    QPushButton *testConnectButton();

    /**
     * Returns the current authentication settings widget.
     * \since QGIS 3.8
     */
    QgsAuthSettingsWidget *authSettingsWidget() SIP_SKIP;

    /**
     * Returns the "WFS version detect" button.
     * \since QGIS 3.2
     */
    QPushButton *wfsVersionDetectButton() SIP_SKIP;

    /**
     * Returns the "WFS version" combobox.
     * \since QGIS 3.2
     */
    QComboBox *wfsVersionComboBox() SIP_SKIP;

    /**
     * Returns the "WFS paging" combobox
     * \since QGIS 3.36
     */
    QComboBox *wfsPagingComboBox() SIP_SKIP;

    /**
     * Returns the "Use GML2 encoding for transactions" checkbox
     * \since QGIS 3.16
     */
    QCheckBox *wfsUseGml2EncodingForTransactions() SIP_SKIP;

    /**
     * Returns the "WFS page size" edit
     * \since QGIS 3.2
     */
    QLineEdit *wfsPageSizeLineEdit() SIP_SKIP;

    /**
     * Returns the url.
     * \since QGIS 3.2
     */
    QUrl urlTrimmed() const SIP_SKIP;

    /**
     * Returns the QSettings key for WFS related settings for the connection.
     * \see wmsSettingsKey()
     */
    virtual QString wfsSettingsKey( const QString &base, const QString &connectionName ) const;

    /**
     * Returns the QSettings key for WMS related settings for the connection.
     * \see wfsSettingsKey()
     */
    virtual QString wmsSettingsKey( const QString &base, const QString &connectionName ) const;

    /**
     * Triggers a resync of the GUI widgets for the service specific settings (i.e. WFS
     * and WMS related settings).
     */
    void updateServiceSpecificSettings();

  private:
    ConnectionTypes mTypes = ConnectionWms;

    QString mServiceName;
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    void showHelp();
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsNewHttpConnection::ConnectionTypes )
Q_DECLARE_OPERATORS_FOR_FLAGS( QgsNewHttpConnection::Flags )

// clazy:excludeall=qstring-allocations

#endif //  QGSNEWHTTPCONNECTION_H
