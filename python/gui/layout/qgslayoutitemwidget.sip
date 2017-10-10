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


class QgsLayoutItemPropertiesWidget: QWidget
{
%Docstring
 A widget for controlling the common properties of layout items (e.g. position and size, background, stroke, frame).
 This widget can be embedded into other layout item widgets.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemwidget.h"
%End
  public:
    QgsLayoutItemPropertiesWidget( QWidget *parent, QgsLayoutItem *item );

    QgsLayoutItem::ReferencePoint positionMode() const;
%Docstring
 :rtype: QgsLayoutItem.ReferencePoint
%End

    void showBackgroundGroup( bool showGroup );

    void showFrameGroup( bool showGroup );

  protected slots:
    void initializeDataDefinedButtons();
%Docstring
Initializes data defined buttons to current atlas coverage layer
%End
    void populateDataDefinedButtons();
%Docstring
Sets data defined button state to match item
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/layout/qgslayoutitemwidget.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
