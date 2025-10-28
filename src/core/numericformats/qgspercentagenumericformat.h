/***************************************************************************
                             qgspercentagenumericformat.h
                             ----------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPERCENTAGENUMERICFORMAT_H
#define QGSPERCENTAGENUMERICFORMAT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsbasicnumericformat.h"

/**
 * \ingroup core
 * \brief A numeric formatter which returns a text representation of a percentage value.
 *
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsPercentageNumericFormat : public QgsBasicNumericFormat
{
  public:

    //! Input value format, which specifies the format of the incoming values
    enum InputValues
    {
      ValuesArePercentage, //!< Incoming values are percentage values (e.g. 50 for 50%)
      ValuesAreFractions, //!< Incoming values are numeric fractions (e.g. 0.5 for 50%)
    };

    /**
      * Default constructor
      */
    QgsPercentageNumericFormat();

    QString id() const override;
    QString visibleName() const override;
    int sortKey() override;
    double suggestSampleValue() const override;
    QString formatDouble( double value, const QgsNumericFormatContext &context ) const override;
    QgsNumericFormat *clone() const override SIP_FACTORY;
    QgsNumericFormat *create( const QVariantMap &configuration, const QgsReadWriteContext &context ) const override SIP_FACTORY;
    QVariantMap configuration( const QgsReadWriteContext &context ) const override;

    /**
     * Returns the format of the incoming values.
     *
     * \see setInputValues()
     */
    InputValues inputValues() const;

    /**
     * Sets the \a format of the incoming values.
     *
     * \see inputValues()
     */
    void setInputValues( InputValues format );

  private:

    InputValues mInputValues = ValuesArePercentage;

};

#endif // QGSPERCENTAGENUMERICFORMAT_H
