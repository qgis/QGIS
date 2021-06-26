/***************************************************************************
                         qgspointcloudsourceselect.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOINTCLOUDSOURCESELECT_H
#define QGSPOINTCLOUDSOURCESELECT_H

///@cond PRIVATE
#include "qgis_sip.h"
#define SIP_NO_FILE

#include "ui_qgspointcloudsourceselectbase.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgis_gui.h"


/**
 * \class QgsPointCloudSourceSelect
 * \brief Dialog to select point cloud supported sources
 */
class QgsPointCloudSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsPointCloudSourceSelectBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsPointCloudSourceSelect( QWidget *parent = nullptr,
                               Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags,
                               QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  public slots:
    //! Determines the tables the user selected and closes the dialog
    void addButtonClicked() override;

  private:
    QString mPath;

};

///@endcond
///
#endif // QGSPOINTCLOUDSOURCESELECT_H
