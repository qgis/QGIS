/***************************************************************************
 *   Copyright (C) 2004 by Lars Luthman                                    *
 *   larsl@users.sourceforge.net                                           *
 *                                                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef EXIF2GPX_H
#define EXIF2GPX_H

#include <map>
#include <utility>

#include <qdatetime.h>
#include <qdom.h>
#include <qstring.h>
#include <qstringlist.h>


class Exif2GPX {
 public:
  
  bool loadGPX(const QString& filename, bool useTracks, bool useWaypoints);
  bool writeGPX(const QStringList& pictures, const QString& gpxOutput,
		bool interpolate, unsigned offset, const QString& prefix);
  
 private:
  QDateTime getTimeFromEXIF(const QString& filename);
  std::pair<double, double> computePosition(QDateTime time,
					    bool interpolate);
  void addWaypoint(QDomElement& elt, std::pair<double,double> position,
		   QDateTime time, const QString& prefix, 
		   const QString& name);
  void loadPoint(const QDomNode& node);
  
  std::map<QDateTime, std::pair<double, double> > points;
};


#endif
