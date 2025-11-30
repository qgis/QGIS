/***************************************************************************
  qgsprocessingaggregatewidgetwrapper.h
  ---------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGAGGREGATEWIDGETWRAPPER_H
#define QGSPROCESSINGAGGREGATEWIDGETWRAPPER_H

#define SIP_NO_FILE

#include "ui_qgsprocessingaggregatemappingpanelbase.h"

#include "qgsprocessingparameterdefinitionwidget.h"
#include "qgsprocessingwidgetwrapper.h"

class QLineEdit;
class QToolButton;

/// @cond PRIVATE

class GUI_EXPORT QgsProcessingAggregatePanelWidget : public QgsPanelWidget, private Ui::QgsProcessingAggregateMapPanelBase
{
    Q_OBJECT

  public:
    QgsProcessingAggregatePanelWidget( QWidget *parent = nullptr );

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer();
    [[nodiscard]] QVariant value() const;
    void setValue( const QVariant &value );

    /**
     * Register an expression context \a generator class that will be used to retrieve
     * an expression context for the widget.
     */
    void registerExpressionContextGenerator( const QgsExpressionContextGenerator *generator );

  signals:

    void changed();

  private slots:
    void loadFieldsFromLayer();
    void addField();
    void loadLayerFields();

  private:
    QgsAggregateMappingModel *mModel = nullptr;

    QgsVectorLayer *mLayer = nullptr;
    bool mBlockChangedSignal = false;

    bool mSkipConfirmDialog = false;
    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingAggregateParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:
    QgsProcessingAggregateParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition = nullptr, const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    [[nodiscard]] QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, Qgis::ProcessingParameterFlags flags ) const override;

  private:
    QComboBox *mParentLayerComboBox = nullptr;
};


class GUI_EXPORT QgsProcessingAggregateWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:
    QgsProcessingAggregateWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr, Qgis::ProcessingMode type = Qgis::ProcessingMode::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    [[nodiscard]] QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, Qgis::ProcessingMode type ) override SIP_FACTORY;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr
    ) override;

    void postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers ) override;
    [[nodiscard]] int stretch() const override;

  public slots:
    void setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );

  protected:
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    [[nodiscard]] QVariant widgetValue() const override;

    [[nodiscard]] QString modelerExpressionFormatString() const override;
    [[nodiscard]] const QgsVectorLayer *linkedVectorLayer() const override;

  private:
    QgsProcessingAggregatePanelWidget *mPanel = nullptr;
    std::unique_ptr<QgsVectorLayer> mParentLayer;

    friend class TestProcessingGui;
};


/// @endcond

#endif // QGSPROCESSINGAGGREGATEWIDGETWRAPPER_H
