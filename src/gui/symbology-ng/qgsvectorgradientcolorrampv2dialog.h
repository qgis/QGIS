/***************************************************************************
    qgsvectorgradientcolorrampv2dialog.h
    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORGRADIENTCOLORRAMPV2DIALOG_H
#define QGSVECTORGRADIENTCOLORRAMPV2DIALOG_H

#include <QDialog>

#include "ui_qgsvectorgradientcolorrampv2dialogbase.h"

class QgsVectorGradientColorRampV2;

class GUI_EXPORT QgsVectorGradientColorRampV2Dialog : public QDialog, private Ui::QgsVectorGradientColorRampV2DialogBase
{
    Q_OBJECT

  public:
    QgsVectorGradientColorRampV2Dialog( QgsVectorGradientColorRampV2* ramp, QWidget* parent = NULL );

  public slots:
    void setColor1( const QColor& color );
    void setColor2( const QColor& color );

    void toggledStops( bool on );
    void addStop();
    void removeStop();

    void stopDoubleClicked( QTreeWidgetItem* item, int column );
    void setItemStopColor( const QColor& newColor );

  protected slots:
    void on_cboType_currentIndexChanged( int index );
    void on_btnInformation_pressed();

  protected:

    void updateStops();
    void updatePreview();
    void setStopColor( QTreeWidgetItem* item, QColor color );

    QgsVectorGradientColorRampV2* mRamp;

    static const int StopColorRole = Qt::UserRole + 1;
    static const int StopOffsetRole = Qt::UserRole + 2;

    QTreeWidgetItem* mCurrentItem;
};

#endif
