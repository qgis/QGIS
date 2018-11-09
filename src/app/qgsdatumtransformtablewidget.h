/***************************************************************************
    qgsdatumtransformtablewidget.h
     --------------------------------------
    Date                 : 28.11.2017
    Copyright            : (C) 2017 Denis Rouzaud
    Email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDATUMTRANSFORMTABLEWIDGET_H
#define QGSDATUMTRANSFORMTABLEWIDGET_H

#include <QAbstractTableModel>
#include <QWidget>

#include "ui_qgsdatumtransformtablewidgetbase.h"
#include "qgis_app.h"
#include "qgscoordinatetransformcontext.h"


/**
 * The QgsDatumTransformTableModel class is a table model to display and edit
 * datum transformations.
 *
 * \since 3.0
 */
class APP_EXPORT QgsDatumTransformTableModel : public QAbstractTableModel
{
    Q_OBJECT
  public:

    enum TableColumns
    {
      SourceCrsColumn  = 0,
      SourceTransformColumn,
      DestinationCrsColumn,
      DestinationTransformColumn,
    };

    QgsDatumTransformTableModel( QObject *parent = nullptr );

    void setTransformContext( const QgsCoordinateTransformContext &context );

    QgsCoordinateTransformContext transformContext() const
    {
      return mTransformContext;
    }

    /**
     * remove the transformation at given indexes
     */
    void removeTransform( const QModelIndexList &indexes );

    int rowCount( const QModelIndex &parent ) const override;
    int columnCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;
    QVariant headerData( int section, Qt::Orientation orientation, int role ) const override;

  private:

    QgsCoordinateTransformContext mTransformContext;
};


class APP_EXPORT QgsDatumTransformTableWidget : public QWidget, private Ui::QgsDatumTransformTableWidgetBase
{
    Q_OBJECT

  public:
    explicit QgsDatumTransformTableWidget( QWidget *parent = nullptr );

    void setTransformContext( const QgsCoordinateTransformContext &context )
    {
      mModel->setTransformContext( context );
    }

    QgsCoordinateTransformContext transformContext() const
    {
      return mModel->transformContext();
    }

  public slots:
    //! add a new datum transform
    void addDatumTransform();

    //! remove currently selected datum transform
    void removeDatumTransform();

    //! edit currently selected datum transform
    void editDatumTransform();

  private:
    QgsDatumTransformTableModel *mModel = new QgsDatumTransformTableModel( this );
};


#endif // QGSDATUMTRANSFORMTABLEWIDGET_H
