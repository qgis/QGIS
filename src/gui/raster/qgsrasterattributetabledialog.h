/***************************************************************************
  qgsrasterattributetabledialog.h - QgsRasterAttributeTableDialog

 ---------------------
 begin                : 13.10.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERATTRIBUTETABLEDIALOG_H
#define QGSRASTERATTRIBUTETABLEDIALOG_H

#include <QDialog>
#include "qgis_gui.h"
#include "ui_qgsrasterattributetabledialogbase.h"

#ifndef SIP_RUN
class QgsRasterLayer;
#endif

/**
 * \ingroup gui
 * \brief The QgsRasterAttributeTableDialog class embeds an attribute table widget.
 * \since QGIS 3.30
 */
class GUI_EXPORT QgsRasterAttributeTableDialog: public QDialog, private Ui::QRasterAttributeTableDialogBase
{
    Q_OBJECT

  public:

    /**
     * Create a new QgsRasterAttributeTableDialog
     */
    QgsRasterAttributeTableDialog( QgsRasterLayer *rasterLayer, int bandNumber = 0, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    // QDialog interface
  public slots:

    void reject() override;

};

#endif // QGSRASTERATTRIBUTETABLEDIALOG_H
