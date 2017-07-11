/***************************************************************************
    qgssourceselect.h  -  base class for source selector widgets
                             -------------------
    begin                : 10 July 2017
    original             : (C) 2017 by Alessandro Pasotti email  : apasotti at boundlessgeo dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSOURCESELECT_H
#define QGSSOURCESELECT_H
#include "qgis_sip.h"
#include "qgis.h"
#include "qgis_gui.h"

#include "qgsproviderregistry.h"
#include "qgsguiutils.h"

#include <QDialog>

/** \ingroup gui
 * \brief  Abstract base Dialog to create connections and add layers
 * This class must provide common functionality and the interface for all
 * source select dialogs used by data providers to configure data sources
 * and add layers.
 */
class GUI_EXPORT QgsSourceSelect : public QDialog
{
    Q_OBJECT

  public:

    //! Constructor
    QgsSourceSelect( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

    //! Destructor
    ~QgsSourceSelect( );

    //! Return the widget mode
    QgsProviderRegistry::WidgetMode widgetMode( ) { return mWidgetMode; }

  public slots:

    //! Triggered when the provider's connections need to be refreshed
    virtual void refresh( ) = 0;

  signals:

    //! Emitted when the provider's connections have changed
    void connectionsChanged();

  private:

    QgsProviderRegistry::WidgetMode mWidgetMode;
};

#endif // QGSSOURCESELECT_H
