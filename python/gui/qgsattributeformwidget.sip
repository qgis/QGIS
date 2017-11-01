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
    };

    explicit QgsAttributeFormWidget( QgsWidgetWrapper *widget, QgsAttributeForm *form );

    virtual void createSearchWidgetWrappers() = 0;
%Docstring
 Creates the search widget wrappers for the widget used when the form is in
 search mode.

 \param context editor context (not available in Python bindings)
%End

    virtual QString currentFilterExpression() const = 0;
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

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformwidget.h                                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
