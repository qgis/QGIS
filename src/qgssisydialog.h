/***************************************************************************
                         qgssisydialog.h  -  description
                             -------------------
    begin                : Oct 2003
    copyright            : (C) 2003 by Marco Hugentobler
    email                : mhugent@geo.unizh.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSSISYDIALOG_H
#define QGSSISYDIALOG_H
#ifdef WIN32
#include "qgssisydialogbase.h"
#else
#include "qgssisydialogbase.uic.h"
#endif

class QgsVectorLayer;

/**QgsSiSyDialog is a dialog to set symbology for the legend type 'single symbol'*/
class QgsSiSyDialog: public QgsSiSyDialogBase
{
    Q_OBJECT
public:
    QgsSiSyDialog(QgsVectorLayer* layer);
    ~QgsSiSyDialog();
    QColor getOutlineColor();
    Qt::PenStyle getOutlineStyle();
    int getOutlineWidth();
    QColor getFillColor();
    Qt::BrushStyle getFillStyle();
    void setOutlineColor(QColor& c);
    void setOutlineStyle(Qt::PenStyle pstyle);
    void setOutlineWidth(int width);
    void setFillColor(QColor& c);
    void setFillStyle(Qt::BrushStyle fstyle);
    void setLabel(QString label);
    QString label();


protected:
    QgsVectorLayer* mVectorLayer;
public slots:
    /**applies the changes to the vector layer*/
    void apply();
    /**emits the signal settingsChanged()*/
    void resendSettingsChanged();


    void solid_clicked();
    void fdiag_clicked();
    void dense4_clicked();
    void horizontal_clicked();
    void diagcross_clicked();
    void dense5_clicked();
    void vertical_clicked();
    void dense1_clicked();
    void dense6_clicked();
    void cross_clicked();
    void dense2_clicked();
    void dense7_clicked();
    void bdiag_clicked();
    void dense3_clicked();
    void nopen_clicked();
    void pbnLineSolid_clicked();
    void pbnLineDashDot_clicked();
    void pbnLineDash_clicked();
    void pbnLineDashDotDot_clicked();
    void pbnLineDot_clicked();
    void pbnLineNoPen_clicked();



protected slots:
    void selectOutlineColor();
    void selectFillColor();
private:
    /**Default constructor is privat to not use is*/
    QgsSiSyDialog();

signals:
    void settingsChanged();
};

#endif
