/***************************************************************************
    qgssensormodel.h
    ---------------
    begin                : March 2023
    copyright            : (C) 2023 by Mathieu pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSENSORMODEL_H
#define QGSSENSORMODEL_H

#include "qgis_core.h"
#include "qgis_sip.h"

#include <QAbstractItemModel>

class QgsSensorManager;

/**
 * \ingroup core
 * \class QgsSensorModel
 * \brief A QAbstractItemModel subclass for showing sensors within a QgsSensorManager.
 * \since QGIS 3.32
 */
class CORE_EXPORT QgsSensorModel: public QAbstractItemModel
{
    Q_OBJECT

  public:

    //! Model columns
    enum class Column : int
    {
      Name = 0, //!< Name
      LastValue = 1, //!< Last value
    };

    // *INDENT-OFF*

    /**
     * Custom model roles.
     *
     * \note Prior to QGIS 3.36 this was available as QgsSensorModel::Role
     * \since QGIS 3.36
     */
    enum class CustomRole SIP_MONKEYPATCH_SCOPEENUM_UNNEST( QgsSensorModel, Role ) : int
    {
      SensorType = Qt::UserRole + 1, //!< Sensor type
      SensorId, //!< Sensor id
      SensorName, //!< Sensor name
      SensorStatus, //!< Sensor status (disconnected, connected, etc.)
      SensorLastValue, //!< Sensor last captured value
      SensorLastTimestamp, //!< Sensor timestamp of last captured value
      Sensor, //!< Sensor object pointer
    };
    Q_ENUM( CustomRole )
    // *INDENT-ON*

    /**
     * Constructor for QgsSensorModel, for the specified \a manager and \a parent object.
     */
    explicit QgsSensorModel( QgsSensorManager *manager, QObject *parent SIP_TRANSFERTHIS = nullptr );

    QVariant data( const QModelIndex &index, int role ) const override;
    bool setData( const QModelIndex &index, const QVariant &value, int role = Qt::EditRole ) override;
    Qt::ItemFlags flags( const QModelIndex &index ) const override;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const override;
    QModelIndex index( int row, int column,
                       const QModelIndex &parent = QModelIndex() ) const override;
    QModelIndex parent( const QModelIndex &index ) const override;
    int rowCount( const QModelIndex &parent = QModelIndex() ) const override;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const override;

  private slots:

    void sensorAdded( const QString &id );
    void sensorRemoved( const QString &id );
    void sensorNameChanged( const QString &id );
    void sensorStatusChanged( const QString &id );
    void sensorDataCaptured( const QString &id );

  private:

    QgsSensorManager *mSensorManager = nullptr;
    QStringList mSensorIds;
};

#endif //QGSSENSORMODEL_H
