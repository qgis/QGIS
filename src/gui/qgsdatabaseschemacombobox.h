/***************************************************************************
   qgsdatabaseschemacombobox.h
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

#ifndef QGSDATABASESCHEMACOMBOBOX_H
#define QGSDATABASESCHEMACOMBOBOX_H

#include <QComboBox>

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QSortFilterProxyModel>

class QgsDatabaseSchemaModel;
class QgsAbstractDatabaseProviderConnection;

///@cond PRIVATE
#ifndef SIP_RUN
class GUI_EXPORT QgsDatabaseSchemaComboBoxSortModel: public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    explicit QgsDatabaseSchemaComboBoxSortModel( QObject *parent = nullptr );
  protected:
    bool lessThan( const QModelIndex &source_left, const QModelIndex &source_right ) const override;

};
#endif
///@endcond

/**
 * \ingroup gui
 * \brief The QgsDatabaseSchemaComboBox class is a combo box which displays the list of schemas for a specific database connection.
 *
 * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
 * in order for the combobox to work correctly.
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsDatabaseSchemaComboBox : public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsDatabaseSchemaComboBox, for the specified \a provider and \a connection.
     *
    * \warning The provider must support the connection API methods in its QgsProviderMetadata implementation
    * in order for the model to work correctly.
     */
    explicit QgsDatabaseSchemaComboBox( const QString &provider, const QString &connection, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Constructor for QgsDatabaseSchemaComboBox, for the specified \a connection.
     *
     * Ownership of \a connection is transferred to the combobox.
     */
    explicit QgsDatabaseSchemaComboBox( QgsAbstractDatabaseProviderConnection *connection SIP_TRANSFER, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets whether an optional empty schema ("not set") option is present in the combobox.
     * \see allowEmptySchema()
     */
    void setAllowEmptySchema( bool allowEmpty );

    /**
     * Returns TRUE if the combobox allows the empty schema ("not set") choice.
     * \see setAllowEmptySchema()
     */
    bool allowEmptySchema() const;

    /**
     * Returns the name of the current schema selected in the combo box.
     */
    QString currentSchema() const;

    /**
     * Returns the combobox portion of the widget.
     */
    QComboBox *comboBox() { return mComboBox; }

  public slots:

    /**
     * Sets the current schema selected in the combo box.
     */
    void setSchema( const QString &schema );

    /**
     * Sets the database connection name from which to retrieve the available schemas.
     *
     * Optionally the \a provider can be reset too.
     */
    void setConnectionName( const QString &connection, const QString &provider = QString() );

    /**
     * Refreshes the list of available schemas.
     */
    void refreshSchemas();

  signals:
    //! Emitted whenever the currently selected schema changes.
    void schemaChanged( const QString &schema );

  private slots:
    void indexChanged( int i );
    void rowsChanged();

  private:
    void init();

    bool mAllowEmpty = false;
    QString mProvider;
    QgsDatabaseSchemaModel *mModel = nullptr;
    QSortFilterProxyModel *mSortModel = nullptr;
    QComboBox *mComboBox = nullptr;
};

#endif // QGSDATABASESCHEMACOMBOBOX_H
