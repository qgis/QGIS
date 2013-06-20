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
#include "getInsertions.h"
#include <vector>

class Builder: public DL_CreationAdapter
{
  public:
    Builder( std::string theFname,
             int theShapefileType,
             double *theGrpXVals, double *theGrpYVals,
             std::string *theGrpNames,
             int theInsertCount,
             bool theConvertText );
    ~Builder();

    void FinalizeAnyPolyline();

    virtual void addLayer( const DL_LayerData &data );
    virtual void addPoint( const DL_PointData &data );
    virtual void addLine( const DL_LineData &data );
    virtual void addPolyline( const DL_PolylineData &data );
    virtual void addArc( const DL_ArcData &data );
    virtual void addCircle( const DL_CircleData &data );
    virtual void addVertex( const DL_VertexData &data );
    virtual void addBlock( const DL_BlockData &data );
    virtual void endBlock();
    virtual void endSequence();
    virtual void addText( const DL_TextData &data );

    void print_shpObjects();

    int textObjectsSize();
    std::string outputShp();
    std::string outputTShp();

  private:
    std::string fname;
    int shapefileType; // SHPT_POINT, ...
    double *grpXVals;
    double *grpYVals;
    std::string *grpNames;
    int insertCount;
    bool convertText;

    std::string outputdbf;
    std::string outputshp;
    std::string outputtdbf;
    std::string outputtshp;

    std::vector <DL_VertexData> polyVertex;
    std::vector <SHPObject *> shpObjects; // all read objects are stored here
    std::vector <DL_TextData> textObjects;

    int fetchedprims;
    int fetchedtexts;

    bool ignoringBlock;
    bool current_polyline_willclose;
    bool store_next_vertex_for_polyline_close;

    long current_polyline_pointcount;

    double closePolyX, closePolyY, closePolyZ;
    double currentBlockX, currentBlockY;
};
