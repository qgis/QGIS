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

#include "qgsoptionsdialogbase.h"
#include "ui_qgsoptionsbase.h"
#include "qgsguiutils.h"
#include "qgisapp.h"
#include "qgshelp.h"

#include "qgscoordinatereferencesystem.h"

#include <QList>
#include "qgis_app.h"

class QgsExpressionContext;
class QgsOptionsPageWidget;
class QgsLocatorOptionsWidget;
class QgsAuthConfigSelect;
class QgsBearingNumericFormat;
class QgsGeographicCoordinateNumericFormat;
class QStandardItemModel;

/**
 * \class QgsOptions
 * \brief Set user options and preferences
 */
class APP_EXPORT QgsOptions : public QgsOptionsDialogBase, private Ui::QgsOptionsBase
{
    Q_OBJECT
  public:

    /**
     * Behavior to use when encountering a layer with an unknown CRS
     * \since QGIS 3.10
     */
    enum UnknownLayerCrsBehavior
    {
      NoAction = 0, //!< Take no action and leave as unknown CRS
      PromptUserForCrs = 1, //!< User is prompted for a CRS choice
      UseProjectCrs = 2, //!< Copy the current project's CRS
      UseDefaultCrs = 3, //!< Use the default layer CRS set via QGIS options
    };
    Q_ENUM( UnknownLayerCrsBehavior )


    /**
     * Constructor
     * \param parent Parent widget (usually a QgisApp)
     * \param name name for the widget
     * \param modal TRUE for modal dialog
     * \param optionsFactories factories for additional option pages
     */
    QgsOptions( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
                const QList<QgsOptionsWidgetFactory *> &optionsFactories = QList<QgsOptionsWidgetFactory *>() );

    ~QgsOptions() override;

    /**
     * Sets the page with the specified widget name as the current page
     * \since QGIS 2.1
     */
    void setCurrentPage( const QString &pageWidgetName );

    void setCurrentPage( int pageNumber );

  public slots:
    void cbxProjectDefaultNew_toggled( bool checked );
    void setCurrentProjectDefault();
    void resetProjectDefault();
    void browseTemplateFolder();
    void resetTemplateFolder();
    //! Slot called when user chooses to change the default 'on the fly' projection.
    void leLayerGlobalCrs_crsChanged( const QgsCoordinateReferenceSystem &crs );
    void lstRasterDrivers_itemDoubleClicked( QTreeWidgetItem *item, int column );
    void editCreateOptions();
    void editPyramidsOptions();
    void editGdalDriver( const QString &driverName );
    void saveOptions();

    /**
     * Slot to reset any temporarily applied options on dialog close/cancel
     */
    void rejectOptions();

    //! Slot to change the theme this is handled when the user
    void iconSizeChanged( const QString &iconSize );

    void uiThemeChanged( const QString &theme );

    /**
     * Slot to handle when type of project to open after launch is changed
     */
    void mProjectOnLaunchCmbBx_currentIndexChanged( int indx );

    //! Slot to choose path to project to open after launch
    void selectProjectOnLaunch();

    /**
     * Returns the desired state of newly added layers. If a layer
     * is to be drawn when added to the map, this function returns
     * TRUE.
     */
    bool newVisible();

    //! Slot to select the default font point size for app
    void spinFontSize_valueChanged( int fontSize );

    //! Slot to set font family for app to Qt default
    void mFontFamilyRadioQt_released();

    //! Slot to set font family for app to custom choice
    void mFontFamilyRadioCustom_released();

    //! Slot to select custom font family choice for app
    void mFontFamilyComboBox_currentFontChanged( const QFont &font );

    void mProxyTypeComboBox_currentIndexChanged( int idx );

    //! Add a new URL to no proxy URL list
    void addNoProxyUrl();

    //! Remove current URL from no proxy URL list
    void removeNoProxyUrl();

    //! Slot to flag restoring/delete window state settings upon restart
    void restoreDefaultWindowState();

    //! Slot to enable custom environment variables table and buttons
    void mCustomVariablesChkBx_toggled( bool chkd );

    //! Slot to add a custom environment variable to the app
    void addCustomVariable();

    //! Slot to remove a custom environment variable from the app
    void removeCustomVariable();

    //! Slot to filter out current environment variables not specific to QGIS
    void mCurrentVariablesQGISChxBx_toggled( bool qgisSpecific );

    /**
     * Let the user add a path to the list of search paths
     * used for finding user Plugin libs.
    */
    void addPluginPath();

    /**
     * Let the user remove a path from the list of search paths
     * used for finding Plugin libs.
    */
    void removePluginPath();

    /**
     * Let the user add a path to the list of search paths
     * used for finding QGIS help.
    */
    void addHelpPath();

    /**
     * Let the user remove a path from the list of search paths
     * used for finding QGIS help.
    */
    void removeHelpPath();

    /**
     * Let the user move selected path(s) up in the list raising
     * their priority.
    */
    void moveHelpPathUp();

    /**
     * Let the user move selected path(s) down in the list lowering
     * their priority.
    */
    void moveHelpPathDown();

    /**
     * Let the user add a path to the list of search paths
     * used for finding composer template files.
    */
    void addTemplatePath();

    /**
     * Let the user remove a path from the list of search paths
     * used for finding composer template files.
    */
    void removeTemplatePath();

    /**
     * Let the user add a path to the list of search paths
     * used for finding SVG files.
    */
    void addSVGPath();

    /**
     * Let the user remove a path from the list of search paths
     * used for finding SVG files.
    */
    void removeSVGPath();

    /**
     * Let the user remove a path from the hidden path list
     * for the browser.
    */
    void removeHiddenPath();

    void browseCacheDirectory();
    void clearCache();

    /**
     * \brief clearAuthenticationConnectionCache clears the QNetworkAccessManager
     * authentication connection cache
     */
    void clearAccessCache();

    /**
     * Let the user add a scale to the list of scales
     * used in scale combobox
     */
    void addScale();

    /**
     * Let the user remove a scale from the list of scales
     * used in scale combobox
     */
    void removeScale();

    /**
     * Let the user restore default scales
     * used in scale combobox.
    */
    void restoreDefaultScaleValues();

    //! Let the user load scales from file
    void importScales();

    //! Let the user load scales from file
    void exportScales();

    //! Auto slot executed when the active page in the option section widget is changed
    void optionsStackedWidget_CurrentChanged( int index ) override;

    //! A scale in the list of predefined scales changed
    void scaleItemChanged( QListWidgetItem *changedScaleItem );

    /* Load the list of drivers available in GDAL */
    void loadGdalDriverList();

    /* Save the list of which gdal drivers should be used. */
    void saveGdalDriverList();

    void addColor();

  private slots:
    void removeLocalizedDataPath();
    void addLocalizedDataPath();
    void moveLocalizedDataPathUp();
    void moveLocalizedDataPathDown();

  private:
    QgsSettings *mSettings = nullptr;
    QStringList i18nList();

    void initContrastEnhancement( QComboBox *cbox, const QString &name, const QString &defaultVal );
    void saveContrastEnhancement( QComboBox *cbox, const QString &name );
    void initMinMaxLimits( QComboBox *cbox, const QString &name, const QString &defaultVal );
    void saveMinMaxLimits( QComboBox *cbox, const QString &name );
    void setZoomFactorValue();
    double zoomFactorValue();
    QgsCoordinateReferenceSystem mLayerDefaultCrs;
    bool mLoadedGdalDriverList;

    //! Generate table row for custom environment variables
    void addCustomEnvVarRow( const QString &varName, const QString &varVal, const QString &varApply = QString() );

    void showHelp();

    QListWidgetItem *addScaleToScaleList( const QString &newScale );
    void addScaleToScaleList( QListWidgetItem *newItem );

    void refreshSchemeComboBox();

    void updateSampleLocaleText();

    void customizeBearingFormat();
    void customizeCoordinateFormat();

  protected:
    QgisAppStyleSheet *mStyleSheetBuilder = nullptr;
    QMap<QString, QVariant> mStyleSheetNewOpts;
    QMap<QString, QVariant> mStyleSheetOldOpts;

    static const int PALETTE_COLOR_ROLE = Qt::UserRole + 1;
    static const int PALETTE_LABEL_ROLE = Qt::UserRole + 2;

  private:

    QList< QgsOptionsPageWidget * > mAdditionalOptionWidgets;
    QgsLocatorOptionsWidget *mLocatorOptionsWidget = nullptr;

    std::unique_ptr< QgsBearingNumericFormat > mBearingFormat;
    std::unique_ptr< QgsGeographicCoordinateNumericFormat > mCoordinateFormat;

    QStandardItemModel *mTreeModel = nullptr;

    void updateActionsForCurrentColorScheme( QgsColorScheme *scheme );

    void checkPageWidgetNameMap();

    friend class QgsAppScreenShots;
};

#endif // #ifndef QGSOPTIONS_H
