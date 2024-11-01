/***************************************************************************
    qgsaggregatetoolbutton.h
     --------------------------------------
    Date                 : Nov 2017
    Copyright            : (C) 2017 Matthias Kuhn
    Email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAGGREGATETOOLBUTTON_H
#define QGSAGGREGATETOOLBUTTON_H

#include <QToolButton>
#include <QVariant>

#include "qgsaggregatecalculator.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 *
 * \brief Offers a toolbutton to choose between different aggregate functions.
 * Functions are filtered based on the type.
 *
 */
class GUI_EXPORT QgsAggregateToolButton : public QToolButton
{
    Q_OBJECT

  public:
    /**
     * Constructor
     */
    QgsAggregateToolButton();

    /**
     * Based on the \a type of underlying data, some aggregates will be available or not.
     */
    void setType( QMetaType::Type type );

    /**
     * Based on the \a type of underlying data, some aggregates will be available or not.
     * \deprecated QGIS 3.38. Use the method with a QMetaType::Type argument instead.
     */
    Q_DECL_DEPRECATED void setType( QVariant::Type type ) SIP_DEPRECATED;

    /**
     * Based on the \a type of underlying data, some aggregates will be available or not.
     */
    QMetaType::Type type() const;

    /**
     * When this flag is FALSE, the aggregate will be deactivated. I.e. no aggregate is chosen.
     */
    void setActive( bool active );

    /**
     * When this flag is FALSE, the aggregate will be deactivated. I.e. no aggregate is chosen.
     */
    bool active() const;

    /**
     * The function name of the selected aggregate or a Null String if none is chosen.
     */
    QString aggregate() const;

    /**
     * The function name of the selected aggregate or a Null String if none is chosen.
     */
    void setAggregate( const QString &aggregate );

  signals:

    /**
     * The function name of the selected aggregate has changed.
     */
    void aggregateChanged();

    /**
     * A function has been selected or deselected.
     */
    void activeChanged();

  private slots:
    void aboutToShowMenu();

  private:
    void updateAvailableAggregates();
    QMenu *mMenu = nullptr;
    QMetaType::Type mType = QMetaType::Type::UnknownType;
    bool mActive = false;
    QString mAggregate;
    QList<QgsAggregateCalculator::AggregateInfo> mAvailableAggregates;
};

#endif // QGSAGGREGATETOOLBUTTON_H
