/***************************************************************************
                         qgsdwgimporter.h
                         --------------
    begin                : May 2016
    copyright            : (C) 2016 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "drw_interface.h"

#include <QCoreApplication>
#include <QString>
#include <QElapsedTimer>

#include <ogr_api.h>

#include "qgsabstractgeometry.h"
#include "qgsogrutils.h"

class QgsCompoundCurve;
class QgsLineString;
class QgsCircularString;
class QgsQgsCoordinateReferenceSystem;
class QLabel;
class QTextCodec;

class QgsDwgImporter : public DRW_Interface
{
    Q_DECLARE_TR_FUNCTIONS( QgsDwgImporter )

  public:
    QgsDwgImporter( const QString &database, const QgsCoordinateReferenceSystem &crs );
    ~QgsDwgImporter() override;

    bool import( const QString &drawing, QString &error, bool expandInserts, bool useCurves, QLabel *label );

    //! Called when header is parsed.
    void addHeader( const DRW_Header *data ) override;

    //! Called for every line Type.
    void addLType( const DRW_LType &data ) override;

    //! Called for every layer.
    void addLayer( const DRW_Layer &data ) override;

    //! Called for every dim style.
    void addDimStyle( const DRW_Dimstyle &data ) override;

    //! Called for every VPORT table.
    void addVport( const DRW_Vport &data ) override;

    //! Called for every text style.
    void addTextStyle( const DRW_Textstyle &data ) override;

    //! Called for every AppId entry.
    void addAppId( const DRW_AppId &data ) override;

    void addBlock( const DRW_Block &data ) override;

    void setBlock( int handle ) override;

    //! Called to end the current block
    void endBlock() override;

    //! Called for every point
    void addPoint( const DRW_Point &data ) override;

    //! Called for every line
    void addLine( const DRW_Line &data ) override;

    //! Called for every ray
    void addRay( const DRW_Ray &data ) override;

    //! Called for every xline
    void addXline( const DRW_Xline &data ) override;

    //! Called for every arc
    void addArc( const DRW_Arc &data ) override;

    //! Called for every circle
    void addCircle( const DRW_Circle &data ) override;

    //! Called for every ellipse
    void addEllipse( const DRW_Ellipse &data ) override;

    //! Called for every lwpolyline
    void addLWPolyline( const DRW_LWPolyline &data ) override;

    //! Called for every polyline start
    void addPolyline( const DRW_Polyline &data ) override;

    //! Called for every spline
    void addSpline( const DRW_Spline *data ) override;

    //! Called for every spline knot value
    void addKnot( const DRW_Entity &data ) override;

    //! Called for every insert.
    void addInsert( const DRW_Insert &data ) override;

    //! Called for every trace start
    void addTrace( const DRW_Trace &data ) override;

    //! Called for every 3dface start
    void add3dFace( const DRW_3Dface &data ) override;

    //! Called for every solid start
    void addSolid( const DRW_Solid &data ) override;

    //! Called for every Multi Text entity.
    void addMText( const DRW_MText &data ) override;

    //! Called for every Text entity.
    void addText( const DRW_Text &data ) override;

    //! Called for every dimension entity.
    void addDim( const DRW_Dimension *data );

    //! Called for every aligned dimension entity.
    void addDimAlign( const DRW_DimAligned *data ) override;

    //! Called for every linear or rotated dimension entity.
    void addDimLinear( const DRW_DimLinear *data ) override;

    //! Called for every radial dimension entity.
    void addDimRadial( const DRW_DimRadial *data ) override;

    //! Called for every diametric dimension entity.
    void addDimDiametric( const DRW_DimDiametric *data ) override;

    //! Called for every angular dimension (2 lines version) entity.
    void addDimAngular( const DRW_DimAngular *data ) override;

    //! Called for every angular dimension (3 points version) entity.
    void addDimAngular3P( const DRW_DimAngular3p *data ) override;

    //! Called for every ordinate dimension entity.
    void addDimOrdinate( const DRW_DimOrdinate *data ) override;

    //! Called for every leader start.
    void addLeader( const DRW_Leader *data ) override;

    //! Called for every hatch entity.
    void addHatch( const DRW_Hatch *data ) override;

    //! Called for every viewport entity.
    void addViewport( const DRW_Viewport &data ) override;

    //! Called for every image entity.
    void addImage( const DRW_Image *data ) override;

    //! Called for every image definition.
    void linkImage( const DRW_ImageDef *data ) override;

    //! Called for every comment in the DXF file (code 999).
    void addComment( const char *comment ) override;

    void writeHeader( DRW_Header &data ) override;
    void writeBlocks() override;
    void writeBlockRecords() override;
    void writeEntities() override;
    void writeLTypes() override;
    void writeLayers() override;
    void writeTextstyles() override;
    void writeVports() override;
    void writeDimstyles() override;
    void writeAppId() override;

  private:
    void startTransaction();
    void commitTransaction();
    bool exec( const QString &sql, bool logError = true );
    OGRLayerH query( const QString &sql );

    void progress( const QString &msg );
    QString decode( const std::string &s ) const;
    void cleanText( QString &s );

    void addEntity( OGRFeatureDefnH dfn, OGRFeatureH f, const DRW_Entity &data );
    QString colorString( int color, int color24, int transparency, const QString &layer ) const;
    double lineWidth( int lWeight, const QString &layer ) const;
    QString linetypeString( const QString &linetype, const QString &layer ) const;
    void setString( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, const std::string &value ) const;
    void setString( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, const QString &value ) const;
    void setString( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, const char *value ) const;
    void setDouble( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, double value ) const;
    void setInteger( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, int value ) const;
    void setPoint( OGRFeatureDefnH dfn, OGRFeatureH f, const QString &field, const DRW_Coord &value ) const;

    bool curveFromLWPolyline( const DRW_LWPolyline &data, QgsCompoundCurve &cc );
    bool circularStringFromArc( const DRW_Arc &data, QgsCircularString &c );
    bool lineFromSpline( const DRW_Spline &data, QgsLineString &l );

    bool expandInserts( QString &error );
    bool expandInserts( QString &error, int block, QTransform base );

    bool createFeature( OGRLayerH layer, OGRFeatureH f, const QgsAbstractGeometry &g ) const;

    gdal::ogr_datasource_unique_ptr mDs;
    QString mDatabase;
    bool mInTransaction;
    int mSplineSegs;
    int mBlockHandle;
    int mCrs;
    OGRSpatialReferenceH mCrsH = nullptr;
    bool mUseCurves;

    QHash<QString, QString> mLayerColor;
    QHash<QString, double> mLayerLinewidth;
    QHash<QString, QString> mLayerLinetype;
    QHash<QString, QString> mLinetype;
    QHash<QString, int> mBlockNames;
    QHash<QString, QgsPointXY> mBlockBases;

    QLabel *mLabel = nullptr;
    int mEntities = 0;
    QTextCodec *mCodec = nullptr;
    QElapsedTimer mTime;
};
