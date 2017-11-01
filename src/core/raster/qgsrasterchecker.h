/***************************************************************************
     qgsrasterchecker.h - compare two rasters
                     --------------------------------------
               Date                 : 5 Sep 2012
               Copyright            : (C) 2012 by Radim Blazek
               email                : radim dot blazek at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERCHECKER_H
#define QGSRASTERCHECKER_H

#include "qgis_core.h"
#include <QDir>
#include <QString>
class QImage;

/**
 * \ingroup core
 * This is a helper class for unit tests that need to
 * write an image and compare it to an expected result
 * or render time.
 */
class CORE_EXPORT QgsRasterChecker
{
  public:

    QgsRasterChecker();

    QString report() { return mReport; }

    /**
     * Test using renderer to generate the image to be compared.
     * \param verifiedKey verified provider key
     * \param verifiedUri URI of the raster to be verified
     * \param expectedKey expected provider key
     * \param expectedUri URI of the expected (control) raster
     */
    bool runTest( const QString &verifiedKey, QString verifiedUri,
                  const QString &expectedKey, QString expectedUri );
  private:
    QString mReport;
    QString mExpectedUri;
    QString mCheckedUri;
    QString mTabStyle;
    QString mCellStyle;
    QString mOkStyle;
    QString mErrStyle;
    QString mErrMsgStyle;

    // Log error in html
    void error( const QString &message, QString &report );
    // compare values and add table row in html report, set ok to false if not equal
    QString compareHead();
    bool compare( double verifiedVal, double expectedVal, double tolerance );
    void compare( const QString &paramName, int verifiedVal, int expectedVal, QString &report, bool &ok );
    void compare( const QString &paramName, double verifiedVal, double expectedVal, QString &report, bool &ok, double tolerance = 0 );
    void compareRow( const QString &paramName, const QString &verifiedVal, const QString &expectedVal, QString &report, bool ok, const QString &difference = QString(), const QString &tolerance = QString() );
    double tolerance( double val, int places = 6 );
}; // class QgsRasterChecker

// clazy:excludeall=qstring-allocations

#endif
