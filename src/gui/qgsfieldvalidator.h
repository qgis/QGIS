/***************************************************************************
                         qgsfieldvalidator.h  -  description
                             -------------------
    begin                : March 2011
    copyright            : (C) 2011 by SunilRajKiran-kCube
    email                : sunilraj.kiran@kcubeconsulting.com

  adapted version of QValidator for QgsField
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFIELDVALIDATOR_H
#define QGSFIELDVALIDATOR_H

#include <QValidator>
#include <QVariant>
#include <QSettings>
#include "qgsfields.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsFieldValidator
 */
class GUI_EXPORT QgsFieldValidator : public QValidator
{
    Q_OBJECT

  public:
    QgsFieldValidator( QObject *parent, const QgsField &field, const QString &defaultValue, const QString &dateFormat = "yyyy-MM-dd" );
    ~QgsFieldValidator() override;

    State validate( QString &s SIP_CONSTRAINED SIP_INOUT, int &i SIP_INOUT ) const override;
    void fixup( QString &s SIP_CONSTRAINED ) const override;

    QString dateFormat() const { return mDateFormat; }

  private:
    // Disables copy constructing
    Q_DISABLE_COPY( QgsFieldValidator )

    QValidator *mValidator = nullptr;
    QgsField mField;
    QString mNullValue;
    QString mDefaultValue;
    QString mDateFormat;
};

// clazy:excludeall=qstring-allocations

#endif // QGSFIELDVALIDATOR_H
