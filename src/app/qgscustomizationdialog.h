/***************************************************************************
    qgscustomizationdialog.h
    ---------------------
    begin                : 2025/12/16
    copyright            : (C) 2025 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCUSTOMIZATIONDIALOG_H
#define QGSCUSTOMIZATIONDIALOG_H

#include "ui_qgscustomizationdialogbase.h"

#include "qgis_app.h"
#include "qgscustomization.h"
#include "qgssettingstree.h"

class QgisApp;

/**
 * \ingroup app
 * \brief Dialog to customize application
 * \since QGIS 4.0
*/
class APP_EXPORT QgsCustomizationDialog : public QMainWindow, private Ui::QgsCustomizationDialogBase
{
    Q_OBJECT
  public:
    /**
     * Constructor
     * \param customization object to customize the application
     * \param parent parent widget
     */
    QgsCustomizationDialog( QgisApp *qgisApp );

    static inline QgsSettingsTreeNode *sTreeCustomization = QgsSettingsTree::sTreeApp->createChildNode( u"customization"_s );
    static const QgsSettingsEntryString *sSettingLastSaveDir;

  private slots:

    /**
     * Called whenever user clicks the OK button
     */
    void ok();

    /**
     * Called whenever user clicks the Apply button
     */
    void apply();

    /**
     * Called whenever user clicks the Cancel button
     */
    void cancel();

    /**
     * Called whenever user clicks the Help button
     */
    void showHelp();

    /**
     * Called by QgsApplication::notify() method so we can preempt all the mouse clicks
     * when the "catch" mode is enabled
     * see onActionCatchToggled()
     */
    void preNotify( QObject *receiver, QEvent *event, bool *done );

    /**
     * Enable "catch" mode, allows clicking on the application UI to select action
     */
    void onActionCatchToggled( bool toggled );

    /**
     * Cancel all current, non applied, modification
     */
    void reset();

    /**
     * Save customization to a file
     */
    void onSaveFile( bool checked );

    /**
     * Load customization to a file
     */
    void onLoadFile( bool checked );

    /**
     * Expand all tree view items
     */
    void onExpandAll( bool checked );

    /**
     * Collapse all tree view items
     */
    void onCollapseAll( bool checked );

    /**
     * Select all tree view items
     */
    void onSelectAll( bool checked );

    /**
     * Enable or disable current customization according to \a cheched boolean
     */
    void enableCustomization( bool checked );

  private:
    /**
     * find QAction associated to \a toolbutton
     */
    QAction *findAction( QToolButton *toolbutton );

    /**
     * Select item associated to \a widget in tree view
     */
    bool selectWidget( QWidget *widget );

    const std::unique_ptr<QgsCustomization> &customization() const;

    /**
     * \ingroup app
     * \brief tree view to edit a customization
     * \since QGIS 4.0
     */
    class QgsCustomizationModel : public QAbstractItemModel
    {
      public:
        enum class Mode
        {
          ActionSelector, //!< Use to select actions
          ItemVisibility  //!< Use to change item visibility
        };

        QgsCustomizationModel( QgisApp *qgisApp, Mode mode, QObject *parent = nullptr );

        QVariant data( const QModelIndex &index, int role ) const override;
        bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
        Qt::ItemFlags flags( const QModelIndex &index ) const override;
        QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
        QModelIndex index( int row, int column, const QModelIndex &parent = {} ) const override;
        QModelIndex parent( const QModelIndex &index ) const override;
        int rowCount( const QModelIndex &parent = {} ) const override;
        int columnCount( const QModelIndex &parent = {} ) const override;

        /**
       * Initialize (or reinitialize if already initialized) model
       */
        void init();

        /**
       * Reset all current modifications
       */
        void reset();

        /**
       * Apply modification to the application
       */
        void apply();

        /**
       * Read new customization model file and reset model
       * Returns error string. An empty string is returned if no error occurred
       */
        QString readFile( const QString &filePath );

        /**
       * Returns current customization
       */
        const std::unique_ptr<QgsCustomization> &customization() const;

      private:
        Mode mMode = Mode::ActionSelector;
        QgisApp *mQgisApp = nullptr;
        std::unique_ptr<QgsCustomization> mCustomization; // current customization, copy of the application one
        QList<QgsCustomization::QgsItem *> mRootItems;
    };

    QString mLastDirSettingsName;

    QgisApp *mQgisApp = nullptr;
    QgsCustomizationModel *mItemsVisibilityModel = nullptr;

    friend class TestQgsCustomization;
};

#endif
