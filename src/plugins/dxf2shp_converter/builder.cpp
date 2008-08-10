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

#include <string>
#include <iostream>
#include <fstream>
#include "builder.h"

#define DEBUG 1

Builder::Builder() {
  currentBlockX = 0.0;
  currentBlockY = 0.0;
  ignoringBlock = false;
  current_polyline_pointcount = 0;
}

Builder::~Builder(){

  if(polyVertex.size() != 0)
    polyVertex.clear();  

  if(shpObjects.size() != 0) 
    shpObjects.clear();

  if(textObjects.size() != 0)
    textObjects.clear();    

  logfile.close();    

}

void Builder::initBuilder(string outf, int stype, double* grps_XVals, double* grps_YVals, string* grps_Names, int in_insertCount, bool getTexts) {
  shptype = stype;  
  fname = outf;

  logfname = fname;
  logfname = logfname.replace((logfname.length()-3),logfname.length(), "log");
  logfile.open (logfname.c_str());

  insertCount = in_insertCount;    
  awaiting_polyline_vertices = 0;
  fetchedprims = 0;
  fetchedtexts = 0;
  grpXVals = grps_XVals;
  grpYVals = grps_YVals;
  grpNames = grps_Names;
  convertText = getTexts;

  switch (shptype) {
    case 1:
      shapefileType = SHPT_POINT;
      break;
    case 0:
      shapefileType = SHPT_ARC;
      break;
    case 2:
      shapefileType = SHPT_POLYGON;
      break;
    default:
      shapefileType = SHPT_NULL;
      break;
  }    
}



void Builder::addBlock(const DL_BlockData& data)
{

  logfile << "Block added\n";
  if (data.name.compare("ADCADD_ZZ") == 0)
  {
    logfile << "(Ignoring Block "<< data.name.c_str()<<")\n";
    ignoringBlock = true;
  }   
  else 
    for (int i = 0; i < insertCount; i++)
    {
      if (strcmp(grpNames[i].c_str(), data.name.c_str()) == 0)
      {
        currentBlockX = grpXVals[i];
        currentBlockY = grpYVals[i];
        logfile << "Found Coord for Block: ("<< grpXVals[i]<< "," << grpYVals[i]<<")\n";
      }
    }
}

void Builder::endBlock()
{

  FinalizeAnyPolyline();

  currentBlockX = 0.0;
  currentBlockY = 0.0;
  ignoringBlock = false;

  logfile << "endBlock added\n";
}

void Builder::addLayer(const DL_LayerData& data) {
  logfile << "Layer: " << data.name.c_str() << "\n";
}

void Builder::addPoint(const DL_PointData& data) {
  if(shptype != 1) {
    logfile << " Ignoring Point \n";
    return;
  }

  logfile << "(Add Point (" << data.x << "," << data.y << "," << data.z << ")\n";

  if (ignoringBlock)
  {
    logfile << "ADCADD_ZZ block; ignoring. Exiting last add function.\n";
    return;
  }


  double  x = data.x + currentBlockX;
  double  y = data.y + currentBlockY;
  double z = data.z;

  SHPObject	*psObject;
  psObject = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL,
      1, &x, &y, &z, NULL  ); 

  shpObjects.push_back(psObject);  

  fetchedprims++;
}

void Builder::addLine(const DL_LineData& data) {

  //data.x1 , data.y1, data.z1 
  //data.x2 , data.y2, data.z2 

  if (shptype != 0)
  {
    logfile << "(Add Line - Ignoring [not creating polyline shapefile])\n";
    return;
  }

  logfile << "(AddLine (" << data.x1 << "," << data.y1 << "," << data.z1 << " to "
    << data.x2 << ","  << data.y2 << "," << data.z2 <<")\n";

  if (ignoringBlock)
  {
    logfile << "ADCADD_ZZ block; ignoring. Exiting last add function.\n";
    return;
  }


  double xv[2], yv[2], zv[2];
  xv[0] = data.x1 + currentBlockX;
  yv[0] = data.y1 + currentBlockY;
  zv[0] = data.z1;

  xv[1] = data.x2 + currentBlockX;
  yv[1] = data.y2 + currentBlockY;
  zv[1] = data.z2;   

  SHPObject	*psObject;

  psObject = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL,
      2, xv, yv, zv, NULL  ); 

  shpObjects.push_back(psObject);          

  fetchedprims++;
}



  void Builder::addPolyline(const DL_PolylineData& data) {
    if (shptype != 0 && shptype != 2)
    {
      logfile << "(Add Polyline - Ignoring [not creating polyline shapefile or polygon shapefile])\n";
      return;
    }

    logfile << "(Add Polyline) Awaiting Vertices...\n";

    if (ignoringBlock)
    {
      logfile << "ADCADD_ZZ block; ignoring. Exiting last add function.\n";
      return;
    }

    // Add previously created polyline if not finalized yet
    if (current_polyline_pointcount > 0)
    {
      if (current_polyline_willclose)
      {

        DL_VertexData myVertex;

        myVertex.x = closePolyX; 
        myVertex.y = closePolyY;
        myVertex.z = closePolyZ;

        polyVertex.push_back(myVertex);  

      }


      SHPObject	*psShape;
      int dim = polyVertex.size();
      double *xv = new double[dim];
      double *yv = new double[dim];
      double *zv = new double[dim];

      for (int i=0; i < dim; i++) 
      {
        xv[i] = polyVertex[i].x;
        yv[i] = polyVertex[i].y;
        zv[i] = polyVertex[i].z;
      }

      psShape = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, dim, xv, yv, zv, NULL  );

      delete [] xv;
      delete [] yv;
      delete [] zv;

      shpObjects.push_back(psShape);


      fetchedprims++;

      polyVertex.clear();

      logfile <<  "Finalized adding of polyline shape " << fetchedprims - 1 << "\n";
      current_polyline_pointcount = 0;
    }

    // Now that the currently-adding polyline (if any) is
    // finalized, parse out the flag options

    if (data.flags == 1 || data.flags == 32)
    {
      current_polyline_willclose = true;
      store_next_vertex_for_polyline_close = true;
    }
    else
    {
      current_polyline_willclose = false;
      store_next_vertex_for_polyline_close = false;
    }

    current_polyline_pointcount = 0;

  }


  void Builder::addVertex(const DL_VertexData& data)  {
    if (shptype != 0 && shptype != 2)
    {
      logfile << "(Add Vertex - Ignoring [not creating polyline shapefile or polygon shapefile])\n";
      return;
    }

    logfile << "(Add Vertex) (" << data.x << "," << data.y << "," << data.z << ")\n";

    if (ignoringBlock)
    {
      logfile << "ADCADD_ZZ block; ignoring. Exiting last add function.\n";
      return;
    }

    DL_VertexData myVertex;
    myVertex.x = data.x + currentBlockX;
    myVertex.y = data.y + currentBlockY;
    myVertex.z = data.z;

    polyVertex.push_back(myVertex);

    current_polyline_pointcount++;		

    if (store_next_vertex_for_polyline_close)
    {
      store_next_vertex_for_polyline_close = false;
      closePolyX = data.x + currentBlockX;
      closePolyY = data.y + currentBlockY;
      closePolyZ = data.z;
    }
  }


void Builder::endSequence() {
  logfile << "endSequence\n";
}

void Builder::addArc(const DL_ArcData& data)
{
  if (shptype != 0)
  {
    logfile << "(Add Arc - Ignoring [not creating polyline shapefile])\n";
    return;
  }

  double pi = 3.1415926535897932384626433832795;

  int fromAngle = (int) data.angle1 + 1;
  int toAngle = (int) data.angle2 + 1;

  logfile << "(Add Arc (" << data.cx << "," << data.cy << "," << data.cz << ","
    << data.radius << "," << data.angle1 << ","  << data.angle2 << ")\n";

  if (ignoringBlock)
  {
    logfile << "ADCADD_ZZ block; ignoring. Exiting last add function.\n";
    return;
  }

  register int i = 0;
  register long shpIndex = 0;

  // Approximate the arc

  register double radianMeasure;
  register double deg2radFactor = (pi / 180);

  vector <DL_PointData> arcPoints;
  DL_PointData myPoint;

  for (i = fromAngle; ; i += 1, shpIndex++)
  {
    if (i > 360)
      i = 0;

    if (shpIndex > 1000) break;

    radianMeasure = i * deg2radFactor;

    myPoint.x = data.radius * cos(radianMeasure) + data.cx + currentBlockX;
    myPoint.y = data.radius * sin(radianMeasure) + data.cy + currentBlockY;
    myPoint.z = data.cz;

    arcPoints.push_back(myPoint);

    if (i == toAngle) break;
  }

  // Finalize

  SHPObject	*psShape;
  int dim = arcPoints.size();
  double *xv = new double[dim];
  double *yv = new double[dim];
  double *zv = new double[dim];

  for (int i=0; i < dim; i++) 
  {
    xv[i] = arcPoints[i].x;
    yv[i] = arcPoints[i].y;
    zv[i] = arcPoints[i].z;

  }

  psShape = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, dim, xv, yv, zv, NULL  );

  delete [] xv;
  delete [] yv;
  delete [] zv;

  shpObjects.push_back(psShape);

  fetchedprims++;

  arcPoints.clear();

}


void Builder::addCircle(const DL_CircleData& data)
{
  if (shptype != 0 && shptype != 2)
  {
    logfile << "(Add Circle - Ignoring [not creating polyline shapefile or polygon shapefile])\n";
    return;
  }

  logfile << "(Add Circle  (%6.3f, %6.3f, %6.3f) %6.3f\n", data.cx, data.cy, data.cz, data.radius;

  if (ignoringBlock)
  {
    logfile << "ADCADD_ZZ block; ignoring. Exiting last add function.\n";
    return;
  }


  register double i = 0;
  register long shpIndex = 0;

  // Approximate the circle with 360 line segments connecting points along that circle
  double pi = 3.1415926535897932384626433832795;
  double increment = ((2 * pi ) / 360);

  vector <DL_PointData> circlePoints;
  DL_PointData myPoint;


  for (; i <= 2 * pi; i += increment, shpIndex++)
  {

    myPoint.x = data.radius * cos(i) + data.cx + currentBlockX;
    myPoint.y = data.radius * sin(i) + data.cy + currentBlockY;
    myPoint.z = data.cz;

    circlePoints.push_back(myPoint);
  }

  SHPObject	*psShape;
  int dim = circlePoints.size();
  double *xv = new double[dim];
  double *yv = new double[dim];
  double *zv = new double[dim];

  for (int i=0; i < dim; i++) 
  {
    xv[i] = circlePoints[i].x;
    yv[i] = circlePoints[i].y;
    zv[i] = circlePoints[i].z;

  }

  psShape = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, dim, xv, yv, zv, NULL  );

  delete [] xv;
  delete [] yv;
  delete [] zv;

  shpObjects.push_back(psShape);

  fetchedprims++;

  circlePoints.clear();

}

void Builder::set_numlayers(int n) { numlayers = n; }
void Builder::set_numpoints(int n) { numpoints = n; }
void Builder::set_numlines(int n) { numlines = n; }
void Builder::set_numplines(int n) { numplines = n; }  

int Builder::ret_numlayers() { return numlayers; }
int Builder::ret_numpoints() { return numpoints; }
int Builder::ret_numlines() { return numlines; }
int Builder::ret_numplines() { return numplines; }
int Builder::ret_textObjectsSize() { return textObjects.size(); }

string Builder::ret_outputshp() { return outputshp; }
string Builder::ret_outputtshp() { return outputtshp; }

void Builder::addText(const DL_TextData& data) {
  if(convertText) {
    DL_TextData myText(
        data.ipx + currentBlockX, data.ipy + currentBlockY,data.ipz,
        data.apx,data.apy,data.apz,               
        data.height,data.xScaleFactor,data.textGenerationFlags,
        data.hJustification,data.vJustification,
        data.text,data.style,data.angle
        );

    textObjects.push_back(myText);

    logfile << "Text added: " <<data.text << "\n";
    fetchedtexts++;
  }
}

void Builder::set_shptype(int n) { shptype = n; } 

void Builder::set_convertText(bool b) { convertText = b; }

void Builder::FinalizeAnyPolyline()
{
  // Save the last polyline / polygon if one exists.
  if (current_polyline_pointcount > 0)
  {
    if (current_polyline_willclose)
    {
      DL_VertexData myVertex;
      myVertex.x = closePolyX;
      myVertex.y = closePolyY;
      myVertex.z = closePolyZ;

      polyVertex.push_back(myVertex);          
    }

    SHPObject	*psObject;
    int dim = polyVertex.size();
    double *xv = new double[dim];
    double *yv = new double[dim];
    double *zv = new double[dim];

    for (int i=0; i < dim; i++) 
    {
      xv[i] = polyVertex[i].x;
      yv[i] = polyVertex[i].y;
      zv[i] = polyVertex[i].z;

    }

    psObject = SHPCreateObject( shapefileType, fetchedprims, 0, NULL, NULL, dim, xv, yv, zv, NULL  );

    delete [] xv;
    delete [] yv;
    delete [] zv;

    shpObjects.push_back(psObject);

    polyVertex.clear();  

    fetchedprims++;

    logfile << "Finalized adding of polyline shape " << fetchedprims - 1 <<"\n";
    current_polyline_pointcount = 0;
  }
}

void Builder::print_shpObjects() {
  int dim = shpObjects.size();
  int dimTexts = textObjects.size();

  logfile << "Number Of Primitives: " << dim << "\n";
  logfile << "Numper of Text Fields: " << dimTexts << "\n";

  SHPHandle	hSHP;

  if(strcmp(".shp",(fname.substr(fname.length()-4)).c_str()) == 0) {
    outputdbf = fname;
    outputdbf = outputdbf.replace((outputdbf.length()-3),outputdbf.length(), "dbf");
    outputshp = fname;
    outputshp = outputshp.replace((outputshp.length()-3),outputshp.length(), "shp");
    outputtdbf = fname;
    outputtdbf = outputtdbf.replace((outputtdbf.length()-4),outputtdbf.length(), "_texts.dbf");
    outputtshp = fname;
    outputtshp = outputtshp.replace((outputtshp.length()-4),outputtshp.length(), "_texts.shp");

    //        outputdbf = fname.replace((fname.length()-3),fname.length(), "dbf");
    //      outputshp = fname.replace((fname.length()-3),fname.length(), "shp");
    //    outputtdbf = fname.replace((fname.length()-4),fname.length(), "_texts.dbf");
    //   outputtshp = fname.replace((fname.length()-4),fname.length(), "_texts.shp");

  }
  else {
    outputdbf = fname;
    outputdbf = outputdbf.append(".dbf");
    outputshp = fname;
    outputshp = outputdbf.append(".shp");
    outputtdbf = fname;
    outputtdbf = outputtdbf.append(".dbf");
    outputtshp = fname;
    outputtshp = outputtdbf.append(".shp");
    //outputdbf = fname.append(".dbf");
    //outputshp = fname.append(".shp");
    //outputtdbf = fname.append("_texts.dbf");
    //outputtshp = fname.append("_texts.shp");
  }

  DBFHandle dbffile = DBFCreate(outputdbf.c_str());
  DBFAddField( dbffile, "myid", FTInteger, 10, 0);

  hSHP = SHPCreate( outputshp.c_str(), shapefileType );

  logfile << "Writing to main shp file...\n"; 

  for(int i=0; i< dim; i++) {
    SHPWriteObject(hSHP, -1, shpObjects[i] );
    SHPDestroyObject(shpObjects[i]);
    DBFWriteIntegerAttribute(dbffile, i, 0, i);      
  }
  SHPClose(hSHP);
  DBFClose(dbffile);

  logfile << "Done!\n";

  if((convertText) && (dimTexts > 0)) {
    SHPHandle	thSHP;

    DBFHandle Tdbffile = DBFCreate(outputtdbf.c_str());
    thSHP = SHPCreate( outputtshp.c_str(), SHPT_POINT );           

    DBFAddField( Tdbffile, "tipx", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tipy", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tipz", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tapx", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tapy", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "tapz", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "height", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "scale", FTDouble, 20, 10 );
    DBFAddField( Tdbffile, "flags", FTInteger, 10, 0 );
    DBFAddField( Tdbffile, "hjust", FTInteger, 10, 0 );
    DBFAddField( Tdbffile, "vjust", FTInteger, 10, 0 );
    DBFAddField( Tdbffile, "text", FTString, 50, 0 );
    DBFAddField( Tdbffile, "style", FTString, 50, 0 );
    DBFAddField( Tdbffile, "angle", FTDouble, 20, 10 );

    logfile << "Writing Texts' shp File...\n";

    for(int i=0; i< dimTexts; i++) {

      SHPObject	*psObject;
      double x = textObjects[i].ipx;
      double y = textObjects[i].ipy;
      double z = textObjects[i].ipz;
      psObject = SHPCreateObject( SHPT_POINT, i, 0, NULL, NULL,
          1, &x, &y, &z, NULL  );

      SHPWriteObject(thSHP, -1, psObject );

      DBFWriteDoubleAttribute( Tdbffile, i, 0, textObjects[i].ipx );
      DBFWriteDoubleAttribute( Tdbffile, i, 1, textObjects[i].ipy );
      DBFWriteDoubleAttribute( Tdbffile, i, 2, textObjects[i].ipz );

      DBFWriteDoubleAttribute( Tdbffile, i, 3, textObjects[i].apx );
      DBFWriteDoubleAttribute( Tdbffile, i, 4, textObjects[i].apy );
      DBFWriteDoubleAttribute( Tdbffile, i, 5, textObjects[i].apz );

      DBFWriteDoubleAttribute( Tdbffile, i, 6, textObjects[i].height );
      DBFWriteDoubleAttribute( Tdbffile, i, 7, textObjects[i].xScaleFactor );
      DBFWriteIntegerAttribute( Tdbffile, i, 8, textObjects[i].textGenerationFlags );

      DBFWriteIntegerAttribute( Tdbffile, i, 9, textObjects[i].hJustification );
      DBFWriteIntegerAttribute( Tdbffile, i, 10, textObjects[i].vJustification );

      DBFWriteStringAttribute( Tdbffile, i, 11, textObjects[i].text.c_str() );
      DBFWriteStringAttribute( Tdbffile, i, 12, textObjects[i].style.c_str() );

      DBFWriteDoubleAttribute( Tdbffile, i, 13, textObjects[i].angle );

      SHPDestroyObject(psObject);
    }
    SHPClose(thSHP);
    DBFClose(Tdbffile);

    logfile << "Done!";
  }
}
