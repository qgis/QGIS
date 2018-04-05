#ifndef QGSPROCESSINGALGORITHMCONFIGURATIONWIDGET_H
#define QGSPROCESSINGALGORITHMCONFIGURATIONWIDGET_H


#include <QWidget>
#include <QVariantMap>

#include "qgis_core.h"

class CORE_EXPORT QgsProcessingAlgorithmConfigurationWidget : public QWidget
{
    Q_OBJECT

  public:
    QgsProcessingAlgorithmConfigurationWidget( QWidget *parent = nullptr );
    virtual ~QgsProcessingAlgorithmConfigurationWidget() = default;
    virtual QVariantMap configuration() const = 0;
    virtual void setConfiguration( const QVariantMap &configuration ) = 0;
};

#endif // QGSPROCESSINGALGORITHMCONFIGURATIONWIDGET_H
