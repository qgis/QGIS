//
// Created by myarjunar on 27/03/17.
//

#ifndef QGSGEONODENEWCONNECTION_H
#define QGSGEONODENEWCONNECTION_H

#include "ui_qgsnewgeonodeconnectionbase.h"
#include "qgsgui.h"
#include "qgsguiutils.h"
#include "qgis_app.h"
#include "qgshelp.h"
#include "qgsauthconfigselect.h"

#include <QNetworkReply>

class APP_EXPORT QgsGeoNodeNewConnection : public QDialog, private Ui::QgsNewGeoNodeConnectionBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsGeoNodeNewConnection( QWidget *parent = nullptr, const QString &connName = QString::null, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    QNetworkReply *request( QString &endPoint );

  public slots:
    void accept() override;
    void okButtonBehavior( const QString & );
    //! Test the connection using the parameters supplied
    void testConnection();

  private:
    QString mBaseKey;
    QString mCredentialsBaseKey;
    QString mOriginalConnName; //store initial name to delete entry in case of rename
    QgsAuthConfigSelect *mAuthConfigSelect = nullptr;
};

#endif //QGSGEONODENEWCONNECTION_H
