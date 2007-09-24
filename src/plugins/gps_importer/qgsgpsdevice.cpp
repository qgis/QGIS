/***************************************************************************
     qgsgpsdevice.cpp
     --------------------------------------
    Date                 : Sun Sep 16 12:04:15 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QRegExp>

#include "qgsgpsdevice.h"


QgsGPSDevice::QgsGPSDevice(const QString& wptDlCmd, const QString& wptUlCmd,
			   const QString& rteDlCmd, const QString& rteUlCmd,
			   const QString& trkDlCmd, const QString& trkUlCmd) {
  if (!wptDlCmd.isEmpty())
    mWptDlCmd = QStringList::split(QRegExp("\\s"), wptDlCmd); 
  if (!wptUlCmd.isEmpty())
    mWptUlCmd = QStringList::split(QRegExp("\\s"), wptUlCmd); 
  if (!rteDlCmd.isEmpty())
    mRteDlCmd = QStringList::split(QRegExp("\\s"), rteDlCmd); 
  if (!rteUlCmd.isEmpty())
    mRteUlCmd = QStringList::split(QRegExp("\\s"), rteUlCmd); 
  if (!trkDlCmd.isEmpty())
    mTrkDlCmd = QStringList::split(QRegExp("\\s"), trkDlCmd); 
  if (!trkUlCmd.isEmpty())
    mTrkUlCmd = QStringList::split(QRegExp("\\s"), trkUlCmd); 
}


QStringList QgsGPSDevice::importCommand(const QString& babel, 
					const QString& type,
					const QString& in, 
					const QString& out) const {
  const QStringList* original;
  if (type == "-w")
    original = &mWptDlCmd;
  else if (type == "-r")
    original = &mRteDlCmd;
  else if (type == "-t")
    original = &mTrkDlCmd;
  else throw "Bad error!";
  QStringList copy;
  QStringList::const_iterator iter;
  for (iter = original->begin(); iter != original->end(); ++iter) {
    if (*iter == "%babel")
      copy.append(babel);
    else if (*iter == "%type")
      copy.append(type);
    else if (*iter == "%in")
      copy.append(in);
    else if (*iter == "%out")
      copy.append(out);
    else
      copy.append(*iter);
  }
  return copy;
}


QStringList QgsGPSDevice::exportCommand(const QString& babel, 
					const QString& type,
					const QString& in, 
					const QString& out) const {
  const QStringList* original;
  if (type == "-w")
    original = &mWptUlCmd;
  else if (type == "-r")
    original = &mRteUlCmd;
  else if (type == "-t")
    original = &mTrkUlCmd;
  else throw "Bad error!";
  QStringList copy;
  QStringList::const_iterator iter;
  for (iter = original->begin(); iter != original->end(); ++iter) {
    if (*iter == "%babel")
      copy.append(babel);
    else if (*iter == "%type")
      copy.append(type);
    else if (*iter == "%in")
      copy.append(in);
    else if (*iter == "%out")
      copy.append(out);
    else
      copy.append(*iter);
  }
  return copy;
}



