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
#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QPointer>

class QgsProcessingParameterDefinition;
class QgsProcessingContext;
class QLabel;

/**
 * \class QgsAbstractProcessingParameterWidgetWrapper
 *
 * A widget wrapper for Processing parameter value widgets.
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
class GUI_EXPORT QgsAbstractProcessingParameterWidgetWrapper : public QObject
{
    Q_OBJECT

  public:

    //! Types of dialogs the widgets can be created for
    enum WidgetType
    {
      Standard, //!< Standard algorithm dialog
      Batch, //!< Batch processing dialog
      Modeler, //!< Modeler dialog
    };

    /**
     * Constructor for QgsAbstractProcessingParameterWidgetWrapper, for the specified
     * \a parameter definition and dialog \a type.
     */
    QgsAbstractProcessingParameterWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        WidgetType type = Standard, QObject *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the dialog type for which widgets and labels will be created by this wrapper.
     */
    WidgetType type() const;

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
    QWidget *createWrappedWidget( const QgsProcessingContext &context ) SIP_FACTORY;

    /**
     * Creates and returns a new label to accompany widgets created by the wrapper.
     *
     * The caller takes ownership of the returned label. Some parameter type and dialog type
     * combinations will return nullptr for this method. If nullptr is returned, then no
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

    /**
     * Sets the current \a value for the parameter.
     *
     * The \a context argument is used to specify the wider Processing context which the
     * current value is associated with.
     *
     * \see value()
     */
    virtual void setWidgetValue( const QVariant &value, const QgsProcessingContext &context ) = 0;

    /**
     * Returns the current value of the parameter.
     *
     * \see setWidgetValue()
     */
    virtual QVariant value() const = 0;

    /**
     * Called after all wrappers have been created within a particular dialog or context,
     * allowing the wrapper to connect to the wrappers of other, related parameters.
     */
    virtual void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers );

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
     * combinations will return nullptr for this method. If nullptr is returned, then no
     * label should be shown for the parameter's widget (i.e. the label is embedded inside the
     * widget itself).
     *
     * \see createWidget()
     */
    virtual QLabel *createLabel() SIP_FACTORY;

  private:

    WidgetType mType = Standard;
    const QgsProcessingParameterDefinition *mParameterDefinition = nullptr;

    QPointer< QWidget > mWidget;
    QPointer< QLabel > mLabel;

};


/**
 * \class QgsProcessingParameterWidgetFactoryInterface
 *
 * An interface for Processing widget wrapper factories.
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
     */
    virtual QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter,
        QgsAbstractProcessingParameterWidgetWrapper::WidgetType type ) = 0 SIP_FACTORY;

};

#endif // QGSPROCESSINGWIDGETWRAPPER_H
