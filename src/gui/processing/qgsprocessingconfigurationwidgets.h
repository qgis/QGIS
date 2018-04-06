/***************************************************************************
                         qgsprocessingconfigurationwidgets.h
                         ---------------------
    begin                : April 2018
    copyright            : (C) 2018 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROCESSINGCONFIGURATIONWIDGETS_H
#define QGSPROCESSINGCONFIGURATIONWIDGETS_H

#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

class QTableWidget;

/**
 * This class is responsible for the management of processing widgets which
 * QGIS internally.
 *
 * Standalone applications that require to show configuration widgets for processing
 * algorithms will need to execute the following code to have the configuration
 * interfaces available.
 *
 * \code{.py}
 * # At startup time
 * QgsApplicationProcessingConfigurationWidgets.initialize()
 *
 * # At exit time
 * QgsApplicationProcessingConfigurationWidgets.cleanup()
 * \endcode
 */
class GUI_EXPORT QgsProcessingConfigurationWidgets
{
  public:

    /**
     * Initialize native configuration widgets.
     */
    static void initialize();

    /**
     * Cleanup native configuration widgets.
     */
    static void cleanup();

  private:
    QgsProcessingConfigurationWidgets() SIP_FORCE;
    static QList<QgsProcessingAlgorithmConfigurationWidgetFactory *> sProcessingAlgorithmConfigurationWidgetFactories;
};

///@cond PRIVATE

#ifndef SIP_RUN

class QgsFilterAlgorithmConfigurationWidget : public QgsProcessingAlgorithmConfigurationWidget
{
    Q_OBJECT

  public:
    QgsFilterAlgorithmConfigurationWidget( QWidget *parent = nullptr );

    QVariantMap configuration() const override;

    void setConfiguration( const QVariantMap &configuration ) override;

  private slots:
    void removeSelectedOutputs();
    void addOutput();

  private:
    QTableWidget *mOutputExpressionWidget;
};

class QgsFilterAlgorithmConfigurationWidgetFactory : public QgsProcessingAlgorithmConfigurationWidgetFactory
{
  public:
    virtual QgsProcessingAlgorithmConfigurationWidget *create( QgsProcessingAlgorithm *algorithm ) const override;
    virtual bool canCreateFor( QgsProcessingAlgorithm *algorithm ) const override;
};

#endif

///@endcond

#endif // QGSPROCESSINGCONFIGURATIONWIDGETS_H
