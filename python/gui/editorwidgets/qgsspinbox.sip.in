/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsspinbox.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





%ModuleHeaderCode
// fix to allow compilation with sip 4.7 that for some reason
// doesn't add this include to the file where the code from
// ConvertToSubClassCode goes.
#include <qgsspinbox.h>
%End


class QgsSpinBox : QSpinBox
{
%Docstring
 The QgsSpinBox is a spin box with a clear button that will set the value to the defined clear value.
The clear value can be either the minimum or the maiximum value of the spin box or a custom value.
This value can then be handled by a special value text.
%End

%TypeHeaderCode
#include "qgsspinbox.h"
%End
%ConvertToSubClassCode
    if ( qobject_cast<QgsSpinBox *>( sipCpp ) )
      sipType = sipType_QgsSpinBox;
    else
      sipType = NULL;
%End
  public:

    enum ClearValueMode
    {
      MinimumValue,
      MaximumValue,
      CustomValue,
    };

    explicit QgsSpinBox( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsSpinBox.

:param parent: parent widget
%End

    void setShowClearButton( const bool showClearButton );
%Docstring
Sets whether the widget will show a clear button. The clear button
allows users to reset the widget to a default or empty state.

:param showClearButton: set to true to show the clear button, or false to hide it

.. seealso:: :py:func:`showClearButton`
%End

    bool showClearButton() const;
%Docstring
Returns whether the widget is showing a clear button.

.. seealso:: :py:func:`setShowClearButton`
%End

    void setExpressionsEnabled( const bool enabled );
%Docstring
Sets if the widget will allow entry of simple expressions, which are
evaluated and then discarded.

:param enabled: set to true to allow expression entry

.. versionadded:: 2.7
%End

    bool expressionsEnabled() const;
%Docstring
Returns whether the widget will allow entry of simple expressions, which are
evaluated and then discarded.

:return: true if spin box allows expression entry

.. versionadded:: 2.7
%End

    virtual void clear();

%Docstring
Set the current value to the value defined by the clear value.
%End

    void setClearValue( int customValue, const QString &clearValueText = QString() );
%Docstring
Defines the clear value as a custom value and will automatically set the clear value mode to CustomValue.

:param customValue: defines the numerical value used as the clear value
:param clearValueText: is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.

.. seealso:: :py:func:`setClearValue`
%End

    void setClearValueMode( ClearValueMode mode, const QString &clearValueText = QString() );
%Docstring
Defines if the clear value should be the minimum or maximum values of the widget or a custom value.

:param mode: mode to user for clear value
:param clearValueText: is the text displayed when the spin box is at the clear value. If not specified, no special value text is used.
%End

    int clearValue() const;
%Docstring
Returns the value used when clear() is called.

.. seealso:: :py:func:`setClearValue`
%End

    virtual int valueFromText( const QString &text ) const;

    virtual QValidator::State validate( QString &input, int &pos ) const;


  protected:

    virtual void changeEvent( QEvent *event );

    virtual void paintEvent( QPaintEvent *event );

    virtual void wheelEvent( QWheelEvent *event );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgsspinbox.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
