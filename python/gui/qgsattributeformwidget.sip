/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAttributeFormWidget : QWidget /Abstract/
{
%Docstring

Base class for all widgets shown on a QgsAttributeForm.
Consists of the widget which is visible in edit mode as well as the widget visible in search mode.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsattributeformwidget.h"
%End
  public:

    enum Mode
    {
      DefaultMode,
      MultiEditMode,
      SearchMode,
      AggregateSearchMode,
    };

    explicit QgsAttributeFormWidget( QgsWidgetWrapper *widget, QgsAttributeForm *form );
%Docstring
A new form widget for the wrapper ``widget`` on ``form``.
%End

    virtual void createSearchWidgetWrappers() = 0;
%Docstring
Creates the search widget wrappers for the widget used when the form is in
search mode.

:param context: editor context (not available in Python bindings)
%End

    virtual QString currentFilterExpression() const;
%Docstring
Creates an expression matching the current search filter value and
search properties represented in the widget.

.. versionadded:: 2.16
%End


    void setMode( Mode mode );
%Docstring
Sets the current mode for the widget. The widget will adapt its state and visible widgets to
reflect the updated mode. For example, showing multi edit tool buttons if the mode is set to MultiEditMode.

:param mode: widget mode

.. seealso:: :py:func:`mode`
%End

    Mode mode() const;
%Docstring
Returns the current mode for the widget.

.. seealso:: :py:func:`setMode`
%End

    QgsVectorLayer *layer();
%Docstring
The layer for which this widget and its form is shown.
%End

    QgsAttributeForm *form() const;
%Docstring
The form on which this widget is shown.
%End



    void setSearchWidgetWrapper( QgsSearchWidgetWrapper *wrapper );
%Docstring
Sets the search widget wrapper for the widget used when the form is in
search mode.

:param wrapper: search widget wrapper.

.. note::

   the search widget wrapper should be created using searchWidgetFrame()
as its parent

.. note::

   this method is in place for unit testing only, and is not considered
stable API
%End

    void addAdditionalSearchWidgetWrapper( QgsSearchWidgetWrapper *wrapper );
%Docstring
Adds an additional search widget wrapper.
Used to register a secondary search widget as used for "between" searches.
%End

    QList< QgsSearchWidgetWrapper * > searchWidgetWrappers();
%Docstring
Returns the search widget wrapper used in this widget. The wrapper must
first be created using createSearchWidgetWrapper()

.. note::

   this method is in place for unit testing only, and is not considered
stable API
%End

    void resetSearch();
%Docstring
Resets the search/filter value of the widget.
%End

    bool searchWidgetToolButtonVisible() const;
%Docstring
The visibility of the search widget tool button, that allows (de)activating
this search widgte or defines the comparison operator to use.
%End

    void setSearchWidgetToolButtonVisible( bool searchWidgetToolButtonVisible );
%Docstring
The visibility of the search widget tool button, that allows (de)activating
this search widgte or defines the comparison operator to use.
%End

  protected:




};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
