/***************************************************************************
   qgsdatabasetablecombobox.h
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

#ifndef QGSDATABASETABLECOMBOBOX_H
#define QGSDATABASETABLECOMBOBOX_H

#include <QComboBox>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QSortFilterProxyModel>

class QgsDatabaseTableModel;
class QgsAbstractDatabaseProviderConnection;

///@cond PRIVATE
#ifndef SIP_RUN
class GUI_EXPORT QgsDatabaseTableComboBoxSortModel: public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    explicit QgsDatabaseTableComboBoxSortModel( QObject *parent = nullptr );
  protected:
    bool lessThan( const QModelIndex &source_left, const QModelIndex &source_right ) const override;

};
#endif
///@endcond

/**
 * \ingroup gui
 * \brief The QgsDatabaseTableComboBox class is a combo box which displays the list of tables for a specific database connection.
 *
 * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
 * in order for the combobox to work correctly.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsDatabaseTableComboBox : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDatabaseTableComboBox, for the specified \a provider and \a connection.
     *
     * The optional \a schema argument can be used to restrict the listed tables to a specific schema.
     *
    * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
    * in order for the model to work correctly.
     */
    explicit QgsDatabaseTableComboBox( const QString &provider, const QString &connection, const QString &schema = QString(), QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsDatabaseTableComboBox, for the specified \a connection.
     *
     * The optional \a schema argument can be used to restrict the listed tables to a specific schema.
     *
     * Ownership of \a connection is transferred to the combobox.
     */
    explicit QgsDatabaseTableComboBox( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER, const QString &schema = QString(), QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets whether an optional empty table ("not set") option is present in the combobox.
     * \see allowEmptyTable()
     */
    void setAllowEmptyTable( bool allowEmpty );

    /**
     * Returns TRUE if the combobox allows the empty table ("not set") choice.
     * \see setAllowEmptyTable()
     */
    bool allowEmptyTable() const;

    /**
     * Returns the name of the current table selected in the combo box.
     */
    QString currentTable() const;

    /**
     * Returns the schema of the current table selected in the combo box.
     */
    QString currentSchema() const;

    /**
     * Returns the combobox portion of the widget.
     */
    QComboBox *comboBox() { return mComboBox; }

  public slots:

    /**
     * Sets the current table selected in the combo box.
     *
     * If necessary, the \a schema can be specified too.
     */
    void setTable( const QString &table, const QString &schema = QString() );

    /**
     * Sets the database connection name from which to retrieve the available tables.
     *
     * Optionally the \a provider can be reset too.
     */
    void setConnectionName( const QString &connection, const QString &provider = QString() );

    /**
     * Sets the \a schema from which to retrieve the available tables.
     */
    void setSchema( const QString &schema );

    /**
     * Refreshes the list of available tables.
     */
    void refreshTables();

  signals:
    //! Emitted whenever the currently selected table changes.
    void tableChanged( const QString &table, const QString &schema = QString() );

  private slots:
    void indexChanged( int i );
    void rowsChanged();

  private:
    void init();

    bool mAllowEmpty = false;
    QString mProvider;
    QString mConnection;
    QString mSchema;
    QgsDatabaseTableModel *mModel = nullptr;
    QSortFilterProxyModel *mSortModel = nullptr;
    QComboBox *mComboBox = nullptr;
};

#endif // QGSDATABASETABLECOMBOBOX_H
