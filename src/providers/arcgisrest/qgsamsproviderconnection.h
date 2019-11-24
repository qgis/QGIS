#ifndef QGSAMSPROVIDERCONNECTION_H
#define QGSAMSPROVIDERCONNECTION_H


#include "qgsabstractwebserviceproviderconnection.h"

class QgsAmsProviderConnection : public QgsAbstractWebServiceProviderConnection
{
  public:
    QgsAmsProviderConnection( const QString &name );
    QgsAmsProviderConnection( const QString &uri, const QVariantMap &configuration );

    // QgsAbstractWebServiceProviderConnection interface
  public:
    QString layerUri( const QString &layerName ) const override;
    QList<QgsAbstractWebServiceProviderConnection::LayerProperty> layers( const QgsAbstractWebServiceProviderConnection::LayerFlags &flags ) const override;

  private:
    void setDefaultCapabilities();
};

#endif // QGSAMSPROVIDERCONNECTION_H
