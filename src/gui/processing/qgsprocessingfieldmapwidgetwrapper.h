/***************************************************************************
  qgsprocessingfieldmapwidgetwrapper.h
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

#ifndef QGSPROCESSINGFIELDMAPWIDGETWRAPPER_H
#define QGSPROCESSINGFIELDMAPWIDGETWRAPPER_H

#define SIP_NO_FILE

#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingparameterdefinitionwidget.h"

#include "ui_qgsprocessingfieldsmappingpanelbase.h"

class QLineEdit;
class QToolButton;

/// @cond PRIVATE


class GUI_EXPORT QgsProcessingFieldMapPanelWidget : public QgsPanelWidget, private Ui::QgsProcessingFieldMapPanelBase
{
    Q_OBJECT

  public:
    QgsProcessingFieldMapPanelWidget( QWidget *parent = nullptr );

    void setLayer( QgsVectorLayer *layer );
    QgsVectorLayer *layer();
    QVariant value() const;
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
    QgsFieldMappingModel *mModel = nullptr;

    QgsVectorLayer *mLayer = nullptr;
    bool mBlockChangedSignal = false;
};


class GUI_EXPORT QgsProcessingFieldMapParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:
    QgsProcessingFieldMapParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition = nullptr, const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, Qgis::ProcessingParameterFlags flags ) const override;

  private:
    QComboBox *mParentLayerComboBox = nullptr;
};


class GUI_EXPORT QgsProcessingFieldMapWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:
    QgsProcessingFieldMapWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr, QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override SIP_FACTORY;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr
    ) override;

    void postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers ) override;
    int stretch() const override;

  public slots:
    void setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );

  protected:
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;
    const QgsVectorLayer *linkedVectorLayer() const override;

  private:
    QgsProcessingFieldMapPanelWidget *mPanel = nullptr;
    std::unique_ptr<QgsVectorLayer> mParentLayer;

    friend class TestProcessingGui;
};


/// @endcond

#endif // QGSPROCESSINGFIELDMAPWIDGETWRAPPER_H
