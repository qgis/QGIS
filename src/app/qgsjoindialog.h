/***************************************************************************
                              qgsjoindialog.h
                              ------------------
  begin                : July 10, 2010
  copyright            : (C) 2010 by Marco Hugentobler
  email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QgsJoinDIALOG_H
#define QgsJoinDIALOG_H

#include "ui_qgsjoindialogbase.h"

class QgsVectorLayer;
class QgsVectorJoinInfo;

class APP_EXPORT QgsJoinDialog: public QDialog, private Ui::QgsJoinDialogBase
{
    Q_OBJECT
  public:
    QgsJoinDialog( QgsVectorLayer* layer, QList<QgsMapLayer*> alreadyJoinedLayers, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsJoinDialog();

    /** Configure the dialog for an existing join */
    void setJoinInfo( const QgsVectorJoinInfo& joinInfo );

    /** Returns the join info */
    QgsVectorJoinInfo joinInfo() const;

    /** Returns true if user wants to create an attribute index on the join field*/
    bool createAttributeIndex() const;

  private slots:
    void joinedLayerChanged( QgsMapLayer* layer );

  private:
    /**Target layer*/
    QgsVectorLayer* mLayer;
};


#endif // QgsJoinDIALOG_H
