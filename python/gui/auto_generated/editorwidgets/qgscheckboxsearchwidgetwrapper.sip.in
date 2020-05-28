/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgscheckboxsearchwidgetwrapper.h               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsCheckboxSearchWidgetWrapper : QgsSearchWidgetWrapper
{
%Docstring
Wraps a checkbox edit widget for searching.

.. versionadded:: 2.16
%End

%TypeHeaderCode
#include "qgscheckboxsearchwidgetwrapper.h"
%End
  public:

    explicit QgsCheckboxSearchWidgetWrapper( QgsVectorLayer *vl, int fieldIdx, QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsCheckboxSearchWidgetWrapper.

:param vl: associated vector layer
:param fieldIdx: index of associated field
:param parent: parent widget
%End

    QVariant value() const;
%Docstring
Returns a variant representing the current state of the widget.

.. note::

   this will not be a boolean ``True`` or ``False`` value, it will instead
   be the values configured to represent checked and unchecked states in
   the editor widget configuration.
%End

    virtual bool applyDirectly();

    virtual QString expression() const;

    virtual bool valid() const;

    virtual QgsSearchWidgetWrapper::FilterFlags supportedFlags() const;

    virtual QgsSearchWidgetWrapper::FilterFlags defaultFlags() const;

    virtual QString createExpression( QgsSearchWidgetWrapper::FilterFlags flags ) const;


  public slots:

    virtual void clearWidget();

    virtual void setEnabled( bool enabled );


  protected:
    virtual QWidget *createWidget( QWidget *parent );

    virtual void initWidget( QWidget *editor );


  protected slots:
    virtual void setExpression( const QString &expression );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/editorwidgets/qgscheckboxsearchwidgetwrapper.h               *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
