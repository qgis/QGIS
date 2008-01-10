/***************************************************************************
                          qgsprojectfile.h  -  description
                             -------------------
    begin                : Sun 15 dec 2007
    copyright            : (C) 2007 by Magnus Homann
    email                : magnus at homann.se
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROJECTVERSION_H
#define QGSPROJECTVERSION_H

#include <QString>
#include <QStringList>

class QgsProjectVersion
{

 public:

  QgsProjectVersion() {} 

  QgsProjectVersion(int major, int minor, int sub, QString name = "");

  QgsProjectVersion(QString string);

  ~QgsProjectVersion() {} 

  int major() { return mMajor;};
  int minor() { return mMinor;};
  int sub()   { return mSub;};

  QString text();

  /*! Boolean equal operator
   */
  bool operator==(const QgsProjectVersion &other);


  /*! Boolean >= operator
   */
  bool operator>=(const QgsProjectVersion &other);

  /*! Boolean > operator
   */
  bool operator>(const QgsProjectVersion &other);

 private:
  int mMajor;
  int mMinor;
  int mSub;
  QString mName;
};

#endif // QGSPROJECTVERSION_H
