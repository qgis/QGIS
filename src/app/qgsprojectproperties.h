/***************************************************************************
                             qgsprojectproperties.h
       Set various project properties (inherits qgsprojectpropertiesbase)
                              -------------------
  begin                : May 18, 2004
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


#include "ui_qgsprojectpropertiesbase.h"

#include "qgsoptionsdialogbase.h"
#include "qgsoptionswidgetfactory.h"
#include "qgis.h"
#include "qgsunittypes.h"
#include "qgsguiutils.h"
#include "qgsscalewidget.h"
#include "qgshelp.h"
#include "qgis_app.h"

#include <QList>

class QgsMapCanvas;
class QgsRelationManagerDialog;
class QgsStyle;
class QgsExpressionContext;
class QgsLayerTreeGroup;
class QgsMetadataWidget;
class QgsTreeWidgetItem;
class QgsLayerCapabilitiesModel;
class QgsBearingNumericFormat;
class QgsGeographicCoordinateNumericFormat;
class QgsOptionsPageWidget;

/**
 * Dialog to set project level properties
 *
 * \note actual state is stored in QgsProject singleton instance
 *
 */
class APP_EXPORT QgsProjectProperties : public QgsOptionsDialogBase, private Ui::QgsProjectPropertiesBase, public QgsExpressionContextGenerator
{
    Q_OBJECT

  public:
    //! Constructor
    QgsProjectProperties( QgsMapCanvas *mapCanvas, QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
                          const QList<QgsOptionsWidgetFactory *> &optionsFactories = QList<QgsOptionsWidgetFactory *>() );

    ~QgsProjectProperties() override;

    void setCurrentPage( const QString & );

    /**
      * Every project has a title
     */
    QString title() const;
    void title( QString const &title );

    /**
     * Sets the \a crs shown as selected within the dialog.
     */
    void setSelectedCrs( const QgsCoordinateReferenceSystem &crs );

    QgsExpressionContext createExpressionContext() const override;
  public slots:

    /**
     * Slot called when apply button is pressed or dialog is accepted
     */
    void apply();

    /**
     * Let the user add a scale to the list of project scales
     * used in scale combobox instead of global ones.
    */
    void pbnAddScale_clicked();

    /**
     * Let the user remove a scale from the list of project scales
     * used in scale combobox instead of global ones.
    */
    void pbnRemoveScale_clicked();

    //! Let the user load scales from file
    void pbnImportScales_clicked();

    //! Let the user load scales from file
    void pbnExportScales_clicked();

    //! A scale in the list of project scales changed
    void scaleItemChanged( QListWidgetItem *changedScaleItem );

    //! generate the ts file with the locale selected in the checkbox
    void onGenerateTsFileButton() const;

    /**
     * Set WMS default extent to current canvas extent
     */
    void pbnWMSExtCanvas_clicked();

    /**
     *
     */
    void pbnWMSAddSRS_clicked();
    void pbnWMSRemoveSRS_clicked();
    void pbnWMSSetUsedSRS_clicked();

    /**
     * Slots to link WMS CRS list to WMTS Grids tree view
     */
    void lwWmsRowsInserted( const QModelIndex &parent, int first, int last );
    void lwWmsRowsRemoved( const QModelIndex &parent, int first, int last );

    /**
     *
     */
    void mAddWMSPrintLayoutButton_clicked();
    void mRemoveWMSPrintLayoutButton_clicked();
    void mAddLayerRestrictionButton_clicked();
    void mRemoveLayerRestrictionButton_clicked();
    void mWMSInspireScenario1_toggled( bool on );
    void mWMSInspireScenario2_toggled( bool on );

    /**
     * Slots to select/deselect all the WFS layers
     */
    void pbnWFSLayersSelectAll_clicked();
    void pbnWFSLayersDeselectAll_clicked();

    /**
     * Slots to select/deselect all the WCS layers
     */
    void pbnWCSLayersSelectAll_clicked();
    void pbnWCSLayersDeselectAll_clicked();

    /**
     * Slots to launch OWS test
     */
    void pbnLaunchOWSChecker_clicked();

    /**
     * Slot to link WMTS checkboxes in tree widget
     */
    void twWmtsItemChanged( QTreeWidgetItem *item, int column );
    void twWmtsGridItemDoubleClicked( QTreeWidgetItem *item, int column );
    void twWmtsGridItemChanged( QTreeWidgetItem *item, int column );

    /**
     * Slot to link WFS checkboxes
     */
    void cbxWFSPubliedStateChanged( int aIdx );

    /**
     * Slot to link WCS checkboxes
     */
    void cbxWCSPubliedStateChanged( int aIdx );

    /**
     * Update ComboBox accorindg to the selected new index
     * Also sets the new selected Ellipsoid.
    */
    void updateEllipsoidUI( int newIndex );

    void mButtonAddColor_clicked();

  signals:
    //! Signal used to inform listeners that the mouse display precision may have changed
    void displayPrecisionChanged();

  private slots:

    void customizeBearingFormat();
    void customizeGeographicCoordinateFormat();

    /**
     * Sets the start and end dates input values from the project
     * temporal layers.
     *
     * Looks for the smallest date and the greatest date from the
     * project layers and set them for start and end dates
     *  input respectively.
     *
     * \since QGIS 3.14
     */
    void calculateFromLayersButton_clicked();

    void addStyleDatabase();
    void removeStyleDatabase();
    void newStyleDatabase();

  private:

    /**
      * Called when the user sets a CRS for the project.
      */
    void crsChanged( const QgsCoordinateReferenceSystem &crs );

    //! Formats for displaying coordinates
    enum CoordinateFormat
    {
      Geographic, //!< Geographic
      MapUnits, //!< Show coordinates in map units
    };

    QgsRelationManagerDialog *mRelationManagerDlg = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
    QgsStyle *mStyle = nullptr;
    QgsMetadataWidget *mMetadataWidget = nullptr;
    QgsLayerCapabilitiesModel *mLayerCapabilitiesModel = nullptr;

    QDoubleSpinBox *mWMSDefaultMapUnitsPerMm = nullptr;
    QgsScaleWidget *mWMSDefaultMapUnitScale = nullptr;

    QgsCoordinateReferenceSystem mCrs;

    void checkPageWidgetNameMap();

    /**
     * Reset the Python macros
     */
    void resetPythonMacros();

    // List for all ellispods, also None and Custom
    struct EllipsoidDefs
    {
      QString acronym;
      QString description;
      double semiMajor;
      double semiMinor;
    };
    QList<EllipsoidDefs> mEllipsoidList;
    int mEllipsoidIndex;
    bool mBlockCrsUpdates = false;

    QList< QgsOptionsPageWidget * > mAdditionalProjectPropertiesWidgets;

    std::unique_ptr< QgsBearingNumericFormat > mBearingFormat;
    std::unique_ptr< QgsGeographicCoordinateNumericFormat > mGeographicCoordinateFormat;

    //! populate WMTS tree
    void populateWmtsTree( const QgsLayerTreeGroup *treeGroup, QgsTreeWidgetItem *treeItem );
    //! add WMTS Grid definition based on CRS
    void addWmtsGrid( const QString &crsStr );

    //! Populates list with ellipsoids from Sqlite3 db
    void populateEllipsoidList();

    void setCurrentEllipsoid( const QString &ellipsoidAcronym );

    //! Create a new scale item and add it to the list of scales
    QListWidgetItem *addScaleToScaleList( const QString &newScale );

    //! Add a scale item to the list of scales
    void addScaleToScaleList( QListWidgetItem *newItem );

    static const char *GEO_NONE_DESC;

    void updateGuiForMapUnits();
    void addStyleDatabasePrivate( bool createNew );

    void showHelp();

    friend class TestQgsProjectProperties;
};
