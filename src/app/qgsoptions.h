/***************************************************************************
                          qgsoptions.h
                        Set user options and preferences
                             -------------------
    begin                : May 28, 2004
    copyright            : (C) 2004 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSOPTIONS_H
#define QGSOPTIONS_H

#include "ui_qgsoptionsbase.h"
#include "qgisgui.h"
#include "qgisapp.h"
#include "qgisappstylesheet.h"
#include "qgscontexthelp.h"

#include <qgscoordinatereferencesystem.h>

#include <QList>

/**
 * \class QgsOptions
 * \brief Set user options and preferences
 */
class QgsOptions : public QDialog, private Ui::QgsOptionsBase
{
    Q_OBJECT
  public:
    /**
     * Constructor
     * @param parent Parent widget (usually a QgisApp)
     * @param name name for the widget
     * @param modal true for modal dialog
     */
    QgsOptions( QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsOptions();
    /**
     * Return the currently selected theme
     * @return theme name (a directory name in the themes directory)
     */
    QString theme();

  public slots:
    void on_cbxProjectDefaultNew_toggled( bool checked );
    void on_pbnProjectDefaultSetCurrent_clicked();
    void on_pbnProjectDefaultReset_clicked();
    void on_pbnTemplateFolderBrowse_pressed();
    void on_pbnTemplateFolderReset_pressed();
    //! Slot called when user chooses to change the project wide projection.
    void on_pbnSelectProjection_clicked();
    //! Slot called when user chooses to change the default 'on the fly' projection.
    void on_pbnSelectOtfProjection_clicked();
    void on_lstGdalDrivers_itemDoubleClicked( QTreeWidgetItem * item, int column );
    void on_pbnEditCreateOptions_pressed();
    void on_pbnEditPyramidsOptions_pressed();
    void editGdalDriver( const QString& driverName );
    void saveOptions();
    /*!
    * Slot to reset any temporarily applied options on dialog close/cancel
    * @note added in QGIS 2.0
    */
    void rejectOptions();
    //! Slot to change the theme this is handled when the user
    // activates or highlights a theme name in the drop-down list
    void themeChanged( const QString & );

    void iconSizeChanged( const QString &iconSize );

    /** Slot to handle when type of project to open after launch is changed
     * @note added in QGIS 1.9
     */
    void on_mProjectOnLaunchCmbBx_currentIndexChanged( int indx );

    /** Slot to choose path to project to open after launch
     * @note added in QGIS 1.9
     */
    void on_mProjectOnLaunchPushBtn_pressed();

    //! Slot to change backbuffering. This is handled when the user changes
    // the value of the checkbox
    void toggleEnableBackbuffer( int );

    /**
     * Return the desired state of newly added layers. If a layer
     * is to be drawn when added to the map, this function returns
     * true.
     */
    bool newVisible();

    /** Slot to select the default font point size for app
     * @note added in QGIS 1.9
     */
    void on_spinFontSize_valueChanged( int fontSize );

    /** Slot to set font family for app to Qt default
     * @note added in QGIS 1.9
     */
    void on_mFontFamilyRadioQt_released();

    /** Slot to set font family for app to custom choice
     * @note added in QGIS 1.9
     */
    void on_mFontFamilyRadioCustom_released();

    /** Slot to select custom font family choice for app
     * @note added in QGIS 1.9
     */
    void on_mFontFamilyComboBox_currentFontChanged( const QFont& font );

    /** Slot to set whether to use custom group boxes
     * @note added in QGIS 1.9
     */
    void on_mCustomGroupBoxChkBx_clicked( bool chkd );

    /** Slot to set whether to bold group box titles
     * @note added in QGIS 1.9
     */
    void on_mBoldGroupBoxTitleChkBx_clicked( bool chkd );

    /**Add a new URL to exclude from Proxy*/
    void on_mAddUrlPushButton_clicked();

    /**Remove an URL to exclude from Proxy*/
    void on_mRemoveUrlPushButton_clicked();

    /** Slot to enable custom environment variables table and buttons
     * @note added in QGIS 1.9
     */
    void on_mCustomVariablesChkBx_toggled( bool chkd );

    /** Slot to add a custom environment variable to the app
     * @note added in QGIS 1.9
     */
    void on_mAddCustomVarBtn_clicked();

    /** Slot to remove a custom environment variable from the app
     * @note added in QGIS 1.9
     */
    void on_mRemoveCustomVarBtn_clicked();

    /** Slot to filter out current environment variables not specific to QGIS
     * @note added in QGIS 1.9
     */
    void on_mCurrentVariablesQGISChxBx_toggled( bool qgisSpecific );

    /* Let the user add a path to the list of search paths
     * used for finding user Plugin libs.
     * @note added in QGIS 1.7
     */
    void on_mBtnAddPluginPath_clicked();

    /* Let the user remove a path from the list of search paths
     * used for finding Plugin libs.
     * @note added in QGIS 1.7
     */
    void on_mBtnRemovePluginPath_clicked();

    /* Let the user add a path to the list of search paths
     * used for finding SVG files.
     * @note added in QGIS 1.4
     */
    void on_mBtnAddSVGPath_clicked();

    /* Let the user remove a path from the list of search paths
     * used for finding SVG files.
     * @note added in QGIS 1.4
     */
    void on_mBtnRemoveSVGPath_clicked();

    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    void on_mBrowseCacheDirectory_clicked();
    void on_mClearCache_clicked();

    /** Let the user add a scale to the list of scales
     * used in scale combobox
     * @note added in QGIS 2.0
     */
    void on_pbnAddScale_clicked();

    /** Let the user remove a scale from the list of scales
     * used in scale combobox
     * @note added in QGIS 2.0
     */
    void on_pbnRemoveScale_clicked();

    /** Let the user restore default scales
     * used in scale combobox
     * @note added in QGIS 2.0
     */
    void on_pbnDefaultScaleValues_clicked();

    /** Let the user load scales from file
     * @note added in QGIS 2.0
     */
    void on_pbnImportScales_clicked();

    /** Let the user load scales from file
     * @note added in QGIS 2.0
     */
    void on_pbnExportScales_clicked();

    /** Auto slot executed when the active page in the option section widget is changed
     * @note added in 1.9
     */
    void on_mOptionsStackedWidget_currentChanged( int theIndx );

    /** Slot to update widget of vertical tabs
     * @note added in QGIS 1.9
     */
    void updateVerticalTabs();

    /* Load the list of drivers available in GDAL
     * @note added in 2.0
     */
    void loadGdalDriverList();

    /* Save the list of which gdal drivers should be used.
     * @note added in 2.0
     */
    void saveGdalDriverList();

  private:
    QStringList i18nList();
    void initContrastEnhancement( QComboBox *cbox, QString name, QString defaultVal );
    void saveContrastEnhancement( QComboBox *cbox, QString name );
    QgsCoordinateReferenceSystem mDefaultCrs;
    QgsCoordinateReferenceSystem mLayerDefaultCrs;
    bool mLoadedGdalDriverList;

    /** Generate table row for custom environment variables
     * @note added in QGIS 1.9
     */
    void addCustomEnvVarRow( QString varName, QString varVal, QString varApply = QString() );

  protected:
    void showEvent( QShowEvent * e );
    void paintEvent( QPaintEvent * e );

    QgisAppStyleSheet* mStyleSheetBuilder;
    QMap<QString, QVariant> mStyleSheetNewOpts;
    QMap<QString, QVariant> mStyleSheetOldOpts;
};

#endif // #ifndef QGSOPTIONS_H
