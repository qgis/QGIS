/***************************************************************************
                          qgsgrassundocommand.h
                             -------------------
    begin                : November, 2015
    copyright            : (C) 2015 by Radim Blazek
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
#ifndef QGSGRASSUNDOCOMMAND_H
#define QGSGRASSUNDOCOMMAND_H

class QgsGrassProvider;

class GRASS_LIB_EXPORT QgsGrassUndoCommand
{
  public:
    virtual ~QgsGrassUndoCommand() {}
    virtual void undo() {}
};

// This class is used to store information that a new cat was attached to a line
// when attribute was changed.
class GRASS_LIB_EXPORT QgsGrassUndoCommandChangeAttribute : public QgsGrassUndoCommand
{
  public:
    QgsGrassUndoCommandChangeAttribute( QgsGrassProvider * provider, int fid, int lid, int field, int cat, bool deleteCat, bool deleteRecord );
    ~QgsGrassUndoCommandChangeAttribute() {}
    void undo() override;
  private:
    QgsGrassProvider *mProvider;
    int mFid;
    int mLid;
    int mField;
    int mCat;
    bool mDeleteCat;
    bool mDeleteRecord;
};

#endif // QGSGRASSUNDOCOMMAND_H
