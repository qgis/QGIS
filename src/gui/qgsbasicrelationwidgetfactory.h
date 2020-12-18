/***************************************************************************
                         qgsbasicrelationwidgetfactory.h
                         ----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBASICRELATIONWIDGETFACTORY_H
#define QGSBASICRELATIONWIDGETFACTORY_H

#include "qgsrelationwidgetfactory.h"
#include "qgsbasicrelationwidget.h"

#define SIP_NO_FILE

/**
 * Factory class for creating a basic relation widget and the respective config widget.
 * \ingroup gui
 * \class QgsBasicRelationWidgetFactory
 * \note not available in Python bindings
 * \since QGIS 3.18
 */
class GUI_EXPORT QgsBasicRelationWidgetFactory : public QgsRelationWidgetFactory
{
  public:
    QgsBasicRelationWidgetFactory();

    QString type() const override;

    QString name() const override;

    QgsRelationWidget *create( const QVariantMap &config, QWidget *parent = nullptr ) const override;

    QgsRelationConfigWidget *configWidget( const QgsRelation &relation, QWidget *parent ) const override;

};

#endif // QGSBASICRELATIONWIDGETFACTORY_H
