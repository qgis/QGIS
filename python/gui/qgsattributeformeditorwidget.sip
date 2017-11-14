/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformeditorwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsAttributeFormEditorWidget : QWidget
{
%Docstring
 A widget consisting of both an editor widget and additional widgets for controlling the behavior
 of the editor widget depending on a number of possible modes. For instance, if the parent attribute
 form is in the multi edit mode, this widget will show both the editor widget and a tool button for
 controlling the multi edit results.
.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgsattributeformeditorwidget.h"
%End
  public:

    enum Mode
    {
      DefaultMode,
      MultiEditMode,
      SearchMode,
    };

    explicit QgsAttributeFormEditorWidget( QgsEditorWidgetWrapper *editorWidget,
                                           QgsAttributeForm *form /TransferThis/ );
%Docstring
 Constructor for QgsAttributeFormEditorWidget.
 \param editorWidget associated editor widget wrapper (for default/edit modes)
 \param form parent attribute form
%End

    ~QgsAttributeFormEditorWidget();

    void createSearchWidgetWrappers( const QString &widgetId, int fieldIdx,
                                     const QVariantMap &config );

%Docstring
 Creates the search widget wrappers for the widget used when the form is in
 search mode.
 \param widgetId id of the widget type to create a search wrapper for
 \param fieldIdx index of field associated with widget
 \param config configuration which should be used for the widget creation
 \param context editor context (not available in Python bindings)
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

    void initialize( const QVariant &initialValue, bool mixedValues = false );
%Docstring
 Resets the widget to an initial value.
 \param initialValue initial value to show in widget
 \param mixedValues set to true to initially show the mixed values state
%End

    bool hasChanged() const;
%Docstring
 Returns true if the widget's value has been changed since it was initialized.
.. seealso:: initialize()
 :rtype: bool
%End

    QVariant currentValue() const;
%Docstring
 Returns the current value of the attached editor widget.
 :rtype: QVariant
%End

    QString currentFilterExpression() const;
%Docstring
 Creates an expression matching the current search filter value and
 search properties represented in the widget.
.. versionadded:: 2.16
 :rtype: str
%End

    void setConstraintStatus( const QString &constraint, const QString &description, const QString &err, QgsEditorWidgetWrapper::ConstraintResult result );
%Docstring
 Set the constraint status for this widget.
%End

  public slots:

    void setIsMixed( bool mixed );
%Docstring
 Sets whether the widget should be displayed in a "mixed values" mode.
 \param mixed set to true to show in a mixed values state
%End

    void changesCommitted();
%Docstring
 Called when field values have been committed;
%End

    void resetSearch();
%Docstring
 Resets the search/filter value of the widget.
%End

  signals:

    void valueChanged( const QVariant &value );
%Docstring
 Emitted when the widget's value changes
 \param value new widget value
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
 stable AP
%End

    QWidget *searchWidgetFrame();
%Docstring
 Returns the widget which should be used as a parent during construction
 of the search widget wrapper.
.. note::

   this method is in place for unit testing only, and is not considered
 stable AP
 :rtype: QWidget
%End

    QList< QgsSearchWidgetWrapper * > searchWidgetWrappers();
%Docstring
 Returns the search widget wrapper used in this widget. The wrapper must
 first be created using createSearchWidgetWrapper()
.. note::

   this method is in place for unit testing only, and is not considered
 stable AP
 :rtype: list of QgsSearchWidgetWrapper
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeformeditorwidget.h                               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
