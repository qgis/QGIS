/***************************************************************************
  qgsnewdatabasetablenamewidget.h - QgsNewDatabaseTableNameWidget

 ---------------------
 begin                : 9.3.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSNEWDATABASETABLENAMEWIDGET_H
#define QGSNEWDATABASETABLENAMEWIDGET_H


#include "ui_qgsnewdatabasetablenamewidget.h"

#include "qgis_gui.h"
#include "qgsbrowserguimodel.h"
#include "qgsbrowserproxymodel.h"
#include "qgspanelwidget.h"

#include <QWidget>
#include <QDialog>

/**
 * \ingroup gui
 * \brief The QgsNewDatabaseTableNameWidget class embeds the browser view to
 * select a DB schema and a new table name.
 *
 * The table name is validated for uniqueness and the selected
 * data item provider, schema and table names can be retrieved with
 * getters.
 *
 * \warning The data provider that originated the data item provider
 *          must support the connections API
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsNewDatabaseTableNameWidget : public QgsPanelWidget, private Ui::QgsNewDatabaseTableNameWidget
{
    Q_OBJECT

  public:
    /**
     * Constructs a new QgsNewDatabaseTableNameWidget
     *
     * \param browserModel an existing browser model (typically from app), if NULLPTR an instance will be created
     * \param providersFilter optional white list of data provider keys that should be
     *        shown in the widget, if not specified all providers data items with database
     *        capabilities will be shown
     * \param parent optional parent for this widget
     */
    explicit QgsNewDatabaseTableNameWidget( QgsBrowserGuiModel *browserModel = nullptr, const QStringList &providersFilter = QStringList(), QWidget *parent = nullptr );

    /**
     * Sets whether the optional "Ok"/accept button should be visible.
     *
     * By default this is hidden, to better allow the widget to be embedded inside other widgets and dialogs.
     */
    void setAcceptButtonVisible( bool visible );

    /**
     * Returns the currently selected schema or file path (in case of filesystem-based DBs like spatialite or GPKG) for the new table
     */
    QString schema() const;

    /**
     * Returns the (possibly blank) string representation of the new table data source URI.
     * The URI might be invalid in case the widget is not in a valid state.
     */
    QString uri() const;

    /**
     * Returns the current name of the new table
     */
    QString table() const;

    /**
     * Returns the currently selected data item provider key
     */
    QString dataProviderKey() const;

    /**
     * Returns TRUE if the widget contains a valid new table name
     */
    bool isValid() const;

    /**
     * Returns the validation error or an empty string is the widget status is valid
     */
    QString validationError() const;

    //! Scroll to last selected index and expand it's children
    void showEvent( QShowEvent *e ) override;

  signals:

    /**
     * This signal is emitted whenever the validation status of the widget changes.
     *
     * \param isValid TRUE if the current status of the widget is valid
     */
    void validationChanged( bool isValid );

    /**
      * This signal is emitted when the user selects a schema (or file path for filesystem-based DBs like spatialite or GPKG).
      *
      * \param schemaName the name of the selected schema
      */
    void schemaNameChanged( const QString &schemaName );

    /**
      * This signal is emitted when the user enters a table name
      *
      * \param tableName the name of the new table
      */
    void tableNameChanged( const QString &tableName );

    /**
      * This signal is emitted when the selects a data provider or a schema name
      * that has a different data provider than the previously selected one.
      *
      * \param providerKey the data provider key of the selected schema
      */
    void providerKeyChanged( const QString &providerKey );

    /**
     * This signal is emitted when the URI of the new table changes, whether or not it is a valid one.
     *
     * \param uri URI string representation
     */
    void uriChanged( const QString &uri );

    /**
     * Emitted when the OK/accept button is clicked.
     */
    void accepted();

  private:
    void updateUri();
    void validate();
    QStringList tableNames();
    void refreshModel( const QModelIndex &index );

    QgsBrowserProxyModel mBrowserProxyModel;
    QgsBrowserGuiModel *mBrowserModel = nullptr;
    QString mDataProviderKey;
    QString mTableName;
    QString mSchemaName;
    QString mConnectionName;
    bool mIsFilePath = false;
    QString mUri;
    //! List of data provider keys of shown providers
    QSet<QString> mShownProviders;
    bool mIsValid = false;
    QString mValidationError;
    //! Table names cache
    QMap<QString, QStringList> mTableNamesCache;

    static QStringList FILESYSTEM_BASED_DATAITEM_PROVIDERS;

    // For testing:
    friend class TestQgsNewDatabaseTableNameWidget;
};


/**
 * \ingroup gui
 * \brief QgsNewDatabaseTableNameDialog is a dialog which allows selection of a DB schema and a new table name.
 *
 * The table name is validated for uniqueness and the selected
 * data item provider, schema and table names can be retrieved with
 * getters.
 *
 * \warning The data provider that originated the data item provider
 *          must support the connections API
 *
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsNewDatabaseTableNameDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructs a new QgsNewDatabaseTableNameDialog
     *
     * \param browserModel an existing browser model (typically from app), if NULLPTR an instance will be created
     * \param providersFilter optional white list of data provider keys that should be
     *        shown in the widget, if not specified all providers data items with database
     *        capabilities will be shown
     * \param parent optional parent for this widget
     */
    explicit QgsNewDatabaseTableNameDialog( QgsBrowserGuiModel *browserModel = nullptr, const QStringList &providersFilter = QStringList(), QWidget *parent = nullptr );

    /**
     * Returns the currently selected schema or file path (in case of filesystem-based DBs like spatialite or GPKG) for the new table
     */
    QString schema() const;

    /**
     * Returns the (possibly blank) string representation of the new table data source URI.
     * The URI might be invalid in case the widget is not in a valid state.
     */
    QString uri() const;

    /**
     * Returns the current name of the new table
     */
    QString table() const;

    /**
     * Returns the currently selected data item provider key
     */
    QString dataProviderKey() const;

    /**
     * Returns TRUE if the widget contains a valid new table name
     */
    bool isValid() const;

    /**
     * Returns the validation error or an empty string is the widget status is valid
     */
    QString validationError() const;

  private:
    QgsNewDatabaseTableNameWidget *mWidget = nullptr;
};
#endif // QGSNEWDATABASETABLENAMEWIDGET_H
