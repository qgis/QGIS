/***************************************************************************
                          qgsrasterterrainanalysisdialog.h  -  description
                             -----------------------------
    begin                : August 8th, 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERTERRAINANALYSISDIALOG_H
#define QGSRASTERTERRAINANALYSISDIALOG_H

#include "ui_qgsrasterterrainanalysisdialogbase.h"

class QgisInterface;

class QgsRasterTerrainAnalysisDialog: public QDialog, private Ui::QgsRasterTerrainAnalysisDialogBase
{
    Q_OBJECT
  public:
    QgsRasterTerrainAnalysisDialog( QgisInterface* iface, QWidget* parent = 0 );
    ~QgsRasterTerrainAnalysisDialog();

    QString selectedInputLayerId() const;
    QString selectedDriverKey() const;
    QString selectedOuputFilePath() const;
    QString selectedAnalysisMethod() const;
    bool addLayerToProject() const;

  private slots:
    void on_mOutputLayerLineEdit_textChanged( const QString& text );
    void on_mOutputLayerPushButton_clicked();
    void on_mButtonBox_accepted();

  private:
    QgisInterface* mIface;
};

#endif // QGSRASTERTERRAINANALYSISDIALOG_H
