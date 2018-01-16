/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsoptionswidgetfactory.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsOptionsPageWidget : QWidget
{
%Docstring
Base class for widgets for pages included in the options dialog.

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsoptionswidgetfactory.h"
%End
  public:

    QgsOptionsPageWidget( QWidget *parent /TransferThis/ = 0 );
%Docstring
Constructor for QgsOptionsPageWidget.
%End

    virtual QString helpKey() const;
%Docstring
Returns the optional help key for the options page. The default implementation
returns an empty string.

If a non-empty string is returned by this method, it will be used as the help key
retrieved when the "help" button is clicked while this options page is active.

If an empty string is returned by this method the default QGIS options
help will be retrieved.
%End

  public slots:

    virtual void apply() = 0;
%Docstring
Called to permanently apply the settings shown in the options page (e.g. save them to
QgsSettings objects). This is usually called when the options dialog is accepted.
%End

};

class QgsOptionsWidgetFactory : QObject
{

%TypeHeaderCode
#include "qgsoptionswidgetfactory.h"
%End
  public:

    QgsOptionsWidgetFactory();
%Docstring
Constructor
%End

    QgsOptionsWidgetFactory( const QString &title, const QIcon &icon );
%Docstring
Constructor
%End

    virtual QIcon icon() const;
%Docstring
 The icon that will be shown in the UI for the panel.

:return: A QIcon for the panel icon.

.. seealso:: :py:func:`setIcon`
%End

    void setIcon( const QIcon &icon );
%Docstring
Set the ``icon`` to show in the interface for the factory object.

.. seealso:: :py:func:`icon`
%End

    virtual QString title() const;
%Docstring
The title of the panel.

.. seealso:: :py:func:`setTitle`
%End

    void setTitle( const QString &title );
%Docstring
Set the ``title`` for the interface.

.. seealso:: :py:func:`title`
%End

    virtual QgsOptionsPageWidget *createWidget( QWidget *parent = 0 ) const = 0 /Factory/;
%Docstring
 Factory function to create the widget on demand as needed by the options dialog.

:param parent: The parent of the widget.

:return: A new widget to show as a page in the options dialog.
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsoptionswidgetfactory.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
