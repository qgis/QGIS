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


#include "qgsoptionsdialogbase.h"
#include "ui_qgsprojectpropertiesbase.h"
#include "qgis.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

class QgsMapCanvas;
class QgsRelationManagerDialog;
class QgsStyleV2;

/** Dialog to set project level properties

  @note actual state is stored in QgsProject singleton instance

 */
class APP_EXPORT QgsProjectProperties : public QgsOptionsDialogBase, private Ui::QgsProjectPropertiesBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsProjectProperties( QgsMapCanvas* mapCanvas, QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

    //! Destructor
    ~QgsProjectProperties();

    /** Gets the currently select map units
     */
    QGis::UnitType mapUnits() const;

    /*!
     * Set the map units
     */
    void setMapUnits( QGis::UnitType );

    /*!
       Every project has a title
    */
    QString title() const;
    void title( QString const & title );

    /** Accessor for projection */
    QString projectionWkt();

    /** Indicates that the projection switch is on */
    bool isProjected();

  public slots:
    /*!
     * Slot called when apply button is pressed or dialog is accepted
     */
    void apply();

    /*!
     * Slot to show the projections tab when the dialog is opened
     */
    void showProjectionsTab();

    /** Let the user add a scale to the list of project scales
     * used in scale combobox instead of global ones */
    void on_pbnAddScale_clicked();

    /** Let the user remove a scale from the list of project scales
     * used in scale combobox instead of global ones */
    void on_pbnRemoveScale_clicked();

    /** Let the user load scales from file */
    void on_pbnImportScales_clicked();

    /** Let the user load scales from file */
    void on_pbnExportScales_clicked();

    /*!
     * Slots for WMS project settings
     */
    void on_pbnWMSExtCanvas_clicked();
    void on_pbnWMSAddSRS_clicked();
    void on_pbnWMSRemoveSRS_clicked();
    void on_pbnWMSSetUsedSRS_clicked();
    void on_mAddWMSComposerButton_clicked();
    void on_mRemoveWMSComposerButton_clicked();
    void on_mAddLayerRestrictionButton_clicked();
    void on_mRemoveLayerRestrictionButton_clicked();

    /*!
     * Slots to select/unselect all the WFS layers
     */
    void on_pbnWFSLayersSelectAll_clicked();
    void on_pbnWFSLayersUnselectAll_clicked();

    /*!
     * Slots to select/unselect all the WCS layers
     */
    void on_pbnWCSLayersSelectAll_clicked();
    void on_pbnWCSLayersUnselectAll_clicked();

    /*!
     * Slots for Styles
     */
    void on_pbtnStyleManager_clicked();
    void on_pbtnStyleMarker_clicked();
    void on_pbtnStyleLine_clicked();
    void on_pbtnStyleFill_clicked();
    void on_pbtnStyleColorRamp_clicked();
    void on_mTransparencySlider_valueChanged( int value );
    void on_mTransparencySpinBox_valueChanged( int value );

    /*!
     * Slot to show the context help for this dialog
     */
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    void on_cbxProjectionEnabled_toggled( bool onFlyEnabled );

    /*!
     * Slot to link WFS checkboxes
     */
    void cbxWFSPubliedStateChanged( int aIdx );

    /*!
     * Slot to link WCS checkboxes
     */
    void cbxWCSPubliedStateChanged( int aIdx );

    /*!
      * If user changes the CRS, set the corresponding map units
      */
    void setMapUnitsToCurrentProjection();

    /* Update ComboBox accorindg to the selected new index
     * Also sets the new selected Ellipsoid. */
    void updateEllipsoidUI( int newIndex );

    //! sets the right ellipsoid for measuring (from settings)
    void projectionSelectorInitialized();

    void on_mButtonAddColor_clicked();
    void on_mButtonImportColors_clicked();
    void on_mButtonExportColors_clicked();

  signals:
    //! Signal used to inform listeners that the mouse display precision may have changed
    void displayPrecisionChanged();

    //! Signal used to inform listeners that project scale list may have chnaged
    void scalesChanged( const QStringList &scales = QStringList() );

    //! let listening canvases know to refresh
    void refresh();

  private:
    QgsRelationManagerDialog *mRelationManagerDlg;
    QgsMapCanvas* mMapCanvas;
    QgsStyleV2* mStyle;

    void populateStyles();
    void editSymbol( QComboBox* cbo );

    /*!
     * Function to save non-base dialog states
     */
    void saveState();

    /*!
     * Function to restore non-base dialog states
     */
    void restoreState();

    /*!
     * Reset the python macros
     */
    void resetPythonMacros();

    long mProjectSrsId;
    long mLayerSrsId;

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

    //! Populates list with ellipsoids from Sqlite3 db
    void populateEllipsoidList();

    static const char * GEO_NONE_DESC;

};
