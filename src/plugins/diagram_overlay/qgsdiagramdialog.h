/***************************************************************************
                         qgsdiagramdialog.h  -  description
                         ------------------
    begin                : January 2007
    copyright            : (C) 2007 by Marco Hugentobler
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

#ifndef QGSDIAGRAMDIALOG_H
#define QGSDIAGRAMDIALOG_H

#include "ui_qgsdiagramdialogbase.h"
#include "qgsapplydialog.h"

class QgsVectorLayer;
class QgsVectorOverlay;

/**Dialog to enter options for diagram symbolisation*/
class QgsDiagramDialog: public QgsApplyDialog, private Ui::QgsDiagramDialogBase
{
    Q_OBJECT
  public:
    QgsDiagramDialog( QgsVectorLayer* vl );
    ~QgsDiagramDialog();
    void apply() const;

  private slots:
    void on_mClassificationTypeComboBox_currentIndexChanged( const QString& newType );
    void on_mClassificationComboBox_currentIndexChanged( const QString& newAttribute );
    void on_mDiagramTypeComboBox_currentIndexChanged( const QString& text );
    void on_mDisplayDiagramsCheckBox_stateChanged( int state );


  private:
    QgsDiagramDialog();
    /**Restores the dialog settings from an already existing overlay*/
    void restoreSettings( const QgsVectorOverlay* overlay );
    /**Vector layer containing the features to be symbolized with diagrams*/
    QgsVectorLayer* mVectorLayer;
    /**Activate / deactivate all the input elements of this dialog*/
    void setGuiElementsEnabled( bool enabled );
};

#endif
