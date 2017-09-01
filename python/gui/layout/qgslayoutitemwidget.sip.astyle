/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutitemwidget.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsLayoutConfigObject: QObject
{
%Docstring

 An object for property widgets for layout items. All layout config type widgets should contain
 this object.

 If you are creating a new QgsLayoutItem configuration widget, you should instead
 inherit from QgsLayoutItemBaseWidget (rather then directly working with QgsLayoutConfigObject).

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemwidget.h"
%End
  public:

    QgsLayoutConfigObject( QWidget *parent /TransferThis/, QgsLayoutObject *layoutObject );
%Docstring
 Constructor for QgsLayoutConfigObject, linked with the specified ``layoutObject``.
%End

    void initializeDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty key );
%Docstring
 Registers a data defined ``button``, setting up its initial value, connections and description.
 The corresponding property ``key`` must be specified.
%End

    void updateDataDefinedButton( QgsPropertyOverrideButton *button );
%Docstring
 Updates a data defined button to reflect the item's current properties.
%End

    QgsVectorLayer *coverageLayer() const;
%Docstring
 Returns the current layout context coverage layer (if set).
 :rtype: QgsVectorLayer
%End


};

class QgsLayoutItemBaseWidget: QgsPanelWidget
{
%Docstring

 A base class for property widgets for layout items. All layout item widgets should inherit from
 this base class.


.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemwidget.h"
%End
  public:

    QgsLayoutItemBaseWidget( QWidget *parent /TransferThis/, QgsLayoutObject *layoutObject );
%Docstring
 Constructor for QgsLayoutItemBaseWidget, linked with the specified ``layoutObject``.
%End

  protected:

    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsLayoutObject::DataDefinedProperty property );
%Docstring
 Registers a data defined ``button``, setting up its initial value, connections and description.
 The corresponding property ``key`` must be specified.
%End

    void updateDataDefinedButton( QgsPropertyOverrideButton *button );
%Docstring
 Updates a previously registered data defined button to reflect the item's current properties.
%End

    QgsVectorLayer *coverageLayer() const;
%Docstring
 Returns the current layout context coverage layer (if set).
 :rtype: QgsVectorLayer
%End


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutitemwidget.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
