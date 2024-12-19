/***************************************************************************
                              qgswmsdimensiondialog.h
                              ------------------
  begin                : August 20, 2019
  copyright            : (C) 2019 by René-Luc D'Hont
  email                : rldhont at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMSDIMENSIONDIALOG_H
#define QGSWMSDIMENSIONDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgswmsdimensiondialogbase.h"
#include "qgsmaplayerserverproperties.h"
#include "qgis_gui.h"

class QgsVectorLayer;

/**
 * \ingroup gui
 * \class QgsWmsDimensionDialog
 *
 * \brief The QgsWmsDimensionDialog class provides an interface for WMS/OAPIF (WFS3) dimensions configuration
 * Available pre-defined dimensions are
 *
 * - DATE (supported by OAPIF only)
 * - TIME (supported by WMS and OAPIF)
 * - ELEVATION (supported by WMS only)
 *
 * Dimensions can also be configured as ranges by defining an "end" field that contains the
 * upper value of the range.
 */
class GUI_EXPORT QgsWmsDimensionDialog: public QDialog, private Ui::QgsWmsDimensionDialogBase
{
    Q_OBJECT
  public:
    QgsWmsDimensionDialog( QgsVectorLayer *layer, QStringList alreadyDefinedDimensions, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    QgsMapLayerServerProperties::WmsDimensionInfo info() const;

    void setInfo( const QgsMapLayerServerProperties::WmsDimensionInfo &info );

  private slots:
    void nameChanged( const QString &name );
    void fieldChanged();
    void defaultDisplayChanged( int index );

  private:
    //! Target layer
    QgsVectorLayer *mLayer = nullptr;
};


#endif // QGSWMSDIMENSIONDIALOG_H
