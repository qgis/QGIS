/***************************************************************************
                         qgsprocessingwidgetwrapper.h
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

#ifndef QGSPROCESSINGWIDGETWRAPPER_H
#define QGSPROCESSINGWIDGETWRAPPER_H

#include <QObject>
#include <QWidget>
#include <QPointer>
#include <memory>
#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsprocessinggui.h"
#include "qgsvectorlayer.h"
#include "qgsprocessingmodelchildparametersource.h"

class QgsProcessingParameterDefinition;
class QgsProcessingContext;
class QgsProcessingModelerParameterWidget;
class QgsProcessingModelAlgorithm;
class QLabel;
class QgsPropertyOverrideButton;
class QgsVectorLayer;
class QgsProcessingModelAlgorithm;
class QgsMapCanvas;
class QgsProcessingAlgorithm;
class QgsProcessingAbstractParameterDefinitionWidget;
class QgsMessageBar;
class QgsBrowserGuiModel;

/**
 * \class QgsProcessingContextGenerator
 * \brief An interface for objects which can create Processing contexts.
 *
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingContextGenerator
{
  public:

    /**
     * This method needs to be reimplemented in all classes which implement this interface
     * and return a Processing context.
     *
     * Note that ownership of the context is not transferred - it is intended that subclasses
     * return a pointer to a context which they have already created and own.
     */
    virtual QgsProcessingContext *processingContext() = 0;

    virtual ~QgsProcessingContextGenerator() = default;
};

/**
 * \class QgsProcessingParametersGenerator
 *
 * \brief An interface for objects which can create sets of parameter values for processing algorithms.
 *
 * \ingroup gui
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingParametersGenerator
{
  public:

    /**
     * Flags controlling parameter generation.
     *
     * \since QGIS 3.24
     */
    enum class Flag : int
    {
      SkipDefaultValueParameters = 1 << 0, //!< Parameters which are unchanged from their default values should not be included
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    /**
     * This method needs to be reimplemented in all classes which implement this interface
     * and return a algorithm parameters.
     *
     * Since QGIS 3.24 the optional \a flags argument can be used to control the behavior
     * of the parameter generation.
     */
    virtual QVariantMap createProcessingParameters( QgsProcessingParametersGenerator::Flags flags = QgsProcessingParametersGenerator::Flags() ) = 0;

    virtual ~QgsProcessingParametersGenerator() = default;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsProcessingParametersGenerator::Flags )


/**
 * \ingroup gui
 * \class QgsProcessingParameterWidgetContext
 * \brief Contains settings which reflect the context in which a Processing parameter widget is shown, e.g., the
 * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
 * to fine-tune its behavior.
 *
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingParameterWidgetContext
{
  public:

    /**
     * Constructor for QgsProcessingParameterWidgetContext.
     */
    QgsProcessingParameterWidgetContext() = default;

    /**
     * Sets the map \a canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * \see mapCanvas()
     */
    void setMapCanvas( QgsMapCanvas *canvas );

    /**
     * Returns the map canvas associated with the widget.
     * \see setMapCanvas()
     */
    QgsMapCanvas *mapCanvas() const;

    /**
     * Sets the message \a bar associated with the widget. This allows the widget to push feedback messages
     * to the user.
     * \see messageBar()
     * \since QGIS 3.12
     */
    void setMessageBar( QgsMessageBar *bar );

    /**
     * Returns the message bar associated with the widget. This allows the widget to push feedback messages
     * to the user.
     * \see setMessageBar()
     * \since QGIS 3.12
     */
    QgsMessageBar *messageBar() const;

    /**
     * Sets the browser \a model associated with the widget. This will usually be the shared app instance of the browser model
     * \see browserModel()
     * \since QGIS 3.14
     */
    void setBrowserModel( QgsBrowserGuiModel *model );

    /**
     * Returns the browser model associated with the widget.
     * \see setBrowserModel()
     * \since QGIS 3.12
     */
    QgsBrowserGuiModel *browserModel() const;

    /**
     * Sets the \a project associated with the widget. This allows the widget to retrieve the map layers
     * and other properties from the correct project.
     * \see project()
     * \since QGIS 3.8
     */
    void setProject( QgsProject *project );

    /**
     * Returns the project associated with the widget.
     * \see setProject()
     */
    QgsProject *project() const;

    /**
     * Returns the model which the parameter widget is associated with.
     *
     * \see setModel()
     * \see modelChildAlgorithmId()
     */
    QgsProcessingModelAlgorithm *model() const;

    /**
     * Sets the \a model which the parameter widget is associated with.
     *
     * \see model()
     * \see setModelChildAlgorithmId()
     */
    void setModel( QgsProcessingModelAlgorithm *model );

    /**
     * Returns the child algorithm ID within the model which the parameter widget is associated with.
     *
     * \see setModelChildAlgorithmId()
     * \see model()
     */
    QString modelChildAlgorithmId() const;

    /**
     * Sets the child algorithm \a id within the model which the parameter widget is associated with.
     *
     * \see modelChildAlgorithmId()
     * \see setModel()
     */
    void setModelChildAlgorithmId( const QString &id );

    /**
     * Returns the current active layer.
     *
     * \see setActiveLayer()
     * \since QGIS 3.14
     */
    QgsMapLayer *activeLayer() const;

    /**
     * Sets the current active \a layer.
     *
     * \see activeLayer()
     * \since QGIS 3.14
     */
    void setActiveLayer( QgsMapLayer *layer );

  private:

    QgsProcessingModelAlgorithm *mModel = nullptr;

    QString mModelChildAlgorithmId;

    QgsMapCanvas *mMapCanvas = nullptr;

    QgsMessageBar *mMessageBar = nullptr;

    QgsProject *mProject = nullptr;

    QgsBrowserGuiModel *mBrowserModel = nullptr;

    QgsMapLayer *mActiveLayer = nullptr;

};

#ifndef SIP_RUN
///@cond PRIVATE
class GUI_EXPORT QgsProcessingGuiUtils
{
  public:

    static QgsExpressionContext createExpressionContext( QgsProcessingContextGenerator *processingContextGenerator = nullptr,
        const QgsProcessingParameterWidgetContext &widgetContext = QgsProcessingParameterWidgetContext(),
        const QgsProcessingAlgorithm *algorithm = nullptr,
        const QgsVectorLayer *linkedLayer = nullptr );


};
///@endcond
#endif

/**
 * \class QgsAbstractProcessingParameterWidgetWrapper
 *
 * \brief A widget wrapper for Processing parameter value widgets.
 *
 * Widget wrappers are used to create widgets for individual Processing parameters, and
 * handle retrieving and setting the current value for those parameters.
 *
 * Widget wrappers can be created for different dialog types, allowing different
 * appearance and behavior of widgets depending on whether they are being created for
 * use in a standard algorithm dialog, a batch processing dialog, or a modeler dialog.
 *
 * Individual widget wrappers are not usually created directly, instead they are
 * constructed through the central registry, via calls to
 * QgsGui::processingGuiRegistry()->createParameterWidgetWrapper().
 *
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsAbstractProcessingParameterWidgetWrapper : public QObject, public QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsAbstractProcessingParameterWidgetWrapper, for the specified
     * \a parameter definition and dialog \a type.
     */
    QgsAbstractProcessingParameterWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the dialog type for which widgets and labels will be created by this wrapper.
     */
    QgsProcessingGui::WidgetType type() const;

    /**
     * Sets the \a context in which the Processing parameter widget is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     *
     * Subclasses should take care to call the base class method when reimplementing this method.
     *
     * \see widgetContext()
     */
    virtual void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

    /**
     * Returns the context in which the Processing parameter widget is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     *
     * \see setWidgetContext()
     */
    const QgsProcessingParameterWidgetContext &widgetContext() const;

    /**
     * Creates and return a new wrapped widget which allows customization of the parameter's value.
     *
     * The caller takes ownership of the returned widget. The wrapped widget can be retrieved at a later
     * stage by calling wrappedWidget().
     *
     * The supplied \a context is used while initializing the widget to the parameter's default value.
     *
     * \see createWrappedLabel()
     */
    QWidget *createWrappedWidget( QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Creates and returns a new label to accompany widgets created by the wrapper.
     *
     * The caller takes ownership of the returned label. Some parameter type and dialog type
     * combinations will return NULLPTR for this method. If NULLPTR is returned, then no
     * label should be shown for the parameter's widget (i.e. the label is embedded inside the
     * widget itself).
     *
     * The wrapped label can be retrieved at a later stage by calling wrappedLabel().
     *
     * \see createWrappedWidget()
     */
    QLabel *createWrappedLabel() SIP_FACTORY;

    /**
     * Returns the current wrapped widget, if any.
     * \see createWrappedWidget()
     */
    QWidget *wrappedWidget();

    /**
     * Returns the current wrapped label, if any.
     * \see createWrappedLabel()
     */
    QLabel *wrappedLabel();

    /**
     * Returns the parameter definition associated with this wrapper.
     */
    const QgsProcessingParameterDefinition *parameterDefinition() const;

    // TODO QGIS 4.0 -- remove
#ifdef SIP_RUN
    % Property( name = param, get = parameterDefinition )
#endif

    /**
     * Sets the current \a value for the parameter.
     *
     * The \a context argument is used to specify the wider Processing context which the
     * current value is associated with.
     *
     * \see parameterValue()
     */
    void setParameterValue( const QVariant &value, QgsProcessingContext &context );

    /**
     * Returns the current value of the parameter.
     *
     * \see setParameterValue()
     */
    QVariant parameterValue() const;

    /**
     * Returns any custom properties set by the wrapper.
     */
    virtual QVariantMap customProperties() const;

    /**
     * Registers a Processing context \a generator class that will be used to retrieve
     * a Processing context for the wrapper when required.
     *
     * Care must be taken to call the base class implementation if overwrite this method.
     */
    virtual void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator );

    /**
     * Registers a Processing parameters \a generator class that will be used to retrieve
     * algorithm parameters for the wrapper when required (e.g. when a wrapper needs access
     * to other parameter's values).
     *
     * \since QGIS 3.14
     */
    void registerProcessingParametersGenerator( QgsProcessingParametersGenerator *generator );

    /**
     * Called after all wrappers have been created within a particular dialog or context,
     * allowing the wrapper to connect to the wrappers of other, related parameters.
     */
    virtual void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers );

    /**
     * Returns the Qt layout "stretch" factor to use when adding this widget to a layout.
     *
     * The default implementation returns 0.
     *
     * \since QGIS 3.14
     */
    virtual int stretch() const;

    QgsExpressionContext createExpressionContext() const override;

    /**
     * Sets the parent \a dialog in which the wrapper is shown.
     *
     * \since QGIS 3.8
     */
    virtual void setDialog( QDialog *dialog );

  signals:

    // TODO QGIS 4.0 - remove wrapper parameter - this is kept for compatibility with 3.x API,
    // yet can easily be retrieved by checking the sender()

    /**
     * Emitted whenever the parameter value (as defined by the wrapped widget) is changed.
     */
    void widgetValueHasChanged( QgsAbstractProcessingParameterWidgetWrapper *wrapper );

  protected:

    /**
     * Creates a new widget which allows customization of the parameter's value.
     *
     * The caller takes ownership of the returned widget.
     *
     * \see createLabel()
     */
    virtual QWidget *createWidget() = 0 SIP_FACTORY;

    /**
     * Creates a new label to accompany widgets created by the wrapper.
     *
     * The caller takes ownership of the returned label. Some parameter type and dialog type
     * combinations will return NULLPTR for this method. If NULLPTR is returned, then no
     * label should be shown for the parameter's widget (i.e. the label is embedded inside the
     * widget itself).
     *
     * \see createWidget()
     */
    virtual QLabel *createLabel() SIP_FACTORY;

    /**
     * Sets the current \a value for the parameter to show in the widget.
     *
     * The \a context argument is used to specify the wider Processing context which the
     * current value is associated with.
     *
     * \see widgetValue()
     */
    virtual void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) = 0;

    /**
     * Returns the current value of the parameter.
     *
     * \see setWidgetValue()
     */
    virtual QVariant widgetValue() const = 0;

    /**
     * Returns the optional vector layer associated with this widget wrapper, or NULLPTR if no vector
     * layer is applicable.
     *
     * This is used to correctly generate expression contexts within the GUI, e.g. to allow expression
     * buttons and property override buttons to correctly show the appropriate vector layer fields.
     *
     * \since QGIS 3.6
     */
    virtual const QgsVectorLayer *linkedVectorLayer() const;

  protected:

    QgsProcessingContextGenerator *mProcessingContextGenerator = nullptr;
    QgsProcessingParametersGenerator *mParametersGenerator = nullptr;
    QgsProcessingParameterWidgetContext mWidgetContext;

  private slots:

    void parentLayerChanged( QgsAbstractProcessingParameterWidgetWrapper *wrapper );

  private:

    QgsProcessingGui::WidgetType mType = QgsProcessingGui::Standard;
    const QgsProcessingParameterDefinition *mParameterDefinition = nullptr;

    void setDynamicParentLayerParameter( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );

    QPointer< QWidget > mWidget;
    QPointer< QgsPropertyOverrideButton > mPropertyButton;
    QPointer< QLabel > mLabel;
    std::unique_ptr< QgsVectorLayer > mDynamicLayer;

    friend class TestProcessingGui;

};


/**
 * \class QgsProcessingParameterWidgetFactoryInterface
 *
 * \brief An interface for Processing widget wrapper factories.
 *
 * Widget wrapper factories allow creation of QgsAbstractProcessingParameterWidgetWrapper objects.
 * They are centrally managed by QgsProcessingGuiRegistry. Usually, individual factories
 * are not directly utilized, rather the QgsGui::processingGuiRegistry()->createParameterWidgetWrapper()
 * method is used to create widget wrappers.
 *
 * \ingroup gui
 * \since QGIS 3.4
 */
class GUI_EXPORT QgsProcessingParameterWidgetFactoryInterface
{

  public:

    virtual ~QgsProcessingParameterWidgetFactoryInterface() = default;

    /**
     * Returns the type string for the parameter type the factory is associated with.
     */
    virtual QString parameterType() const = 0;

    /**
     * Creates a new widget wrapper for the specified \a parameter definition.
     *
     * The \a type argument indicates the dialog type to create a wrapper for.
     *
     * \see createModelerWidgetWrapper()
     */
    virtual QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter,
        QgsProcessingGui::WidgetType type ) = 0 SIP_FACTORY;

    /**
     * Creates a new modeler parameter widget for the given \a parameter. This widget allows
     * configuration of the parameter's value when used inside a Processing \a model.
     *
     * The ID of the child algorithm within the model must be specified via the \a childId
     * argument. This value corresponds to the QgsProcessingModelChildAlgorithm::childId()
     * string, which uniquely identifies which child algorithm the parameter is associated
     * with inside the given \a model.
     *
     * A Processing \a context must also be specified, which allows the widget
     * to resolve parameter values which are context dependent. The context must
     * last for the lifetime of the widget.
     *
     * \see createWidgetWrapper()
     */
    virtual QgsProcessingModelerParameterWidget *createModelerWidgetWrapper( QgsProcessingModelAlgorithm *model,
        const QString &childId,
        const QgsProcessingParameterDefinition *parameter,
        QgsProcessingContext &context );

    /**
     * Creates a new parameter definition widget allowing for configuration of an instance of
     * the parameter type handled by this factory.
     *
     * The \a context argument must specify a Processing context, which will be used
     * by the widget to evaluate existing \a definition properties such as default values. Similarly,
     * the \a widgetContext argument specifies the wider GUI context in which the widget
     * will be used.
     *
     * The optional \a definition argument may specify a parameter definition which
     * should be reflected in the initial state of the returned widget. Subclasses must
     * ensure that they correctly handle both the case when a initial \a definition is
     * passed, or when \a definition is NULLPTR (in which case sensible defaults should
     * be shown in the returned widget).
     *
     * Additionally, the optional \a algorithm parameter may be used to specify the algorithm or model
     * associated with the parameter.
     *
     * If a factory subclass returns NULLPTR for this method (i.e. as the base class implementation does),
     * it indicates that the parameter type cannot be configured via GUI. In this case the parameter
     * type will not be configurable when users add it as an input to their graphical models.
     *
     * \since QGIS 3.10
     */
    virtual QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) SIP_FACTORY;

  protected:

    /**
     * Returns a list of compatible Processing parameter types for inputs
     * for this parameter.
     *
     * In order to determine the available sources for the parameter in a model
     * the types returned by this method are checked. The returned list corresponds to the
     * various available values for QgsProcessingParameterDefinition::type().
     *
     * Subclasses should return a list of all QgsProcessingParameterDefinition::type()
     * values which can be used as input values for the parameter.
     *
     * \see compatibleOutputTypes()
     * \see compatibleDataTypes()
     */
    virtual QStringList compatibleParameterTypes() const = 0;

    /**
     * Returns a list of compatible Processing output types for inputs
     * for this parameter.
     *
     * In order to determine the available sources for the parameter in a model
     * the types returned by this method are checked. The returned list corresponds to the
     * various available values for QgsProcessingOutputDefinition::type().
     *
     * Subclasses should return a list of all QgsProcessingOutputDefinition::type()
     * values which can be used as values for the parameter.
     *
     * \see compatibleParameterTypes()
     * \see compatibleDataTypes()
     */
    virtual QStringList compatibleOutputTypes() const = 0;

    /**
     * Returns a list of compatible Processing data types for inputs
     * for this widget for the specified \a parameter.
     *
     * In order to determine the available sources for the parameter in a model
     * the types returned by this method are checked. The returned list corresponds
     * to the various available values from QgsProcessing::SourceType.
     *
     * Subclasses should return a list of all QgsProcessing::SourceType
     * values which can be used as values for the parameter.
     *
     * \see compatibleParameterTypes()
     * \see compatibleOutputTypes()
     */
    virtual QList< int > compatibleDataTypes( const QgsProcessingParameterDefinition *parameter ) const;

    /**
     * Returns the expected expression format string for expression results for the parameter
     * within model child algorithms.
     *
     * This is shown in modeler widget wrappers when using the "pre-calculated" expression mode,
     * and should give helpful text to users to indicate the expected results from the expression.

     * This is purely a text format and no expression validation is made against it.
     */
    virtual QString modelerExpressionFormatString() const;

    /**
     * Returns the default source type to use for the widget for the specified \a parameter.
     *
     * \since QGIS 3.24
     */
    virtual QgsProcessingModelChildParameterSource::Source defaultModelSource( const QgsProcessingParameterDefinition *parameter ) const;

};

/**
 * \class QgsProcessingHiddenWidgetWrapper
 *
 * \brief An widget wrapper for hidden widgets.
 *
 * The hidden widget wrapper allows for creation of a widget wrapper which does not provide
 * a graphical widget, yet still implements the QgsAbstractProcessingParameterWidgetWrapper
 * interface.
 *
 * \ingroup gui
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProcessingHiddenWidgetWrapper: public QgsAbstractProcessingParameterWidgetWrapper
{
  public:

    /**
     * Constructor for QgsProcessingHiddenWidgetWrapper, for the specified
     * \a parameter definition and dialog \a type.
     */
    QgsProcessingHiddenWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                      QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard,
                                      QObject *parent SIP_TRANSFERTHIS = nullptr );

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    const QgsVectorLayer *linkedVectorLayer() const override;

    /**
     * Sets the vector layer linked to the wrapper.
     */
    void setLinkedVectorLayer( const QgsVectorLayer *layer );

  protected:
    QWidget *createWidget() override;
    QLabel *createLabel() override;

  private:

    QVariant mValue;
    QPointer < const QgsVectorLayer > mLayer;

};

#endif // QGSPROCESSINGWIDGETWRAPPER_H
