/***************************************************************************
                         qgsprocessingwidgetwrapperimpl.h
                         ---------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROCESSINGWIDGETWRAPPERIMPL_H
#define QGSPROCESSINGWIDGETWRAPPERIMPL_H

#define SIP_NO_FILE
#include "qgsprocessingwidgetwrapper.h"

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QgsProjectionSelectionWidget;

///@cond PRIVATE

class GUI_EXPORT QgsProcessingBooleanWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingBooleanWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                       QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    QLabel *createLabel() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QList< int > compatibleDataTypes() const override;

  private:

    QCheckBox *mCheckBox = nullptr;
    QComboBox *mComboBox = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingCrsWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingCrsWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                   QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

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
    QList< int > compatibleDataTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsProjectionSelectionWidget *mProjectionSelectionWidget = nullptr;
    QCheckBox *mUseProjectCrsCheckBox = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingStringWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingStringWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                      QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

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

    QList< int > compatibleDataTypes() const override;

  private:

    QLineEdit *mLineEdit = nullptr;
    QPlainTextEdit *mPlainTextEdit = nullptr;

    friend class TestProcessingGui;
};


///@endcond PRIVATE

#endif // QGSPROCESSINGWIDGETWRAPPERIMPL_H
