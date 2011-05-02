/***************************************************************************
                         qgslinearlyscalingdialog.h  -  description
                         --------------------------
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

#ifndef QGSLINEARLYSCALINGDIALOG_H
#define QGSLINEARLYSCALINGDIALOG_H

#include "ui_qgslinearlyscalingdialogbase.h"
#include "qgsdiagramrendererwidget.h"

class QgsDiagramRenderer;
class QgsVectorLayer;

class QgsLinearlyScalingDialog: public QgsDiagramRendererWidget, private Ui::QgsLinearlyScalingDialogBase
{
    Q_OBJECT
  public:
    QgsLinearlyScalingDialog( QgsVectorLayer* vl );
    ~QgsLinearlyScalingDialog();
    QgsDiagramRenderer* createRenderer( int classAttr, const QgsAttributeList& attributes ) const;
    void applySettings( const QgsDiagramRenderer* renderer );
    /**Is called from QgsDiagramDialog. Inserts new maximum value into the text widget*/
    void changeClassificationField( int newField ) {mClassificationField = newField;}
    void setWellKnownName( const QString& wkn );

    QgsDiagramFactory::SizeUnit sizeUnit() const;

  private:
    QString mClassificationAttribute;
    QString mWellKnownName;
    int mClassificationField;

  private slots:
    /**Calculates and inserts the maximum attribute value for the classification field*/
    void insertMaximumAttributeValue();
};

#endif
