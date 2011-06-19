/***************************************************************************
    qgisexpressionbuilder.h - A genric expression string builder widget.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2006 by Nathan Woodrow
    Email                : nathan.woodrow at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONBUILDER_H
#define QGSEXPRESSIONBUILDER_H

#include <QWidget>
#include "ui_qgsexpressionbuilder.h"

class QgsExpressionBuilder : public QWidget, private Ui::QgsExpressionBuilder {
    Q_OBJECT
public:
    QgsExpressionBuilder(QWidget *parent = 0);
    ~QgsExpressionBuilder();

    QString getExpressionString();
    void setExpressionString(const QString expressionString);
};

#endif // QGSEXPRESSIONBUILDER_H
