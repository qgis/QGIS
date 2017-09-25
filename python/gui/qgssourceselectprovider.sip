/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssourceselectprovider.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsSourceSelectProvider
{
%Docstring
 This is the interface for those who want to add entries to the QgsDataSourceManagerDialog

.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgssourceselectprovider.h"
%End
  public:

    enum Ordering
    {
      OrderLocalProvider,
      OrderDatabaseProvider,
      OrderRemoteProvider,
      OrderGeoCmsProvider,
      OrderOtherProvider,
    };

    virtual ~QgsSourceSelectProvider();

    virtual QString providerKey() const = 0;
%Docstring
Data Provider key
 :rtype: str
%End

    virtual QString name() const;
%Docstring
 Source select provider name, this is useful to retrieve
 a particular source select in case the provider has more
 than one, it should be unique among all providers.

 The default implementation returns the providerKey()
 :rtype: str
%End

    virtual QString text() const = 0;
%Docstring
Text for the menu item entry, it will be visible to the user so make sure it's translatable
 :rtype: str
%End

    virtual QString toolTip() const;
%Docstring
 Text for the tooltip menu item entry, it will be visible to the user so make sure it's translatable

 The default implementation returns an empty string.
 :rtype: str
%End

    virtual QIcon icon() const = 0;
%Docstring
Creates a new instance of an QIcon for the menu item entry
 :rtype: QIcon
%End

    virtual int ordering( ) const;
%Docstring
 Ordering: the source select provider registry will be able to sort
 the source selects (ascending) using this integer value
 :rtype: int
%End

    virtual QgsAbstractDataSourceWidget *createDataSourceWidget( QWidget *parent = 0, Qt::WindowFlags fl = Qt::Widget, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::Embedded ) const = 0 /Factory/;
%Docstring
 Create a new instance of QgsAbstractDataSourceWidget (or null).
 Caller takes responsibility of deleting created.
 :rtype: QgsAbstractDataSourceWidget
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssourceselectprovider.h                                    *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
