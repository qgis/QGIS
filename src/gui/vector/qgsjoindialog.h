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

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgsjoindialogbase.h"
#include "qgis_gui.h"

class QgsVectorLayer;
class QgsVectorLayerJoinInfo;

/**
 * \ingroup gui
 * \class QgsJoinDialog
 */
class GUI_EXPORT QgsJoinDialog: public QDialog, private Ui::QgsJoinDialogBase
{
    Q_OBJECT
  public:
    QgsJoinDialog( QgsVectorLayer *layer, QList<QgsMapLayer *> alreadyJoinedLayers, QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags() );

    //! Configure the dialog for an existing join
    void setJoinInfo( const QgsVectorLayerJoinInfo &joinInfo );

    //! Returns the join info
    QgsVectorLayerJoinInfo joinInfo() const;

    //! Returns true if user wants to create an attribute index on the join field
    bool createAttributeIndex() const;

  private slots:
    void joinedLayerChanged( QgsMapLayer *layer );

    void checkDefinitionValid();

    void editableJoinLayerChanged();

  private:
    //! Target layer
    QgsVectorLayer *mLayer = nullptr;

    // Temporary storage for "cache" setting since the checkbox may be temporarily disabled
    bool mCacheEnabled = false;
};


#endif // QgsJoinDIALOG_H
