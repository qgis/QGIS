/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
   ***************************************************************************
   *                                                                         *
   *   This program is free software; you can redistribute it and/or modify  *
   *   it under the terms of the GNU General Public License as published by  *
   *   the Free Software Foundation; either version 2 of the License, or     *
   *   (at your option) any later version.                                   *
   *                                                                         *
   ***************************************************************************/

#ifndef QGSGROUPWMSDATADIALOG_H
#define QGSGROUPWMSDATADIALOG_H

#include "ui_qgsgroupwmsdatadialogbase.h"
#include "qgsguiutils.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsGroupWmsDataDialog
 * \brief A dialog for configuring a WMS group.
 */
class GUI_EXPORT QgsGroupWmsDataDialog : public QDialog, private Ui::QgsGroupWMSDataDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param parent parent widget
     * \param fl dialog window flags
     */
    QgsGroupWmsDataDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    /**
     * Constructor
     * \param serverProperties used to initialize the dialog
     * \param parent parent widget
     * \param fl dialog window flags
     */
    QgsGroupWmsDataDialog( const QgsMapLayerServerProperties &serverProperties, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    ~QgsGroupWmsDataDialog() override = default;

    /**
     * Returns group WMS title
     *
     * \deprecated QGIS 3.44. Use serverProperties()->title() instead.
     */
    Q_DECL_DEPRECATED QString groupTitle() const;

    /**
     * Returns group WMS short name
     *
     * \deprecated QGIS 3.44. Use serverProperties()->shortName() instead.
     */
    Q_DECL_DEPRECATED QString groupShortName() const;

    /**
     * Returns group WMS abstract
     *
     * \deprecated QGIS 3.44. Use serverProperties()->abstract() instead.
     */
    Q_DECL_DEPRECATED QString groupAbstract() const;

    /**
     * Sets group WMS title
     *
     * \deprecated QGIS 3.44. Use serverProperties()->setTitle() instead.
     */
    Q_DECL_DEPRECATED void setGroupTitle( const QString &title ) SIP_DEPRECATED;

    /**
     * Sets group WMS short name
     *
     * \deprecated QGIS 3.44. Use serverProperties()->setShortName() instead.
     */
    Q_DECL_DEPRECATED void setGroupShortName( const QString &shortName ) SIP_DEPRECATED;

    /**
     * Sets group WMS abstract
     *
     * \deprecated QGIS 3.44. Use serverProperties()->setAbstract() instead.
     */
    Q_DECL_DEPRECATED void setGroupAbstract( const QString &abstract ) SIP_DEPRECATED;

    /**
     * Returns QGIS Server Properties for the layer tree group
     * \since QGIS 3.44
     */
    QgsMapLayerServerProperties *serverProperties();

    /**
     * Returns QGIS Server Properties const for the layer tree group
     * \since QGIS 3.44
     */
    const QgsMapLayerServerProperties *serverProperties() const SIP_SKIP;

    void accept() override;

  private:
    std::unique_ptr<QgsMapLayerServerProperties> mServerProperties;
};

#endif // QGSGROUPWMSDATADIALOG_H
