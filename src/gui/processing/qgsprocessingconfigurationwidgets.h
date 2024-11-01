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

#define SIP_NO_FILE

#include "qgsprocessingalgorithmconfigurationwidget.h"
#include "qgis_gui.h"

class QTableWidget;

///@cond PRIVATE

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
    QgsProcessingAlgorithmConfigurationWidget *create( const QgsProcessingAlgorithm *algorithm ) const override;
    bool canCreateFor( const QgsProcessingAlgorithm *algorithm ) const override;
};


class QgsConditionalBranchAlgorithmConfigurationWidget : public QgsProcessingAlgorithmConfigurationWidget
{
    Q_OBJECT

  public:
    QgsConditionalBranchAlgorithmConfigurationWidget( QWidget *parent = nullptr );

    QVariantMap configuration() const override;

    void setConfiguration( const QVariantMap &configuration ) override;

  private slots:
    void removeSelectedConditions();
    void addCondition();

  private:
    QTableWidget *mConditionExpressionWidget;
};

class QgsConditionalBranchAlgorithmConfigurationWidgetFactory : public QgsProcessingAlgorithmConfigurationWidgetFactory
{
  public:
    QgsConditionalBranchAlgorithmConfigurationWidget *create( const QgsProcessingAlgorithm *algorithm ) const override;
    bool canCreateFor( const QgsProcessingAlgorithm *algorithm ) const override;
};

///@endcond

#endif // QGSPROCESSINGCONFIGURATIONWIDGETS_H
