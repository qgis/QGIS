// The code is heavily based on Christopher Michaelis'  DXF to Shapefile Converter
// (http://www.wanderingidea.com/content/view/12/25/), released under GPL License
//
// This code is based on two other products:
// DXFLIB (http://www.ribbonsoft.com/dxflib.html)
//    This is a library for reading DXF files, also GPL.
// SHAPELIB (http://shapelib.maptools.org/)
//    Used for the Shapefile functionality.
//    It is Copyright (c) 1999, Frank Warmerdam, released under the following "MIT Style" license:
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
//documentation files (the "Software"), to deal in the Software without restriction, including without limitation
//the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
//and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
//OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
//IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#include "dxflib/src/dl_creationadapter.h"
#include "shapelib-1.2.10/shapefil.h"

#include <QList>
#include <QString>


class Builder: public DL_CreationAdapter
{
  public:
    Builder( QString theFname, int theShapefileType, bool theConvertText, bool theConvertInserts );
    ~Builder();

    void FinalizeAnyPolyline();

    virtual void addLayer( const DL_LayerData &data ) override;
    virtual void addPoint( const DL_PointData &data ) override;
    virtual void addLine( const DL_LineData &data ) override;
    virtual void addInsert( const DL_InsertData &data ) override;
    virtual void addPolyline( const DL_PolylineData &data ) override;
    virtual void addArc( const DL_ArcData &data ) override;
    virtual void addCircle( const DL_CircleData &data ) override;
    virtual void addVertex( const DL_VertexData &data ) override;
    virtual void addBlock( const DL_BlockData &data ) override;
    virtual void endBlock() override;
    virtual void endSequence() override;
    virtual void addText( const DL_TextData &data ) override;

    void print_shpObjects();

    int textObjectsSize() const { return textObjects.size(); }
    int insertObjectsSize() const { return insertObjects.size(); }
    QString outputShp() const { return outputshp; }
    QString outputTShp() const { return outputtshp; }
    QString outputIShp() const { return outputishp; }

  private:
    QString fname;
    int shapefileType; // SHPT_POINT, ...
    bool convertText;
    bool convertInserts;

    QString outputdbf;
    QString outputshp;

    QString outputtdbf;
    QString outputtshp;

    QString outputidbf;
    QString outputishp;

    QList<SHPObject *> shpObjects; // all read objects are stored here

    QList<DL_VertexData> polyVertex;
    QList<DL_TextData> textObjects;
    QList<DL_InsertData> insertObjects;

    bool ignoringBlock;
    bool current_polyline_willclose;
    bool store_next_vertex_for_polyline_close;

    long current_polyline_pointcount;

    double closePolyX, closePolyY, closePolyZ;
};
