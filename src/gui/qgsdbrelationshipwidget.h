/***************************************************************************
    qgsdbrelationshipwidget.h
    ------------------
    Date                 : November 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDBRELATIONSHIPWIDGET_H
#define QGSDBRELATIONSHIPWIDGET_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include "ui_qgsdbrelationshipwidgetbase.h"
#include "qgis.h"
#include "qgsweakrelation.h"
#include <QAbstractTableModel>
#include <QDialog>

class QDialogButtonBox;
class QgsAbstractDatabaseProviderConnection;
class QgsDatabaseTableModel;
class QSortFilterProxyModel;

/**
 * \ingroup gui
 * \brief A widget for configuration of the properties of a relationship in a database.
 *
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsDbRelationWidget : public QWidget, private Ui_QgsDbRelationshipWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsDbRelationWidget with the specified \a parent widget, for the specified \a connection.
     *
     * Ownership of \a connection is transferred to the widget.
     */
    QgsDbRelationWidget( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the current \a relationship to show properties for in the widget.
     *
     * \see relationship()
     */
    void setRelationship( const QgsWeakRelation &relationship );

    /**
     * Returns the relationship as defined in the widget.
     *
     * \see setRelationship()
     */
    QgsWeakRelation relationship() const;

    /**
     * Returns TRUE if the widget currently represents a valid relationship configuration.
     *
     * \see validityChanged()
     */
    bool isValid() const;

  signals:

    /**
     * Emitted whenever the validity of the relationship configuration in the widget changes.
     *
     * \see isValid()
     */
    void validityChanged( bool isValid );

  private:
    QgsAbstractDatabaseProviderConnection *mConnection = nullptr;
    QgsDatabaseTableModel *mTableModel = nullptr;
    QSortFilterProxyModel *mProxyModel = nullptr;
    QgsWeakRelation mRelation;
};


/**
 * \ingroup gui
 * \brief A dialog for configuration of the properties of a relationship in a database.
 *
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsDbRelationDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsDbRelationDialog, with the specified \a parent widget and window \a flags, for the specified \a connection.
     *
     * Ownership of \a connection is transferred to the widget.
     */
    explicit QgsDbRelationDialog( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

    /**
     * Sets the current \a relationship to show properties for in the dialog.
     *
     * \see relationship()
     */
    void setRelationship( const QgsWeakRelation &relationship );

    /**
     * Returns the relationship as defined in the dialog.
     *
     * \see setRelationship()
     */
    QgsWeakRelation relationship() const;

  public slots:

    void accept() override;

  private slots:

    void validityChanged( bool isValid );

  private:
    QgsDbRelationWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif // QGSDBRELATIONSHIPWIDGET_H
