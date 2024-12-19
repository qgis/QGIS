/***************************************************************************
                         qgsprocessingrasteroptionswidgetwrapper.h
                         ---------------------
    begin                : November 2020
    copyright            : (C) 2020 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGRASTEROPTIONSWIDGETWRAPPER_H
#define QGSPROCESSINGRASTEROPTIONSWIDGETWRAPPER_H

#define SIP_NO_FILE

#include "qgsprocessingwidgetwrapper.h"

class QComboBox;
class QLineEdit;
class QgsRasterFormatSaveOptionsWidget;

/// @cond private

/**
 * \class QgsProcessingRasterOptionsWidgetWrapper
 *
 * A widget wrapper for Processing string parameter. Provides an UI for
 * defining raster creation options using QgsRasterFormatSaveOptionsWidget.
 *
 * \ingroup gui
 * \since QGIS 3.38
 */
class GUI_EXPORT QgsProcessingRasterOptionsWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:
    QgsProcessingRasterOptionsWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr, QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;

  private:
    QLineEdit *mLineEdit = nullptr;
    QgsRasterFormatSaveOptionsWidget *mOptionsWidget = nullptr;

    friend class TestProcessingGui;
};

/// @endcond

#endif // QGSPROCESSINGRASTEROPTIONSWIDGETWRAPPER_H
