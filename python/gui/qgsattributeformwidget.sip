/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAttributeFormWidget : QWidget /Abstract/
{

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

    virtual void createSearchWidgetWrappers() = 0;
%Docstring
 Creates the search widget wrappers for the widget used when the form is in
 search mode.

 \param context editor context (not available in Python bindings)
%End

    virtual QString currentFilterExpression() const;
%Docstring
 Creates an expression matching the current search filter value and
 search properties represented in the widget.
.. versionadded:: 2.16
 :rtype: str
%End


    void setMode( Mode mode );
%Docstring
 Sets the current mode for the widget. The widget will adapt its state and visible widgets to
 reflect the updated mode. For example, showing multi edit tool buttons if the mode is set to MultiEditMode.
 \param mode widget mode
.. seealso:: mode()
%End

    Mode mode() const;
%Docstring
 Returns the current mode for the widget.
.. seealso:: setMode()
 :rtype: Mode
%End

    QgsVectorLayer *layer();
%Docstring
 :rtype: QgsVectorLayer
%End

    QgsAttributeForm *form() const;
%Docstring
 :rtype: QgsAttributeForm
%End



    void setSearchWidgetWrapper( QgsSearchWidgetWrapper *wrapper );
%Docstring
 Sets the search widget wrapper for the widget used when the form is in
 search mode.
 \param wrapper search widget wrapper.
.. note::

   the search widget wrapper should be created using searchWidgetFrame()
 as its parent
.. note::

   this method is in place for unit testing only, and is not considered
 stable API
%End

    void addAdditionalSearchWidgetWrapper( QgsSearchWidgetWrapper *wrapper );

    QList< QgsSearchWidgetWrapper * > searchWidgetWrappers();
%Docstring
 Returns the search widget wrapper used in this widget. The wrapper must
 first be created using createSearchWidgetWrapper()
.. note::

   this method is in place for unit testing only, and is not considered
 stable API
 :rtype: list of QgsSearchWidgetWrapper
%End

    void resetSearch();
%Docstring
 Resets the search/filter value of the widget.
%End

  protected:



    QgsSearchWidgetToolButton *searchWidgetToolButton();
%Docstring
 Returns a pointer to the search widget tool button in the widget.
.. note::

   this method is in place for unit testing only, and is not considered
 stable API
 :rtype: QgsSearchWidgetToolButton
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
