/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/processing/qgsprocessingalgorithmconfigurationwidget.h       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/








class QgsProcessingAlgorithmConfigurationWidget : QWidget, QgsExpressionContextGenerator
{
%Docstring
A configuration widget for processing algorithms allows providing additional
configuration options directly on algorithm level, in addition to parameters.

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsprocessingalgorithmconfigurationwidget.h"
%End
  public:

    QgsProcessingAlgorithmConfigurationWidget( QWidget *parent = 0 );
%Docstring
Creates a new QgsProcessingAlgorithmConfigurationWidget
%End
    ~QgsProcessingAlgorithmConfigurationWidget();

    virtual QVariantMap configuration() const = 0;
%Docstring
Read the current configuration from this widget.
%End

    virtual void setConfiguration( const QVariantMap &configuration ) = 0;
%Docstring
Set the configuration which this widget should represent.
%End

    virtual void setWidgetContext( const QgsProcessingParameterWidgetContext &context );
%Docstring
Sets the ``context`` in which the Processing algorithm widget is shown, e.g., the
parent model algorithm, a linked map canvas, and other relevant information which allows the widget
to fine-tune its behavior.

Subclasses should take care to call the base class method when reimplementing this method.

.. seealso:: :py:func:`widgetContext`

.. versionadded:: 3.6
%End

    const QgsProcessingParameterWidgetContext &widgetContext() const;
%Docstring
Returns the context in which the Processing algorithm widget is shown, e.g., the
parent model algorithm, a linked map canvas, and other relevant information which allows the widget
to fine-tune its behavior.

.. seealso:: :py:func:`setWidgetContext`

.. versionadded:: 3.6
%End

    void setAlgorithm( const QgsProcessingAlgorithm *algorithm );
%Docstring
Sets the algorithm instance associated with the widget.

.. seealso:: :py:func:`algorithm`

.. versionadded:: 3.6
%End

    const QgsProcessingAlgorithm *algorithm() const;
%Docstring
Returns the algorithm instance associated with this widget.

.. seealso:: :py:func:`setAlgorithm`

.. versionadded:: 3.6
%End

    void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator );
%Docstring
Registers a Processing context ``generator`` class that will be used to retrieve
a Processing context for the widget when required.

.. versionadded:: 3.6
%End

    virtual QgsExpressionContext createExpressionContext() const;


};


class QgsProcessingAlgorithmConfigurationWidgetFactory
{
%Docstring
Interface base class for factories for algorithm configuration widgets.

.. versionadded:: 3.2
%End

%TypeHeaderCode
#include "qgsprocessingalgorithmconfigurationwidget.h"
%End
  public:
    virtual ~QgsProcessingAlgorithmConfigurationWidgetFactory();

    virtual QgsProcessingAlgorithmConfigurationWidget *create( const QgsProcessingAlgorithm *algorithm ) const = 0 /Factory/;
%Docstring
Create a new configuration widget for ``algorithm``.
%End

    virtual bool canCreateFor( const QgsProcessingAlgorithm *algorithm ) const = 0;
%Docstring
Check if this factory can create widgets for ``algorithm``.
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/processing/qgsprocessingalgorithmconfigurationwidget.h       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
