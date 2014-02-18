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

#include <QDir>
#include <QString>
#include <qgsmaprenderer.h>
class QImage;

/** \ingroup UnitTests
 * This is a helper class for unit tests that need to
 * write an image and compare it to an expected result
 * or render time.
 */
class CORE_EXPORT QgsRasterChecker
{
  public:

    QgsRasterChecker();

    //! Destructor
    ~QgsRasterChecker() {}

    QString report() { return mReport; }
    /**
     * Test using renderer to generate the image to be compared.
     * @param theVerifiedKey verified provider key
     * @param theVerifiedUri URI of the raster to be verified
     * @param theExpectedKey expected provider key
     * @param theExpectedUri URI of the expected (control) raster
     */
    bool runTest( QString theVerifiedKey, QString theVerifiedUri,
                  QString theExpectedKey, QString theExpectedUri );
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
    void error( QString theMessage, QString &theReport );
    // compare values and add table row in html report, set ok to false if not equal
    QString compareHead();
    bool compare( double verifiedVal, double expectedVal, double theTolerance );
    void compare( QString theParamName, int verifiedVal, int expectedVal, QString &theReport, bool &theOk );
    void compare( QString theParamName, double verifiedVal, double expectedVal, QString &theReport, bool &theOk, double theTolerance = 0 );
    void compareRow( QString theParamName, QString verifiedVal, QString expectedVal, QString &theReport, bool theOk, QString theDifference = "", QString theTolerance = "" );
    double tolerance( double val, int places = 6 );
}; // class QgsRasterChecker

#endif
