/***************************************************************************
                         qgsprocessingparameterdefinitionwidget.h
                         ----------------------------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
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


#ifndef QGSPROCESSINGPARAMETERDEFINITIONWIDGET_H
#define QGSPROCESSINGPARAMETERDEFINITIONWIDGET_H

#include <QWidget>
#include <QDialog>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "qgsprocessingparameters.h"
#include "qgsexpressioncontextgenerator.h"
#include "qgsprocessingwidgetwrapper.h"

class QgsProcessingContextGenerator;

class QLineEdit;
class QCheckBox;
class QTabWidget;
class QTextEdit;
class QgsColorButton;

/**
 * Abstract base class for widgets which allow users to specify the properties of a
 * Processing parameter.
 *
 * \ingroup gui
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsProcessingAbstractParameterDefinitionWidget : public QWidget, public QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    /**
     * Creates a new QgsProcessingAbstractParameterDefinitionWidget, with the specified \a parent widget.
     *
     * The \a context argument must specify a Processing context, which will be used
     * by the widget to evaluate existing \a definition properties such as default values. Similarly,
     * the \a widgetContext argument specifies the wider GUI context in which the widget
     * will be used.
     *
     * The optional \a definition argument may be used to provide a parameter definition to use
     * to initially populate the widget's state.
     *
     * Additionally, the optional \a algorithm parameter may be used to specify the algorithm or model
     * associated with the parameter.
     */
    QgsProcessingAbstractParameterDefinitionWidget( QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition = nullptr, const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a new instance of a parameter definition, using the current settings defined in the dialog.
     *
     * Common properties for parameters, including the \a name, \a description, and parameter \a flags are passed to the
     * method. Subclass implementations must use these properties when crafting a parameter definition which
     * also respects the additional properties specific to the parameter type handled by the widget subclass.
     */
    virtual QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, Qgis::ProcessingParameterFlags flags ) const = 0 SIP_FACTORY;

    /**
     * Sets the \a context in which the Processing definition widget is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     *
     * Subclasses should take care to call the base class method when reimplementing this method.
     *
     * \see widgetContext()
     * \since QGIS 3.18
     */
    virtual void setWidgetContext( const QgsProcessingParameterWidgetContext &context );

    /**
     * Returns the context in which the Processing definition widget is shown, e.g., the
     * parent model algorithm, a linked map canvas, and other relevant information which allows the widget
     * to fine-tune its behavior.
     *
     * \see setWidgetContext()
     * \since QGIS 3.18
     */
    const QgsProcessingParameterWidgetContext &widgetContext() const;

    /**
     * Registers a Processing context \a generator class that will be used to retrieve
     * a Processing context for the widget when required.
     *
     * \since QGIS 3.18
     */
    void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator );

    QgsExpressionContext createExpressionContext() const override;

  private:
    QgsProcessingContextGenerator *mContextGenerator = nullptr;
    QgsProcessingParameterWidgetContext mWidgetContext;
};


/**
 * A widget which allow users to specify the properties of a Processing parameter.
 *
 * \ingroup gui
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsProcessingParameterDefinitionWidget : public QWidget
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsProcessingParameterDefinitionWidget, for a parameter of the
     * specified \a type.
     *
     * The \a context argument must specify a Processing context, which will be used
     * by the widget to evaluate existing \a definition properties such as default values. Similarly,
     * the \a widgetContext argument specifies the wider GUI context in which the widget
     * will be used.
     *
     * The optional \a definition argument may be used to provide a parameter definition to use
     * to initially populate the widget's state.
     *
     * Additionally, the optional \a algorithm parameter may be used to specify the algorithm or model
     * associated with the parameter.
     *
     */
    QgsProcessingParameterDefinitionWidget( const QString &type, QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition = nullptr, const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a new instance of a parameter definition, using the current settings defined in the dialog.
     *
     * The \a name parameter specifies the name for the newly created parameter.
     */
    QgsProcessingParameterDefinition *createParameter( const QString &name = QString() ) const SIP_FACTORY;

    /**
     * Registers a Processing context \a generator class that will be used to retrieve
     * a Processing context for the widget when required.
     *
     * \since QGIS 3.18
     */
    void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator );

  private:
    QString mType;
    QgsProcessingAbstractParameterDefinitionWidget *mDefinitionWidget = nullptr;
    QLineEdit *mDescriptionLineEdit = nullptr;
    QCheckBox *mRequiredCheckBox = nullptr;
    QCheckBox *mAdvancedCheckBox = nullptr;

    friend class QgsProcessingParameterDefinitionDialog;
};

/**
 * A dialog which allow users to specify the properties of a Processing parameter.
 *
 * \ingroup gui
 * \since QGIS 3.10
 */
class GUI_EXPORT QgsProcessingParameterDefinitionDialog : public QDialog
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsProcessingParameterDefinitionDialog, for a parameter of the
     * specified \a type.
     *
     * The \a context argument must specify a Processing context, which will be used
     * by the widget to evaluate existing \a definition properties such as default values. Similarly,
     * the \a widgetContext argument specifies the wider GUI context in which the widget
     * will be used.
     *
     * The optional \a definition argument may be used to provide a parameter definition to use
     * to initially populate the dialog's state.
     *
     * Additionally, the optional \a algorithm parameter may be used to specify the algorithm or model
     * associated with the parameter.
     *
     */
    QgsProcessingParameterDefinitionDialog( const QString &type, QgsProcessingContext &context, const QgsProcessingParameterWidgetContext &widgetContext, const QgsProcessingParameterDefinition *definition = nullptr, const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a new instance of a parameter definition, using the current settings defined in the dialog.
     *
     * The \a name parameter specifies the name for the newly created parameter.
     */
    QgsProcessingParameterDefinition *createParameter( const QString &name = QString() ) const SIP_FACTORY;

    /**
     * Sets the comments for the parameter.
     * \see comments()
     * \since QGIS 3.14
     */
    void setComments( const QString &comments );

    /**
     * Returns the comments for the parameter.
     * \see setComments()
     * \since QGIS 3.14
     */
    QString comments() const;

    /**
     * Sets the color for the comments for the parameter.
     * \see commentColor()
     * \since QGIS 3.14
     */
    void setCommentColor( const QColor &color );

    /**
     * Returns the color for the comments for the parameter.
     * \see setCommentColor()
     * \since QGIS 3.14
     */
    QColor commentColor() const;

    /**
     * Switches the dialog to the comments tab.
     */
    void switchToCommentTab();

    /**
     * Registers a Processing context \a generator class that will be used to retrieve
     * a Processing context for the widget when required.
     *
     * \since QGIS 3.18
     */
    void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator );

  public slots:
    void accept() override;

  private:
    QTabWidget *mTabWidget = nullptr;
    QTextEdit *mCommentEdit = nullptr;
    QgsColorButton *mCommentColorButton = nullptr;
    QgsProcessingParameterDefinitionWidget *mWidget = nullptr;
};


#endif // QGSPROCESSINGPARAMETERDEFINITIONWIDGET_H
