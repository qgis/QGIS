/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeform.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsAttributeForm : QWidget
{

%TypeHeaderCode
#include "qgsattributeform.h"
%End
  public:

    enum Mode
    {
      SingleEditMode,
      AddFeatureMode,
      MultiEditMode,
      SearchMode,
      AggregateSearchMode,
    };

    enum FilterType
    {
      ReplaceFilter,
      FilterAnd,
      FilterOr,
    };

    explicit QgsAttributeForm( QgsVectorLayer *vl,
                               const QgsFeature &feature = QgsFeature(),
                               const QgsAttributeEditorContext &context = QgsAttributeEditorContext(),
                               QWidget *parent /TransferThis/ = 0 );
    ~QgsAttributeForm();

    const QgsFeature &feature();

    void hideButtonBox();

    void showButtonBox();

    void disconnectButtonBox();

    void addInterface( QgsAttributeFormInterface *iface /Transfer/ );
%Docstring
Takes ownership

:param iface:
%End

    QgsVectorLayer *layer();
%Docstring
Returns the layer for which this form is shown

:return:  Layer
%End

    bool editable();
%Docstring
Returns if the form is currently in editable mode.

:return: Editable mode of this form
%End

    Mode mode() const;
%Docstring
Returns the current mode of the form.

.. versionadded:: 2.16

.. seealso:: :py:func:`setMode`
%End

    void setMode( Mode mode );
%Docstring
Sets the current mode of the form.

:param mode: form mode

.. versionadded:: 2.16

.. seealso:: :py:func:`mode`
%End

    void setEditCommandMessage( const QString &message );
%Docstring
Sets the edit command message (Undo) that will be used when the dialog is accepted

:param message: The message
%End

    virtual bool eventFilter( QObject *object, QEvent *event );

%Docstring
Intercepts keypress on custom form (escape should not close it)

:param object:   The object for which the event has been sent
:param event:    The event which is being filtered

:return:         true if the event has been handled (key was ESC)
%End

    void setMultiEditFeatureIds( const QgsFeatureIds &fids );
%Docstring
Sets all feature IDs which are to be edited if the form is in multiedit mode

:param fids: feature ID list

.. versionadded:: 2.16
%End

    void setMessageBar( QgsMessageBar *messageBar );
%Docstring
Sets the message bar to display feedback from the form in. This is used in the search/filter
mode to display the count of selected features.

:param messageBar: target message bar

.. versionadded:: 2.16
%End

    QString aggregateFilter() const;
%Docstring
The aggregate filter is only useful if the form is in AggregateFilter mode.
In this case it will return a combined expression according to the chosen filters
on all attribute widgets.

.. versionadded:: 3.0
%End

  signals:

    void attributeChanged( const QString &attribute, const QVariant &value );
%Docstring
Notifies about changes of attributes

:param attribute: The name of the attribute that changed.
:param value:     The new value of the attribute.
%End


    void featureSaved( const QgsFeature &feature );
%Docstring
Is emitted, when a feature is changed or added
%End

    void filterExpressionSet( const QString &expression, QgsAttributeForm::FilterType type );
%Docstring
Is emitted when a filter expression is set using the form.

:param expression: filter expression
:param type: filter type

.. versionadded:: 2.16
%End

    void modeChanged( QgsAttributeForm::Mode mode );
%Docstring
Emitted when the form changes mode.

:param mode: new mode
%End

    void closed();
%Docstring
Emitted when the user selects the close option from the form's button bar.

.. versionadded:: 2.16
%End

    void zoomToFeatures( const QString &filter );
%Docstring
Emitted when the user chooses to zoom to a filtered set of features.

.. versionadded:: 3.0
%End

    void flashFeatures( const QString &filter );
%Docstring
Emitted when the user chooses to flash a filtered set of features.

.. versionadded:: 3.0
%End

  public slots:

    void changeAttribute( const QString &field, const QVariant &value, const QString &hintText = QString() );
%Docstring
Call this to change the content of a given attribute. Will update the editor(s) related to this field.

:param field: The field to change
:param value: The new value
:param hintText: A hint text for non existent joined features
%End

    void setFeature( const QgsFeature &feature );
%Docstring
Update all editors to correspond to a different feature.

:param feature: The feature which will be represented by the form
%End

    bool save();
%Docstring
Save all the values from the editors to the layer.

:return: True if successful
%End

    void resetValues();
%Docstring
Sets all values to the values of the current feature
%End

    void resetSearch();
%Docstring
Resets the search/filter form values.

.. versionadded:: 2.16
%End

    void refreshFeature();
%Docstring
reload current feature
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsattributeform.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
