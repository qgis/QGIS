/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgspropertyassistantwidget.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsPropertyAssistantWidget : QgsPanelWidget
{
%Docstring
Shows a user-friendly assistant guiding users through the creation of QgsProperty overrides.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspropertyassistantwidget.h"
%End
  public:

    QgsPropertyAssistantWidget( QWidget *parent /TransferThis/ = 0,
                                const QgsPropertyDefinition &definition = QgsPropertyDefinition(),
                                const QgsProperty &initialState = QgsProperty(),
                                const QgsVectorLayer *layer = 0 );
%Docstring
Constructor for QgsPropertyAssistantWidget. Aside from a ``parent`` widget, the constructor accepts a
corresponding property ``definition`` from which it customizes the displayed options (eg a color based
property definition will show an assistant to allow creation of color based properties).
The ``initialState`` dictates the initial state to show in the widget. A corresponding ``layer``
can also be set to allow population of GUI widgets such as field selectors.
%End

    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );
%Docstring
Register an expression context generator class that will be used to retrieve
an expression context for the button when required.
%End

    void updateProperty( QgsProperty &property );
%Docstring
Updates a ``property`` in place to corresponding to the current settings shown
in the widget.
%End


    virtual void setDockMode( bool dockMode );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgspropertyassistantwidget.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
