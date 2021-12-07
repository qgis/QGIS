/***************************************************************************
  qgsrasterattributetable.h - QgsRasterAttributeTable

 ---------------------
 begin                : 3.12.2021
 copyright            : (C) 2021 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRASTERATTRIBUTETABLE_H
#define QGSRASTERATTRIBUTETABLE_H

#include "qgsfields.h"
#include "qgis_core.h"
#include "gdal.h"

#include <QObject>

class CORE_EXPORT QgsRasterAttributeTable
{

  public:


    QgsRasterAttributeTable();

    enum class FieldUsage : int
    {
      Generic = GFU_Generic, //!< Field usage Generic
      PixelCount = GFU_PixelCount, //!< Field usage PixelCount
      Name = GFU_Name, //!< Field usage Name
      Min = GFU_Min, //!< Field usage Min
      Max = GFU_Max, //!< Field usage Max
      MinMax = GFU_MinMax, //!< Field usage MinMax
      Red = GFU_Red, //!< Field usage Red
      Green = GFU_Green, //!< Field usage Green
      Blue = GFU_Blue, //!< Field usage Blue
      Alpha = GFU_Alpha, //!< Field usage Alpha
      RedMin = GFU_RedMin, //!< Field usage RedMin
      GreenMin = GFU_GreenMin, //!< Field usage GreenMin
      BlueMin = GFU_BlueMin, //!< Field usage BlueMin
      AlphaMin = GFU_AlphaMin, //!< Field usage AlphaMin
      RedMax = GFU_RedMax, //!< Field usage RedMax
      GreenMax = GFU_GreenMax, //!< Field usage GreenMax
      BlueMax = GFU_BlueMax, //!< Field usage BlueMax
      AlphaMax = GFU_AlphaMax, //!< Field usage AlphaMax
      MaxCount = GFU_MaxCount //!< Field usage MaxCount
    };

    enum class RatType : int
    {
      Thematic = GRTT_THEMATIC,
      Athematic = GRTT_ATHEMATIC
    };

    const RatType &type() const;
    void setType( const RatType &newType );
    bool hasColor();
    QgsFields fields();

    const QList<QgsRasterAttributeTable::FieldUsage> &fieldUsages() const;

    bool isDirty() const;
    void setIsDirty( bool newIsDirty );

    bool insertField( const QString &name, QgsRasterAttributeTable::FieldUsage usage, QVariant::Type type, int position = 0 );

    bool appendField( const QString &name, QgsRasterAttributeTable::FieldUsage usage, QVariant::Type type );

    bool insertRow( const QVariantList data, int position = 0 );
    bool appendRow( const QVariantList data );

    bool isValid();

  protected:

    void setFieldUsages( const QList<QgsRasterAttributeTable::FieldUsage> &newFieldUsages );

  private:

    RatType mType;
    QgsFields mFields;
    QVariantList mData;
    QList<FieldUsage> mFieldUsages;
    bool mIsDirty;

};

#endif // QGSRASTERATTRIBUTETABLE_H
