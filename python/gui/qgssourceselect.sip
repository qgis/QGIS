/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgssourceselect.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsSourceSelect : QDialog
{
%Docstring
  Abstract base Dialog to create connections and add layers
 This class must provide common functionality and the interface for all
 source select dialogs used by data providers to configure data sources
 and add layers.
%End

%TypeHeaderCode
#include "qgssourceselect.h"
%End
  public:

    QgsSourceSelect( QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );
%Docstring
Constructor
%End

    ~QgsSourceSelect( );
%Docstring
Destructor
%End

    QgsProviderRegistry::WidgetMode widgetMode( );
%Docstring
Return the widget mode
 :rtype: QgsProviderRegistry.WidgetMode
%End

  public slots:

    virtual void refresh( ) = 0;
%Docstring
Triggered when the provider's connections need to be refreshed
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
 * src/gui/qgssourceselect.h                                            *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
