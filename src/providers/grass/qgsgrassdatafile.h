/***************************************************************************
                            qgsgrassdatafile.h
                             -------------------
    begin                : June, 2015
    copyright            : (C) 2015 Radim Blazek
    email                : radim.blazek@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRASSDATAFILE_H
#define QGSGRASSDATAFILE_H

#include <QFile>

/*
 * This class does blocking reading which is necessary for QgsDataFile.
 * It reimplements QFile::readData to always read requested size of data.
 * I found the blocking readData in combination with QgsGrassDataFile opened
 * in QIODevice::Unbuffered mode the only way to get communication between
 * QgsGrassVectorImport and qgis.v.in module working on Linux.
 * Note that it may seem to work OK even without QgsGrassDataFile and Unbuffered
 * but then some imports randomply fails, so be careful and test well all changes.
 *
 * On Windows it _seemed_ to work even without QgsDataFile.
 *
 */
class GRASS_LIB_EXPORT QgsGrassDataFile : public QFile
{
  public:
    explicit QgsGrassDataFile( QObject *parent = 0 );
    virtual ~QgsGrassDataFile() {};
    // We need FILE* to be able to test feof but QFile::open(FILE *, OpenMode) is not virtual
    bool open( FILE * fh );
    // Block until all data are read
    virtual qint64 readData( char * data, qint64 len ) override;

  private:
    FILE *mFh;
};

#endif // QGSGRASSDATAFILE_H
