/***************************************************************************
  qgsspeedarcproperter.h
  --------------------------------------
  Date                 : 2011-04-01
  Copyright            : (C) 2010 by Yakushev Sergey
  Email                : YakushevS <at> list.ru
****************************************************************************
*                                                                          *
*   This program is free software; you can redistribute it and/or modify   *
*   it under the terms of the GNU General Public License as published by   *
*   the Free Software Foundation; either version 2 of the License, or      *
*   (at your option) any later version.                                    *
*                                                                          *
***************************************************************************/

#ifndef QGSSPEEDARCPROPERTER_H
#define QGSSPEEDARCPROPERTER_H

#include <qgsarcproperter.h>

/** \ingroup analysis
 * \class QgsSpeedArcProperter
 * \note added in QGIS 3.0
 *
 * \brief Used for calculating arc property taking into account travel time.
 */
class ANALYSIS_EXPORT QgsSpeedArcProperter : public QgsArcProperter
{
  public:

    /**
     * Constructor for QgsSpeedArcProperter.
     */
    QgsSpeedArcProperter( int attributeId, double defaultValue, double toMetricFactor );

    //! Returns caluclated edge property
    QVariant property( double distance, const QgsFeature& f ) const override;

    /** QgsGraphDirector will call this method for fetching attributes
     * needed to calculate arc properties from the source layer
     */
    QgsAttributeList requiredAttributes() const override;

  private:
    int mAttributeId;
    double mDefaultValue;
    double mToMetricFactor;

};

#endif // QGSSPEEDARCPROPERTER_H
