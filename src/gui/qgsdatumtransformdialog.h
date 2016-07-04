/***************************************************************************
                         qgsdatumtransformdialog.h
                         -------------------------
    begin                : November 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco.hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATUMTRANSFORMDIALOG_H
#define QGSDATUMTRANSFORMDIALOG_H

#include "ui_qgsdatumtransformdialogbase.h"

/** \ingroup gui
 * \class QgsDatumTransformDialog
 * \note not available in Python bindings
 */
class GUI_EXPORT QgsDatumTransformDialog : public QDialog, private Ui::QgsDatumTransformDialogBase
{
    Q_OBJECT
  public:
    QgsDatumTransformDialog( const QString& layerName, const QList< QList< int > >& dt, QWidget * parent = nullptr, const Qt::WindowFlags& f = nullptr );
    ~QgsDatumTransformDialog();

    //! @note added in 2.4
    void setDatumTransformInfo( const QString& srcCRSauthId, const QString& destCRSauthId );

    //! getter for selected datum tranformations
    QList< int > selectedDatumTransform();

    //! dialog shall remember the selection
    bool rememberSelection() const;

  public slots:
    void on_mHideDeprecatedCheckBox_stateChanged( int state );
    void on_mDatumTransformTreeWidget_currentItemChanged( QTreeWidgetItem *, QTreeWidgetItem * );

  private:
    QgsDatumTransformDialog();
    void updateTitle();
    bool gridShiftTransformation( const QString& itemText ) const;
    /** Returns false if the location of the grid shift files is known (PROJ_LIB) and the shift file is not there*/
    bool testGridShiftFileAvailability( QTreeWidgetItem* item, int col ) const;
    void load();

    const QList< QList< int > > &mDt;
    QString mLayerName;
    QString mSrcCRSauthId, mDestCRSauthId;
};

#endif // QGSDATUMTRANSFORMDIALOG_H
