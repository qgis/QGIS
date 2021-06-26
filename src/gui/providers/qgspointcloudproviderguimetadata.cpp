/***************************************************************************
                         qgspointcloudproviderguimetadata.cpp
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgssourceselectprovider.h"
#include "qgspointcloudsourceselect.h"
#include "qgspointcloudproviderguimetadata.h"

///@cond PRIVATE

class QgsPointCloudSourceSelectProvider : public QgsSourceSelectProvider
{
  public:

    QString providerKey() const override { return QStringLiteral( "pointcloud" ); }
    QString text() const override { return QObject::tr( "Point Cloud" ); }
    int ordering() const override { return QgsSourceSelectProvider::OrderLocalProvider + 25; }
    QIcon icon() const override { return QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddPointCloudLayer.svg" ) ); }
    QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const override
    {
      return new QgsPointCloudSourceSelect( parent, fl, widgetMode );
    }
};

QgsPointCloudProviderGuiMetadata::QgsPointCloudProviderGuiMetadata()
  : QgsProviderGuiMetadata( QStringLiteral( "pointcloud" ) )
{
}

QList<QgsSourceSelectProvider *> QgsPointCloudProviderGuiMetadata::sourceSelectProviders()
{
  QList<QgsSourceSelectProvider *> providers;
  providers << new QgsPointCloudSourceSelectProvider;
  return providers;
}

///@endcond
