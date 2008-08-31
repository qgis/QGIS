// $Id$
//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2008 by Mateusz Loskot <mateusz@loskot.net>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License,
// or (at your option) any later version.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef QGIS_PLUGIN_OGRCONV_TRANSLATOR_H_INCLUDED
#define QGIS_PLUGIN_OGRCONV_TRANSLATOR_H_INCLUDED

// Qt4
#include <QString>

#include <ogr_api.h>

class Translator
{
  public:

    Translator();
    Translator( QString const& src, QString const& dst, QString const& format );

    QString const& targetFormat() const;
    void setTargetFormat( QString const& format );

    QString const& targetLayer() const;
    void setTargetLayer( QString const& layer );

    QString const& sourceLayer() const;
    void setSourceLayer( QString const& layer );

    QString const& targetReferenceSystem() const;
    void setTargetReferenceSystem( QString const& srs );

    QString const& sourceReferenceSystem() const;
    void setSourceReferenceSystem( QString const& srs );

    bool isTargetUpdate() const;
    void setUpdateTarget( bool update );

    bool isTargetLayerOverwrite() const;
    // TODO: Implement, currently always overwrite
    // void setTargetLayerOverwrite(bool overwrite);

    bool translate();

  private:

    QString mSrcUrl;
    QString mDstUrl;
    QString mDstFormat;
    QString mSrcLayer;
    QString mDstLayer;
    QString mSrcSrs;
    QString mDstSrs;
    bool mDstUpdate;
    bool mDstLayerOverwrite;
    // TODO: Append option not supported
    // bool mDstLayerAppend;

    bool translateLayer( OGRDataSourceH srcDs, OGRLayerH srcLayer, OGRDataSourceH dstDs );
    bool copyFields( OGRFeatureDefnH layerDefn, OGRLayerH layer );
    bool copyFeatures( OGRLayerH srcLayer, OGRLayerH dstLayer );

    OGRSFDriverH findDriver( QString const& name );
    OGRLayerH findLayer( OGRDataSourceH ds, QString const& name, int& index );
    OGRDataSourceH openDataSource( QString const& url, bool readOnly );
    OGRDataSourceH openDataTarget( QString const& url, bool update );

};

#endif // QGIS_PLUGIN_OGRCONV_TRANSLATOR_H_INCLUDED
