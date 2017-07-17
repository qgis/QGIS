/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsabstractdatasourcewidget.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsAbstractDataSourceWidget : QDialog
{
%Docstring
  Abstract base Data Source Widget to create connections and add layers
 This class must provide common functionality and the interface for all
 source select dialogs used by data providers to configure data sources
 and add layers.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgsabstractdatasourcewidget.h"
%End
  public:

    QgsAbstractDataSourceWidget( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
%Docstring
Constructor
%End

    virtual ~QgsAbstractDataSourceWidget( ) = 0;
%Docstring
Pure Virtual Destructor
%End

    QgsProviderRegistry::WidgetMode widgetMode( );
%Docstring
Return the widget mode
 :rtype: QgsProviderRegistry.WidgetMode
%End

  public slots:

    virtual void refresh( );
%Docstring
The default implementation does nothing
%End

  signals:

    void connectionsChanged();
%Docstring
Emitted when the provider's connections have changed
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsabstractdatasourcewidget.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
