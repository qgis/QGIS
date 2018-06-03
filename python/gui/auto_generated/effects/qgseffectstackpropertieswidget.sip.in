/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/effects/qgseffectstackpropertieswidget.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsEffectStackPropertiesWidget : QgsPanelWidget
{
%Docstring
A widget for modifying the properties of a QgsEffectStack, including adding
and reordering effects within the stack.

.. seealso:: :py:class:`QgsEffectStack`

.. seealso:: :py:class:`QgsEffectStackPropertiesDialog`

.. seealso:: :py:class:`QgsEffectStackCompactWidget`

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgseffectstackpropertieswidget.h"
%End
  public:

    QgsEffectStackPropertiesWidget( QgsEffectStack *stack, QWidget *parent /TransferThis/ = 0 );
%Docstring
QgsEffectStackPropertiesWidget constructor

:param stack: QgsEffectStack to modify in the widget
:param parent: parent widget
%End

    ~QgsEffectStackPropertiesWidget();

    QgsEffectStack *stack();
%Docstring
Returns effect stack attached to the widget

:return: QgsEffectStack modified by the widget
%End

    void setPreviewPicture( const QPicture &picture );
%Docstring
Sets the picture to use for effect previews for the dialog

:param picture: preview picture
%End

  public slots:

    void moveEffectDown();
%Docstring
Moves the currently selected effect down in the stack.
%End

    void moveEffectUp();
%Docstring
Moves the currently selected effect up in the stack.
%End

    void addEffect();
%Docstring
Adds a new effect to the stack.
%End

    void removeEffect();
%Docstring
Removes the currently selected effect from the stack.
%End

    void effectChanged();
%Docstring
Updates the widget when the selected effect changes type.
%End

    void updatePreview();
%Docstring
Updates the effect preview icon.
%End

    void changeEffect( QgsPaintEffect *newEffect );
%Docstring
Updates the effect stack when the currently selected effect changes properties.

:param newEffect: new effect to replace existing effect at selected position within the stack.
%End

  protected:


    void loadStack();
%Docstring
Refreshes the widget to reflect the current state of the stack.
%End

    void loadStack( QgsEffectStack *stack );
%Docstring
Refreshes the widget to reflect the current state of a specified stack.

:param stack: QgsEffectStack for widget
%End

    void updateUi();
%Docstring
Enables or disables widgets depending on the selected effect within the stack.
%End


    void moveEffectByOffset( int offset );
%Docstring
Moves the currently selected effect within the stack by a specified offset
%End

    void setWidget( QWidget *widget );
%Docstring
Sets the effect properties widget
%End

};



class QgsEffectStackPropertiesDialog: QgsDialog
{
%Docstring
A dialog for modifying the properties of a :py:class:`QgsEffectStack`, including adding
and reordering effects within the stack.

.. seealso:: :py:class:`QgsEffectStack`

.. seealso:: :py:class:`QgsEffectStackPropertiesWidget`

.. seealso:: :py:class:`QgsEffectStackCompactWidget`

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgseffectstackpropertieswidget.h"
%End
  public:

    QgsEffectStackPropertiesDialog( QgsEffectStack *stack, QWidget *parent /TransferThis/ = 0, Qt::WindowFlags f = 0 );
%Docstring
QgsEffectStackPropertiesDialog constructor

:param stack: QgsEffectStack to modify in the dialog
:param parent: parent widget
:param f: window flags
%End

    QgsEffectStack *stack();
%Docstring
Returns effect stack attached to the dialog

:return: QgsEffectStack modified by the dialog
%End

    void setPreviewPicture( const QPicture &picture );
%Docstring
Sets the picture to use for effect previews for the dialog

:param picture: preview picture
%End

  protected:


};



class QgsEffectStackCompactWidget: QgsPanelWidget
{
%Docstring
A small widget consisting of a checkbox for enabling/disabling an effect stack
and a button for opening an effect stack customization dialog.

.. seealso:: :py:class:`QgsEffectStack`

.. seealso:: :py:class:`QgsEffectStackPropertiesWidget`

.. seealso:: :py:class:`QgsEffectStackPropertiesDialog`

.. versionadded:: 2.9
%End

%TypeHeaderCode
#include "qgseffectstackpropertieswidget.h"
%End
  public:

    QgsEffectStackCompactWidget( QWidget *parent /TransferThis/ = 0, QgsPaintEffect *effect = 0 );
%Docstring
QgsEffectStackCompactWidget constructor

:param parent: parent widget
:param effect: QgsPaintEffect for modification by the widget. If the effect
               is not a :py:class:`QgsEffectStack`, it will be automatically converted to an effect
               stack consisting of the original effect
%End
    ~QgsEffectStackCompactWidget();

    void setPaintEffect( QgsPaintEffect *effect );
%Docstring
Sets paint effect attached to the widget,

:param effect: QgsPaintEffect for modification by the widget. If the effect
               is not a :py:class:`QgsEffectStack`, it will be automatically converted to an effect
               stack consisting of the original effect

.. seealso:: :py:func:`paintEffect`
%End

    QgsPaintEffect *paintEffect() const;
%Docstring
Returns paint effect attached to the widget

:return: QgsPaintEffect modified by the widget

.. seealso:: :py:func:`setPaintEffect`
%End

    void setPreviewPicture( const QPicture &picture );
%Docstring
Sets the picture to use for effect previews for the dialog

:param picture: preview picture
%End

  signals:

    void changed();
%Docstring
Emitted when the paint effect properties change
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/effects/qgseffectstackpropertieswidget.h                     *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
