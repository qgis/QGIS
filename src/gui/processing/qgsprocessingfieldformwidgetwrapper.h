/***************************************************************************
                         qgsprocessingfieldformwidgetwrapper.h
                         ----------------------
    begin                : September 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGFIELDFORMWIDGETWRAPPER_H
#define QGSPROCESSINGFIELDFORMWIDGETWRAPPER_H

#define SIP_NO_FILE

#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingparameterdefinitionwidget.h"

#include "qgsfieldformwidget.h"

/// @cond PRIVATE


class GUI_EXPORT QgsProcessingFieldFormParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingFieldFormParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mParentLayerComboBox = nullptr;

};


class GUI_EXPORT QgsProcessingFieldFormWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingFieldFormWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                         QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override SIP_FACTORY;

    QWidget *createWidget() override SIP_FACTORY;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers ) override;
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

    QgsFieldFormWidget *mPanel = nullptr;
    std::unique_ptr< QgsVectorLayer > mParentLayer;

    friend class TestProcessingGui;
};


/// @endcond

#endif // QGSPROCESSINGFIELDFORMWIDGETWRAPPER_H
