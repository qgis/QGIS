/***************************************************************************
                             qgsprocessingparameterswidget.h
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGPARAMETERSWIDGET_H
#define QGSPROCESSINGPARAMETERSWIDGET_H

#include "ui_qgsprocessingparameterswidgetbase.h"

#include "qgis.h"
#include "qgis_gui.h"
#include "qgsprocessingcontext.h"
#include "qgsprocessingwidgetwrapper.h"

#include <QPointer>
#include <QWidget>

class QgsProcessingAlgorithm;
class QgsProcessingParameterDefinition;

///@cond NOT_STABLE

/**
 * \ingroup gui
 * \brief A widget which allows users to select the value for the parameters for an algorithm.
 * \note Not stable API
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingParametersWidget : public QgsPanelWidget, public QgsProcessingParametersGenerator, public QgsProcessingContextGenerator, private Ui::QgsProcessingParametersWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingParametersWidget, for the specified \a algorithm.
     */
    QgsProcessingParametersWidget( const QgsProcessingAlgorithm *algorithm, bool inPlace, QgsMapLayer *activeLayer, QgsMessageBar *messageBar, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsProcessingParametersWidget() override;
    const QgsProcessingAlgorithm *algorithm() const;
    QgsProcessingContext *processingContext() const override;
    QVariantMap createProcessingParameters( QgsProcessingParametersGenerator::Flags flags = QgsProcessingParametersGenerator::Flags() ) override;

    /**
     * Validates the parameter values currently shown in the widget.
     *
     * \since QGIS 4.4
     */
    QList< QgsProcessingParametersGenerator::ParameterValidationResult > validate() const;

    /**
     * Validates and returns the parameter values representing the current state of the widget.
     *
     * \param flags flags controlling how parameter values are generated
     * \param validationResults will be set to results of the validation
     *
     * \returns map of current parameter values
     *
     * \since QGIS 4.4
     */
    QVariantMap createAndValidateParameters( QgsProcessingParametersGenerator::Flags flags, QList< QgsProcessingParametersGenerator::ParameterValidationResult > &validationResults SIP_OUT ) const;

    /**
     * Sets parameter values to show in the panel.
     *
     * \since QGIS 4.4
     */
    void setParameters( const QVariantMap &parameters );

    /**
     * Returns the wrapper for the parameter with matching \a name, or NULLPTR if none exists.
     *
     * \since QGIS 4.4
     */
    QgsAbstractProcessingParameterWidgetWrapper *wrapper( const QString &name ) const;

    /**
     * Returns a list of all widget wrappers contained by the panel widget.
     *
     * \since QGIS 4.4
     */
    QList< QgsAbstractProcessingParameterWidgetWrapper * > wrappers() const;

  protected:
    void initWidgets();

    void addParameterWidget( const QgsProcessingParameterDefinition *parameter, QWidget *widget SIP_TRANSFER, int stretch = 0 );
    void addParameterLabel( const QgsProcessingParameterDefinition *parameter, QWidget *label SIP_TRANSFER );

    void addOutputLabel( QWidget *label SIP_TRANSFER );
    void addOutputWidget( QWidget *widget SIP_TRANSFER, int stretch = 0 );

    void addExtraWidget( QWidget *widget SIP_TRANSFER );

  private slots:

    void parameterChanged();

  private:
    std::unique_ptr< QgsProcessingContext > mContext;

    const QgsProcessingAlgorithm *mAlgorithm = nullptr;
    bool mInPlace = false;
    QPointer< QgsMapLayer > mActiveLayer;
    QgsMessageBar *mMessageBar = nullptr;

    QVariantMap mExtraParameters;

    QMap< QString, QgsAbstractProcessingParameterWidgetWrapper * > mWrappers;

    friend class TestProcessingGui;
};

///@endcond

#endif // QGSPROCESSINGPARAMETERSWIDGET_H
