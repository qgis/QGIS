/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgspropertyoverridebutton.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsPropertyOverrideButton: QToolButton
{
%Docstring
A button for controlling property overrides which may apply to a widget.

QgsPropertyOverrideButton is designed to be used alongside the QGIS
properties framework (QgsProperty, QgsPropertyDefinition
and QgsPropertyCollection).

It allows users to specify field or expression based overrides
which should be applied to a property of an object. Eg, this widget
is used for controlling data defined overrides in symbology, labeling
and composer.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgspropertyoverridebutton.h"
%End
  public:

    QgsPropertyOverrideButton( QWidget *parent /TransferThis/ = 0,
                               const QgsVectorLayer *layer = 0 );
%Docstring
Constructor for QgsPropertyOverrideButton.

:param parent: parent widget
:param layer: associated vector layer
%End


    void init( int propertyKey,
               const QgsProperty &property,
               const QgsPropertiesDefinition &definitions,
               const QgsVectorLayer *layer = 0,
               bool auxiliaryStorageEnabled = false );
%Docstring
Initialize a newly constructed property button (useful if button was included in a UI layout).

:param propertyKey: key for corresponding property
:param property: initial value of associated property to show in widget
:param definitions: properties definitions for corresponding collection
:param layer: associated vector layer
:param auxiliaryStorageEnabled: If true, activate the button to store data defined in auxiliary storage
%End

    void init( int propertyKey,
               const QgsProperty &property,
               const QgsPropertyDefinition &definition,
               const QgsVectorLayer *layer = 0,
               bool auxiliaryStorageEnabled = false );
%Docstring
Initialize a newly constructed property button (useful if button was included in a UI layout).

:param propertyKey: key for corresponding property
:param property: initial value of associated property to show in widget
:param definition: properties definition for button
:param layer: associated vector layer
:param auxiliaryStorageEnabled: If true, activate the button to store data defined in auxiliary storage
%End

    void init( int propertyKey,
               const QgsAbstractPropertyCollection &collection,
               const QgsPropertiesDefinition &definitions,
               const QgsVectorLayer *layer = 0,
               bool auxiliaryStorageEnabled = false );
%Docstring
Initialize a newly constructed property button (useful if button was included in a UI layout).

:param propertyKey: key for corresponding property
:param collection: associated property collection
:param definitions: properties definitions for collection
:param layer: associated vector layer
:param auxiliaryStorageEnabled: If true, activate the button to store data defined in auxiliary storage
%End

    QgsProperty toProperty() const;
%Docstring
Returns a QgsProperty object encapsulating the current state of the
widget.

.. seealso:: :py:func:`setToProperty`
%End

    void setToProperty( const QgsProperty &property );
%Docstring
Sets the widget to reflect the current state of a :py:class:`QgsProperty`.
%End

    int propertyKey() const;
%Docstring
Returns the property key linked to the button.
%End

    bool isActive() const;
%Docstring
Returns true if the button has an active property.
%End

    QgsPropertyDefinition::DataType validDataType() const;
%Docstring
Returns the data type which the widget will accept. This is used to filter
out fields from the associated vector layer to only show fields which
are compatible with the property.
%End

    QString fullDescription() const;
%Docstring
Returns the full definition description and current definition
(internally generated on a contextual basis).
%End

    QString usageInfo() const;
%Docstring
Returns usage information for the property.

.. seealso:: :py:func:`setUsageInfo`
%End

    void setUsageInfo( const QString &info );
%Docstring
Set the usage information for the property.

.. seealso:: :py:func:`usageInfo`
%End

    void setVectorLayer( const QgsVectorLayer *layer );
%Docstring
Sets the vector layer associated with the button. This controls which fields are
displayed within the widget's pop up menu.

.. seealso:: :py:func:`vectorLayer`
%End

    const QgsVectorLayer *vectorLayer() const;
%Docstring
Returns the vector layer associated with the button. This controls which fields are
displayed within the widget's pop up menu.

.. seealso:: :py:func:`setVectorLayer`
%End

    void registerCheckedWidget( QWidget *widget, bool natural = true );
%Docstring
Register a sibling ``widget`` that gets checked when the property is active.
if ``natural`` is false, widget gets unchecked when the property is active.

.. note::

   this should be called after calling init() to be correctly initialized.
%End

    void registerEnabledWidget( QWidget *widget, bool natural = true );
%Docstring
Register a sibling ``widget`` that gets enabled when the property is active, and disabled when the property is inactive.
if ``natural`` is false, widget gets disabled when the property is active, and enabled when the property is inactive.

.. note::

   this should be called after calling init() to be correctly initialized.
%End

    void registerVisibleWidget( QWidget *widget, bool natural = true );
%Docstring
Register a sibling ``widget`` that gets visible when the property is active, and hidden when the property is inactive.
if ``natural`` is false, widget gets hidden when the property is active, and visible when the property is inactive.

.. note::

   this should be called after calling init() to be correctly initialized.
%End

    void registerExpressionWidget( QWidget *widget );
%Docstring
Register a sibling ``widget`` (line edit, text edit) that will receive the property as an expression

.. note::

   this should be called after calling init() to be correctly initialized.
%End

    void registerExpressionContextGenerator( QgsExpressionContextGenerator *generator );
%Docstring
Register an expression context generator class that will be used to retrieve
an expression context for the button when required.
%End

    void updateFieldLists();
%Docstring
Updates list of fields.

.. versionadded:: 3.0
%End


  public slots:

    void setActive( bool active );
%Docstring
Set whether the current property override definition is to be used
%End

  signals:

    void changed();
%Docstring
Emitted when property definition changes
%End

    void activated( bool isActive );
%Docstring
Emitted when the activated status of the widget changes
%End

    void createAuxiliaryField();
%Docstring
Emitted when creating a new auxiliary field
%End

  protected:
    virtual void mouseReleaseEvent( QMouseEvent *event );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgspropertyoverridebutton.h                                  *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
