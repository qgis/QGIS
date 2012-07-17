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
#include "qgis.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

class QgsMapCanvas;
class QgsStyleV2;

/*!  Dialog to set project level properties

  @note actual state is stored in QgsProject singleton instance

 */
class QgsProjectProperties : public QDialog, private Ui::QgsProjectPropertiesBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsProjectProperties( QgsMapCanvas* mapCanvas, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );

    //! Destructor
    ~QgsProjectProperties();

    /*! Gets the currently select map units
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

    /*! Accessor for projection */
    QString projectionWkt();

    /*! Indicates that the projection switch is on */
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

    /*!
     * Slot to select the map selection color
     */
    void on_pbnSelectionColor_clicked();

    /*!
     * Slot to select the map selection color
     */
    void on_pbnCanvasColor_clicked();

    /*!
     * Slots for WMS project settings
     */
    void on_pbnWMSExtCanvas_clicked();
    void on_pbnWMSAddSRS_clicked();
    void on_pbnWMSRemoveSRS_clicked();
    void on_pbnWMSSetUsedSRS_clicked();

    /*!
     * Slots for Styles
     */
    void on_pbtnStyleManager_clicked();
    void on_pbtnStyleMarker_clicked();
    void on_pbtnStyleLine_clicked();
    void on_pbtnStyleFill_clicked();
    void on_pbtnStyleColorRamp_clicked();
    void on_mTransparencySlider_valueChanged( int value );

    /*!
     * Slot to show the context help for this dialog
     */
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }

    void on_cbxProjectionEnabled_stateChanged( int state );

    /*!
      * If user changes the CRS, set the corresponding map units
      */
    void setMapUnitsToCurrentProjection();

  signals:
    //! Signal used to inform listeners that the mouse display precision may have changed
    void displayPrecisionChanged();

    //! let listening canvases know to refresh
    void refresh();


  private:
    QgsMapCanvas* mMapCanvas;
    QgsStyleV2* mStyle;

    void populateStyles();
    void editSymbol( QComboBox* cbo );

    /*!
     * Function to save dialog window state
     */
    void saveState();

    /*!
     * Function to restore dialog window state
     */
    void restoreState();

    long mProjectSrsId;
    long mLayerSrsId;
};
