/***************************************************************************
   qgsproviderconnectioncombobox.h
    --------------------------------
   Date                 : March 2020
   Copyright            : (C) 2020 Nyall Dawson
   Email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSPROVIDERCONNECTIONCOMBOBOX_H
#define QGSPROVIDERCONNECTIONCOMBOBOX_H

#include <QComboBox>

#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsProviderConnectionModel;
class QSortFilterProxyModel;

/**
 * \ingroup gui
 * \brief The QgsProviderConnectionComboBox class is a combo box which displays the list of connections registered for a given provider.
 *
 * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
 * in order for the model to work correctly.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsProviderConnectionComboBox : public QComboBox
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProviderConnectionComboBox, for the specified \a provider.
     *
    * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
    * in order for the model to work correctly.
     */
    explicit QgsProviderConnectionComboBox( const QString &provider, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the name of the current connection selected in the combo box.
     */
    QString currentConnection() const;

    /**
     * Returns the uri of the current connection selected in the combo box.
     */
    QString currentConnectionUri() const;

  public slots:

    /**
     * Sets the current connection selected in the combo box.
     */
    void setConnection( const QString &connection );

  signals:
    //! Emitted whenever the currently selected connection changes.
    void connectionChanged( const QString &connection );

  private slots:
    void indexChanged( int i );
    void rowsChanged();

  private:
    QgsProviderConnectionModel *mModel = nullptr;
    QSortFilterProxyModel *mSortModel = nullptr;
};

#endif // QGSPROVIDERCONNECTIONCOMBOBOX_H
