/***************************************************************************
                         qgsprocessingmodelerparameterwidget.h
                         -------------------------------------
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


#ifndef QGSPROCESSINGMODELERPARAMETERWIDGET_H
#define QGSPROCESSINGMODELERPARAMETERWIDGET_H

#include <QWidget>
#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QPointer>
#include "qgsprocessingcontext.h"
#include "processing/models/qgsprocessingmodelchildparametersource.h"

class QgsProcessingParameterDefinition;
class QgsAbstractProcessingParameterWidgetWrapper;
class QgsExpressionLineEdit;
class QgsProcessingModelAlgorithm;
class QgsProcessingParameterWidgetContext;
class QgsProcessingContextGenerator;
class QgsFilterLineEdit;

class QLabel;
class QToolButton;
class QMenu;
class QStackedWidget;
class QComboBox;

/**
 * \class QgsProcessingModelerParameterWidget
 *
 * \brief A widget for customising the value of Processing algorithm parameter inside
 * a Processing model.
 *
 * The widget provides choice of the linked source for values for the parameter
 * inside the model. E.g. parameters can be a static value, an evaluated
 * expression results, the output from another child algorithm, or an input
 * parameter for the model itself.
 *
 * Individual modeler parameter widgets are not usually created directly, instead they are
 * constructed through the central registry, via calls to
 * QgsGui::processingGuiRegistry()->createModelerParameterWidget().
 *
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingModelerParameterWidget : public QWidget, public QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingModelerParameterWidget, for the specified
     * \a parameter definition within the given \a model.
     *
     * The ID of the child algorithm within the model must be specified via the \a childId
     * argument. This value corresponds to the QgsProcessingModelChildAlgorithm::childId()
     * string, which uniquely identifies which child algorithm the parameter is associated
     * with inside the given \a model.
     *
     * A Processing \a context must also be specified, which allows the widget
     * to resolve parameter values which are context dependent. The context must
     * last for the lifetime of the widget.
     */
    QgsProcessingModelerParameterWidget( QgsProcessingModelAlgorithm *model,
                                         const QString &childId,
                                         const QgsProcessingParameterDefinition *parameter,
                                         QgsProcessingContext &context,
                                         QWidget *parent SIP_TRANSFERTHIS = nullptr );

    ~QgsProcessingModelerParameterWidget() override;

    /**
     * Sets the \a context in which the modeler parameter widget is shown, e.g., the
     * parent model algorithm and other relevant information which allows the widget
     * to fine-tune its behavior.
     */
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

    /**
     * Registers a Processing context \a generator class that will be used to retrieve
     * a Processing context for the widget when required.
     */
    void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator );

    /**
     * Populates the widget with available sources for the parameter's value, e.g.
     * adding the available child algorithm outputs and model input parameter
     * choices.
     *
     * In order to determine the available source, the \a compatibleParameterTypes and
     * \a compatibleOutputTypes arguments are used. These lists correspond to the
     * various available values for QgsProcessingParameterDefinition::type() (for
     * \a compatibleParameterTypes) and the values for QgsProcessingOutputDefinition::type()
     * (for \a compatibleOutputTypes).
     *
     * The \a compatibleDataTypes list corresponds to the compatible data types
     * from QgsProcessing::SourceType.
     */
    void populateSources( const QStringList &compatibleParameterTypes,
                          const QStringList &compatibleOutputTypes,
                          const QList< int > &compatibleDataTypes );

    /**
     * Set the expected expression format \a text, which is shown in the expression builder dialog for the widget
     * when in the "pre-calculated" expression mode. This is purely a text format and no expression validation is made
     * against it.
     */
    void setExpressionHelpText( const QString &text );

    /**
     * Returns the parameter definition associated with this wrapper.
     */
    const QgsProcessingParameterDefinition *parameterDefinition() const;

    /**
     * Creates a label for use identifying the associated parameter.
     *
     * The caller takes ownership of the returned value.
     */
    QLabel *createLabel() SIP_FACTORY;

    /**
     * Sets the current \a value for the parameter.
     *
     * \see value()
     */
    virtual void setWidgetValue( const QgsProcessingModelChildParameterSource &value );

    /**
     * Sets the current \a values for the parameter.
     *
     * \see value()
     * \since QGIS 3.14
     */
    void setWidgetValue( const QList< QgsProcessingModelChildParameterSource > &values );

    /**
     * Sets the widget to a model output, for destination parameters only.
     *
     * \see isModelOutput()
     * \see modelOutputName()
     * \since QGIS 3.14
     */
    void setToModelOutput( const QString &value );

    /**
     * Returns TRUE if the widget is set to the model output mode.
     *
     * \see setToModelOutput()
     * \see modelOutputName()
     * \since QGIS 3.14
     */
    bool isModelOutput() const;

    /**
     * Returns the model output name, if isModelOutput() is TRUE.
     *
     * \see setToModelOutput()
     * \see isModelOutput()
     * \since QGIS 3.14
     */
    QString modelOutputName() const;

    /**
     * Returns the current value of the parameter.
     *
     * \see setWidgetValue()
     */
    virtual QVariant value() const;

    /**
     * Sets the parent \a dialog in which the widget is shown.
     *
     * \since QGIS 3.8
     */
    void setDialog( QDialog *dialog );

    QgsExpressionContext createExpressionContext() const override;

    /**
     * Sets the current source \a type for the parameter.
     *
     * \since QGIS 3.24
     */
    void setSourceType( QgsProcessingModelChildParameterSource::Source type );

  private slots:

    void sourceMenuAboutToShow();
    void sourceMenuActionTriggered( QAction *action );

  private:

    // IMPORTANT - must match order of widgets in the stacked widget!
    enum SourceType
    {
      StaticValue = 0,
      Expression = 1,
      ModelParameter = 2,
      ChildOutput = 3,
      ModelOutput = 4,
    };

    SourceType currentSourceType() const;

    void updateUi();

    QgsProcessingModelAlgorithm *mModel = nullptr;
    const QString mChildId;
    const QgsProcessingParameterDefinition *mParameterDefinition = nullptr;
    QgsProcessingContext &mContext;

    QVariant mStaticValue;
    QString mModelInputParameterName;
    QString mOutputChildId;
    QString mOutputName;
    QString mExpression;

    QToolButton *mSourceButton = nullptr;
    QMenu *mSourceMenu = nullptr;
    QStackedWidget *mStackedWidget = nullptr;

    bool mHasStaticWrapper = false;
    std::unique_ptr< QgsAbstractProcessingParameterWidgetWrapper > mStaticWidgetWrapper;
    QgsExpressionLineEdit *mExpressionWidget = nullptr;
    QComboBox *mModelInputCombo = nullptr;
    QComboBox *mChildOutputCombo = nullptr;
    QgsFilterLineEdit *mModelOutputName = nullptr;

    friend class TestProcessingGui;
};


#endif // QGSPROCESSINGMODELERPARAMETERWIDGET_H
