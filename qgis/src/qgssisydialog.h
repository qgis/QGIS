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

#include <iostream>
#include <vector>

#ifdef WIN32
#include "qgssisydialogbase.h"
#else
#include "qgssisydialogbase.uic.h"
#endif

class QString;
class QgsSingleSymRenderer;
class QgsSymbol;
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
    /* set from QgsSymbol */
    void set(const QgsSymbol *sy);
    /**applies the changes to the vector layer*/
    void apply();
    /**applies the changes to the QgsSymbol */
    void apply( QgsSymbol *sy);
    /**emits the signal settingsChanged()*/
    void resendSettingsChanged();

protected slots:
    void selectOutlineColor();
    void selectFillColor();
private:
    /**Default constructor is privat to not use is*/
    QgsSiSyDialog();

    /** vector of marker names for combo items */
    std::vector<QString> mMarkers;

signals:
    void settingsChanged();
};

#endif
