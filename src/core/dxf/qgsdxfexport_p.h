/***************************************************************************
                         qgsdxfexport_p.h
                         --------------
    begin                : November 2019
    copyright            : (C) 2019 by Matthias Kuhn
    email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsvectorlayer.h"
#include "qgsexpressioncontext.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvectorlayerfeatureiterator.h"
#include "qgsrenderer.h"
#include "qgsvectorlayerlabeling.h"
#include "qgslabelsink.h"

/**
 * Holds information about each layer in a DXF job.
 * This can be used for multithreading.
 */
struct DxfLayerJob
{
    DxfLayerJob( QgsVectorLayer *vl, const QString &layerStyleOverride, QgsRenderContext &renderContext, QgsDxfExport *dxfExport, const QString &splitLayerAttribute )
      : renderContext( renderContext )
      , styleOverride( vl )
      , featureSource( vl )
      , dxfExport( dxfExport )
      , crs( vl->crs() )
      , layerName( vl->name() )
      , splitLayerAttribute( splitLayerAttribute )
      , layerTitle( vl->title().isEmpty() ? vl->name() : vl->title() )
    {
      fields = vl->fields();
      renderer.reset( vl->renderer()->clone() );
      renderContext.expressionContext().appendScope( QgsExpressionContextUtils::layerScope( vl ) );

      if ( !layerStyleOverride.isNull() )
      {
        styleOverride.setOverrideStyle( layerStyleOverride );
      }

      labeling.reset( vl->labelsEnabled() ? vl->labeling()->clone() : nullptr );

      attributes = renderer->usedAttributes( renderContext );
      if ( !splitLayerAttribute.isNull() )
      {
        attributes << splitLayerAttribute;
      }

      if ( labeling )
      {
        QgsLabelingEngine *labelingEngine = renderContext.labelingEngine();
        if ( const QgsRuleBasedLabeling *rbl = dynamic_cast<const QgsRuleBasedLabeling *>( labeling.get() ) )
        {
          ruleBasedLabelProvider = new QgsRuleBasedLabelSinkProvider( *rbl, vl, dxfExport );
          labelingEngine->addProvider( ruleBasedLabelProvider );

          if ( !ruleBasedLabelProvider->prepare( renderContext, attributes ) )
          {
            labelingEngine->removeProvider( ruleBasedLabelProvider );
            ruleBasedLabelProvider = nullptr;
          }
        }
        else
        {
          QgsPalLayerSettings settings = labeling->settings();
          labelProvider = new QgsLabelSinkProvider( vl, QString(), dxfExport, &settings );
          labelingEngine->addProvider( labelProvider );

          if ( !labelProvider->prepare( renderContext, attributes ) )
          {
            labelingEngine->removeProvider( labelProvider );
            labelProvider = nullptr;
          }
        }
      }

      // This will need to be started in a separate thread, if threaded somewhere else to
      renderer->startRender( renderContext, fields );
    };

    QgsRenderContext renderContext;
    QgsFields fields;
    QgsMapLayerStyleOverride styleOverride;
    QgsVectorLayerFeatureSource featureSource;
    std::unique_ptr< QgsFeatureRenderer > renderer;
    std::unique_ptr<QgsAbstractVectorLayerLabeling> labeling;
    QgsDxfExport *dxfExport = nullptr;
    QgsCoordinateReferenceSystem crs;
    QString layerName;
    QgsLabelSinkProvider *labelProvider = nullptr;
    QgsRuleBasedLabelSinkProvider *ruleBasedLabelProvider = nullptr;
    QString splitLayerAttribute;
    QString layerTitle;
    QSet<QString> attributes;

  private:
    DxfLayerJob( const DxfLayerJob & ) = delete;
    DxfLayerJob &operator=( const DxfLayerJob & ) = delete;
};

// dxf color palette
static const int sDxfColors[][3] =
{
  { 255, 255, 255 },
  { 255, 0, 0 },
  { 255, 255, 0 },
  { 0, 255, 0 },
  { 0, 255, 255 },
  { 0, 0, 255 },
  { 255, 0, 255 },
  { 0, 0, 0 },
  { 128, 128, 128 },
  { 192, 192, 192 },
  { 255, 0, 0 },
  { 255, 127, 127 },
  { 204, 0, 0 },
  { 204, 102, 102 },
  { 153, 0, 0 },
  { 153, 76, 76 },
  { 127, 0, 0 },
  { 127, 63, 63 },
  { 76, 0, 0 },
  { 76, 38, 38 },
  { 255, 63, 0 },
  { 255, 159, 127 },
  { 204, 51, 0 },
  { 204, 127, 102 },
  { 153, 38, 0 },
  { 153, 95, 76 },
  { 127, 31, 0 },
  { 127, 79, 63 },
  { 76, 19, 0 },
  { 76, 47, 38 },
  { 255, 127, 0 },
  { 255, 191, 127 },
  { 204, 102, 0 },
  { 204, 153, 102 },
  { 153, 76, 0 },
  { 153, 114, 76 },
  { 127, 63, 0 },
  { 127, 95, 63 },
  { 76, 38, 0 },
  { 76, 57, 38 },
  { 255, 191, 0 },
  { 255, 223, 127 },
  { 204, 153, 0 },
  { 204, 178, 102 },
  { 153, 114, 0 },
  { 153, 133, 76 },
  { 127, 95, 0 },
  { 127, 111, 63 },
  { 76, 57, 0 },
  { 76, 66, 38 },
  { 255, 255, 0 },
  { 255, 255, 127 },
  { 204, 204, 0 },
  { 204, 204, 102 },
  { 153, 153, 0 },
  { 153, 153, 76 },
  { 127, 127, 0 },
  { 127, 127, 63 },
  { 76, 76, 0 },
  { 76, 76, 38 },
  { 191, 255, 0 },
  { 223, 255, 127 },
  { 153, 204, 0 },
  { 178, 204, 102 },
  { 114, 153, 0 },
  { 133, 153, 76 },
  { 95, 127, 0 },
  { 111, 127, 63 },
  { 57, 76, 0 },
  { 66, 76, 38 },
  { 127, 255, 0 },
  { 191, 255, 127 },
  { 102, 204, 0 },
  { 153, 204, 102 },
  { 76, 153, 0 },
  { 114, 153, 76 },
  { 63, 127, 0 },
  { 95, 127, 63 },
  { 38, 76, 0 },
  { 57, 76, 38 },
  { 63, 255, 0 },
  { 159, 255, 127 },
  { 51, 204, 0 },
  { 127, 204, 102 },
  { 38, 153, 0 },
  { 95, 153, 76 },
  { 31, 127, 0 },
  { 79, 127, 63 },
  { 19, 76, 0 },
  { 47, 76, 38 },
  { 0, 255, 0 },
  { 127, 255, 127 },
  { 0, 204, 0 },
  { 102, 204, 102 },
  { 0, 153, 0 },
  { 76, 153, 76 },
  { 0, 127, 0 },
  { 63, 127, 63 },
  { 0, 76, 0 },
  { 38, 76, 38 },
  { 0, 255, 63 },
  { 127, 255, 159 },
  { 0, 204, 51 },
  { 102, 204, 127 },
  { 0, 153, 38 },
  { 76, 153, 95 },
  { 0, 127, 31 },
  { 63, 127, 79 },
  { 0, 76, 19 },
  { 38, 76, 47 },
  { 0, 255, 127 },
  { 127, 255, 191 },
  { 0, 204, 102 },
  { 102, 204, 153 },
  { 0, 153, 76 },
  { 76, 153, 114 },
  { 0, 127, 63 },
  { 63, 127, 95 },
  { 0, 76, 38 },
  { 38, 76, 57 },
  { 0, 255, 191 },
  { 127, 255, 223 },
  { 0, 204, 153 },
  { 102, 204, 178 },
  { 0, 153, 114 },
  { 76, 153, 133 },
  { 0, 127, 95 },
  { 63, 127, 111 },
  { 0, 76, 57 },
  { 38, 76, 66 },
  { 0, 255, 255 },
  { 127, 255, 255 },
  { 0, 204, 204 },
  { 102, 204, 204 },
  { 0, 153, 153 },
  { 76, 153, 153 },
  { 0, 127, 127 },
  { 63, 127, 127 },
  { 0, 76, 76 },
  { 38, 76, 76 },
  { 0, 191, 255 },
  { 127, 223, 255 },
  { 0, 153, 204 },
  { 102, 178, 204 },
  { 0, 114, 153 },
  { 76, 133, 153 },
  { 0, 95, 127 },
  { 63, 111, 127 },
  { 0, 57, 76 },
  { 38, 66, 76 },
  { 0, 127, 255 },
  { 127, 191, 255 },
  { 0, 102, 204 },
  { 102, 153, 204 },
  { 0, 76, 153 },
  { 76, 114, 153 },
  { 0, 63, 127 },
  { 63, 95, 127 },
  { 0, 38, 76 },
  { 38, 57, 76 },
  { 0, 63, 255 },
  { 127, 159, 255 },
  { 0, 51, 204 },
  { 102, 127, 204 },
  { 0, 38, 153 },
  { 76, 95, 153 },
  { 0, 31, 127 },
  { 63, 79, 127 },
  { 0, 19, 76 },
  { 38, 47, 76 },
  { 0, 0, 255 },
  { 127, 127, 255 },
  { 0, 0, 204 },
  { 102, 102, 204 },
  { 0, 0, 153 },
  { 76, 76, 153 },
  { 0, 0, 127 },
  { 63, 63, 127 },
  { 0, 0, 76 },
  { 38, 38, 76 },
  { 63, 0, 255 },
  { 159, 127, 255 },
  { 51, 0, 204 },
  { 127, 102, 204 },
  { 38, 0, 153 },
  { 95, 76, 153 },
  { 31, 0, 127 },
  { 79, 63, 127 },
  { 19, 0, 76 },
  { 47, 38, 76 },
  { 127, 0, 255 },
  { 191, 127, 255 },
  { 102, 0, 204 },
  { 153, 102, 204 },
  { 76, 0, 153 },
  { 114, 76, 153 },
  { 63, 0, 127 },
  { 95, 63, 127 },
  { 38, 0, 76 },
  { 57, 38, 76 },
  { 191, 0, 255 },
  { 223, 127, 255 },
  { 153, 0, 204 },
  { 178, 102, 204 },
  { 114, 0, 153 },
  { 133, 76, 153 },
  { 95, 0, 127 },
  { 111, 63, 127 },
  { 57, 0, 76 },
  { 66, 38, 76 },
  { 255, 0, 255 },
  { 255, 127, 255 },
  { 204, 0, 204 },
  { 204, 102, 204 },
  { 153, 0, 153 },
  { 153, 76, 153 },
  { 127, 0, 127 },
  { 127, 63, 127 },
  { 76, 0, 76 },
  { 76, 38, 76 },
  { 255, 0, 191 },
  { 255, 127, 223 },
  { 204, 0, 153 },
  { 204, 102, 178 },
  { 153, 0, 114 },
  { 153, 76, 133 },
  { 127, 0, 95 },
  { 127, 63, 111 },
  { 76, 0, 57 },
  { 76, 38, 66 },
  { 255, 0, 127 },
  { 255, 127, 191 },
  { 204, 0, 102 },
  { 204, 102, 153 },
  { 153, 0, 76 },
  { 153, 76, 114 },
  { 127, 0, 63 },
  { 127, 63, 95 },
  { 76, 0, 38 },
  { 76, 38, 57 },
  { 255, 0, 63 },
  { 255, 127, 159 },
  { 204, 0, 51 },
  { 204, 102, 127 },
  { 153, 0, 38 },
  { 153, 76, 95 },
  { 127, 0, 31 },
  { 127, 63, 79 },
  { 76, 0, 19 },
  { 76, 38, 47 },
  { 51, 51, 51 },
  { 91, 91, 91 },
  { 132, 132, 132 },
  { 173, 173, 173 },
  { 214, 214, 214 },
  { 255, 255, 255 },
};

static const char *DXF_ENCODINGS[][2] =
{
  { "ASCII", "" },
  { "8859_1", "ISO-8859-1" },
  { "8859_2", "ISO-8859-2" },
  { "8859_3", "ISO-8859-3" },
  { "8859_4", "ISO-8859-4" },
  { "8859_5", "ISO-8859-5" },
  { "8859_6", "ISO-8859-6" },
  { "8859_7", "ISO-8859-7" },
  { "8859_8", "ISO-8859-8" },
  { "8859_9", "ISO-8859-9" },
//  { "DOS437", "" },
  { "DOS850", "CP850" },
//  { "DOS852", "" },
//  { "DOS855", "" },
//  { "DOS857", "" },
//  { "DOS860", "" },
//  { "DOS861", "" },
//  { "DOS863", "" },
//  { "DOS864", "" },
//  { "DOS865", "" },
//  { "DOS869", "" },
//  { "DOS932", "" },
  { "MACINTOSH", "MacRoman" },
  { "BIG5", "Big5" },
  { "KSC5601", "ksc5601.1987-0" },
//   { "JOHAB", "" },
  { "DOS866", "CP866" },
  { "ANSI_1250", "CP1250" },
  { "ANSI_1251", "CP1251" },
  { "ANSI_1252", "CP1252" },
  { "GB2312", "GB2312" },
  { "ANSI_1253", "CP1253" },
  { "ANSI_1254", "CP1254" },
  { "ANSI_1255", "CP1255" },
  { "ANSI_1256", "CP1256" },
  { "ANSI_1257", "CP1257" },
  { "ANSI_874", "CP874" },
  { "ANSI_932", "Shift_JIS" },
  { "ANSI_936", "CP936" },
  { "ANSI_949", "CP949" },
  { "ANSI_949", "ms949" },
  { "ANSI_950", "CP950" },
//  { "ANSI_1361", "" },
//  { "ANSI_1200", "" },
  { "ANSI_1258", "CP1258" },
};

// From GDAL trailer.dxf
#define DXF_TRAILER "\
0\n\
SECTION\n\
2\n\
OBJECTS\n\
0\n\
DICTIONARY\n\
5\n\
C\n\
330\n\
0\n\
100\n\
AcDbDictionary\n\
281\n\
   1\n\
3\n\
ACAD_GROUP\n\
350\n\
D\n\
3\n\
ACAD_LAYOUT\n\
350\n\
1A\n\
3\n\
ACAD_MLEADERSTYLE\n\
350\n\
43\n\
3\n\
ACAD_MLINESTYLE\n\
350\n\
17\n\
3\n\
ACAD_PLOTSETTINGS\n\
350\n\
19\n\
3\n\
ACAD_PLOTSTYLENAME\n\
350\n\
E\n\
3\n\
ACAD_TABLESTYLE\n\
350\n\
42\n\
3\n\
ACAD_VISUALSTYLE\n\
350\n\
2A\n\
0\n\
DICTIONARY\n\
5\n\
D\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
   1\n\
0\n\
DICTIONARY\n\
5\n\
1A\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
   1\n\
3\n\
Layout1\n\
350\n\
1E\n\
3\n\
Layout2\n\
350\n\
26\n\
3\n\
Model\n\
350\n\
22\n\
0\n\
DICTIONARY\n\
5\n\
43\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
   1\n\
0\n\
DICTIONARY\n\
5\n\
17\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
   1\n\
3\n\
Standard\n\
350\n\
18\n\
0\n\
DICTIONARY\n\
5\n\
19\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
   1\n\
0\n\
ACDBDICTIONARYWDFLT\n\
5\n\
E\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
   1\n\
3\n\
Normal\n\
350\n\
F\n\
100\n\
AcDbDictionaryWithDefault\n\
340\n\
F\n\
0\n\
DICTIONARY\n\
5\n\
42\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
   1\n\
0\n\
DICTIONARY\n\
5\n\
2A\n\
102\n\
{ACAD_REACTORS\n\
330\n\
C\n\
102\n\
}\n\
330\n\
C\n\
100\n\
AcDbDictionary\n\
281\n\
   1\n\
3\n\
2dWireframe\n\
350\n\
2F\n\
3\n\
3D Hidden\n\
350\n\
31\n\
3\n\
3dWireframe\n\
350\n\
30\n\
3\n\
Basic\n\
350\n\
32\n\
3\n\
Brighten\n\
350\n\
36\n\
3\n\
ColorChange\n\
350\n\
3A\n\
3\n\
Conceptual\n\
350\n\
34\n\
3\n\
Dim\n\
350\n\
35\n\
3\n\
Facepattern\n\
350\n\
39\n\
3\n\
Flat\n\
350\n\
2B\n\
3\n\
FlatWithEdges\n\
350\n\
2C\n\
3\n\
Gouraud\n\
350\n\
2D\n\
3\n\
GouraudWithEdges\n\
350\n\
2E\n\
3\n\
Linepattern\n\
350\n\
38\n\
3\n\
Realistic\n\
350\n\
33\n\
3\n\
Thicken\n\
350\n\
37\n\
0\n\
LAYOUT\n\
5\n\
1E\n\
102\n\
{ACAD_REACTORS\n\
330\n\
1A\n\
102\n\
}\n\
330\n\
1A\n\
100\n\
AcDbPlotSettings\n\
1\n\
\n\
2\n\
none_device\n\
4\n\
\n\
6\n\
\n\
40\n\
0.0\n\
41\n\
0.0\n\
42\n\
0.0\n\
43\n\
0.0\n\
44\n\
0.0\n\
45\n\
0.0\n\
46\n\
0.0\n\
47\n\
0.0\n\
48\n\
0.0\n\
49\n\
0.0\n\
140\n\
0.0\n\
141\n\
0.0\n\
142\n\
1.0\n\
143\n\
1.0\n\
70\n\
 688\n\
72\n\
   0\n\
73\n\
   0\n\
74\n\
   5\n\
7\n\
\n\
75\n\
  16\n\
76\n\
   0\n\
77\n\
   2\n\
78\n\
 300\n\
147\n\
1.0\n\
148\n\
0.0\n\
149\n\
0.0\n\
100\n\
AcDbLayout\n\
1\n\
Layout1\n\
70\n\
   1\n\
71\n\
   1\n\
10\n\
0.0\n\
20\n\
0.0\n\
11\n\
12.0\n\
21\n\
9.0\n\
12\n\
0.0\n\
22\n\
0.0\n\
32\n\
0.0\n\
14\n\
1.000000000000000E+20\n\
24\n\
1.000000000000000E+20\n\
34\n\
1.000000000000000E+20\n\
15\n\
-1.000000000000000E+20\n\
25\n\
-1.000000000000000E+20\n\
35\n\
-1.000000000000000E+20\n\
146\n\
0.0\n\
13\n\
0.0\n\
23\n\
0.0\n\
33\n\
0.0\n\
16\n\
1.0\n\
26\n\
0.0\n\
36\n\
0.0\n\
17\n\
0.0\n\
27\n\
1.0\n\
37\n\
0.0\n\
76\n\
   0\n\
330\n\
1B\n\
0\n\
LAYOUT\n\
5\n\
26\n\
102\n\
{ACAD_REACTORS\n\
330\n\
1A\n\
102\n\
}\n\
330\n\
1A\n\
100\n\
AcDbPlotSettings\n\
1\n\
\n\
2\n\
none_device\n\
4\n\
\n\
6\n\
\n\
40\n\
0.0\n\
41\n\
0.0\n\
42\n\
0.0\n\
43\n\
0.0\n\
44\n\
0.0\n\
45\n\
0.0\n\
46\n\
0.0\n\
47\n\
0.0\n\
48\n\
0.0\n\
49\n\
0.0\n\
140\n\
0.0\n\
141\n\
0.0\n\
142\n\
1.0\n\
143\n\
1.0\n\
70\n\
 688\n\
72\n\
   0\n\
73\n\
   0\n\
74\n\
   5\n\
7\n\
\n\
75\n\
  16\n\
76\n\
   0\n\
77\n\
   2\n\
78\n\
 300\n\
147\n\
1.0\n\
148\n\
0.0\n\
149\n\
0.0\n\
100\n\
AcDbLayout\n\
1\n\
Layout2\n\
70\n\
   1\n\
71\n\
   2\n\
10\n\
0.0\n\
20\n\
0.0\n\
11\n\
0.0\n\
21\n\
0.0\n\
12\n\
0.0\n\
22\n\
0.0\n\
32\n\
0.0\n\
14\n\
0.0\n\
24\n\
0.0\n\
34\n\
0.0\n\
15\n\
0.0\n\
25\n\
0.0\n\
35\n\
0.0\n\
146\n\
0.0\n\
13\n\
0.0\n\
23\n\
0.0\n\
33\n\
0.0\n\
16\n\
1.0\n\
26\n\
0.0\n\
36\n\
0.0\n\
17\n\
0.0\n\
27\n\
1.0\n\
37\n\
0.0\n\
76\n\
   0\n\
330\n\
23\n\
0\n\
LAYOUT\n\
5\n\
22\n\
102\n\
{ACAD_REACTORS\n\
330\n\
1A\n\
102\n\
}\n\
330\n\
1A\n\
100\n\
AcDbPlotSettings\n\
1\n\
\n\
2\n\
none_device\n\
4\n\
\n\
6\n\
\n\
40\n\
0.0\n\
41\n\
0.0\n\
42\n\
0.0\n\
43\n\
0.0\n\
44\n\
0.0\n\
45\n\
0.0\n\
46\n\
0.0\n\
47\n\
0.0\n\
48\n\
0.0\n\
49\n\
0.0\n\
140\n\
0.0\n\
141\n\
0.0\n\
142\n\
1.0\n\
143\n\
1.0\n\
70\n\
1712\n\
72\n\
   0\n\
73\n\
   0\n\
74\n\
   0\n\
7\n\
\n\
75\n\
   0\n\
76\n\
   0\n\
77\n\
   2\n\
78\n\
 300\n\
147\n\
1.0\n\
148\n\
0.0\n\
149\n\
0.0\n\
100\n\
AcDbLayout\n\
1\n\
Model\n\
70\n\
   1\n\
71\n\
   0\n\
10\n\
0.0\n\
20\n\
0.0\n\
11\n\
12.0\n\
21\n\
9.0\n\
12\n\
0.0\n\
22\n\
0.0\n\
32\n\
0.0\n\
14\n\
30.0\n\
24\n\
49.75\n\
34\n\
0.0\n\
15\n\
130.5\n\
25\n\
163.1318914119703\n\
35\n\
0.0\n\
146\n\
0.0\n\
13\n\
0.0\n\
23\n\
0.0\n\
33\n\
0.0\n\
16\n\
1.0\n\
26\n\
0.0\n\
36\n\
0.0\n\
17\n\
0.0\n\
27\n\
1.0\n\
37\n\
0.0\n\
76\n\
   0\n\
330\n\
1F\n\
331\n\
29\n\
0\n\
MLINESTYLE\n\
5\n\
18\n\
102\n\
{ACAD_REACTORS\n\
330\n\
17\n\
102\n\
}\n\
330\n\
17\n\
100\n\
AcDbMlineStyle\n\
2\n\
Standard\n\
70\n\
   0\n\
3\n\
\n\
62\n\
 256\n\
51\n\
90.0\n\
52\n\
90.0\n\
71\n\
   2\n\
49\n\
0.5\n\
62\n\
 256\n\
6\n\
BYLAYER\n\
49\n\
-0.5\n\
62\n\
 256\n\
6\n\
BYLAYER\n\
0\n\
ACDBPLACEHOLDER\n\
5\n\
F\n\
102\n\
{ACAD_REACTORS\n\
330\n\
E\n\
102\n\
}\n\
330\n\
E\n\
0\n\
VISUALSTYLE\n\
5\n\
2F\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
2dWireframe\n\
70\n\
   4\n\
71\n\
   0\n\
72\n\
   2\n\
73\n\
   0\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      0\n\
66\n\
 257\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   0\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
31\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
3D Hidden\n\
70\n\
   6\n\
71\n\
   1\n\
72\n\
   2\n\
73\n\
   2\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   2\n\
91\n\
      2\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   2\n\
175\n\
   1\n\
42\n\
40.0\n\
92\n\
      0\n\
66\n\
 257\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   3\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   0\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
30\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
3dWireframe\n\
70\n\
   5\n\
71\n\
   0\n\
72\n\
   2\n\
73\n\
   0\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      0\n\
66\n\
 257\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   0\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
32\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Basic\n\
70\n\
   7\n\
71\n\
   1\n\
72\n\
   0\n\
73\n\
   1\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   0\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      8\n\
66\n\
   7\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
36\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Brighten\n\
70\n\
  12\n\
71\n\
   2\n\
72\n\
   2\n\
73\n\
   0\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      8\n\
66\n\
   7\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
50.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
3A\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
ColorChange\n\
70\n\
  16\n\
71\n\
   2\n\
72\n\
   2\n\
73\n\
   3\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   8\n\
421\n\
8421504\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      8\n\
66\n\
   8\n\
424\n\
8421504\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
34\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Conceptual\n\
70\n\
   9\n\
71\n\
   3\n\
72\n\
   2\n\
73\n\
   0\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   2\n\
91\n\
      2\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
40.0\n\
92\n\
      8\n\
66\n\
   7\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   3\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   0\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
35\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Dim\n\
70\n\
  11\n\
71\n\
   2\n\
72\n\
   2\n\
73\n\
   0\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      8\n\
66\n\
   7\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
-50.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
39\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Facepattern\n\
70\n\
  15\n\
71\n\
   2\n\
72\n\
   2\n\
73\n\
   0\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      8\n\
66\n\
   7\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
2B\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Flat\n\
70\n\
   0\n\
71\n\
   2\n\
72\n\
   1\n\
73\n\
   1\n\
90\n\
      2\n\
40\n\
-0.6\n\
41\n\
30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   0\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      8\n\
66\n\
   7\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
     13\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
2C\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
FlatWithEdges\n\
70\n\
   1\n\
71\n\
   2\n\
72\n\
   1\n\
73\n\
   1\n\
90\n\
      2\n\
40\n\
-0.6\n\
41\n\
30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      0\n\
66\n\
 257\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
     13\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
2D\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Gouraud\n\
70\n\
   2\n\
71\n\
   2\n\
72\n\
   2\n\
73\n\
   1\n\
90\n\
      2\n\
40\n\
-0.6\n\
41\n\
30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   0\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      0\n\
66\n\
   7\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
     13\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
2E\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
GouraudWithEdges\n\
70\n\
   3\n\
71\n\
   2\n\
72\n\
   2\n\
73\n\
   1\n\
90\n\
      2\n\
40\n\
-0.6\n\
41\n\
30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      0\n\
66\n\
 257\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
     13\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
38\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Linepattern\n\
70\n\
  14\n\
71\n\
   2\n\
72\n\
   2\n\
73\n\
   0\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   7\n\
175\n\
   7\n\
42\n\
1.0\n\
92\n\
      8\n\
66\n\
   7\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
33\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Realistic\n\
70\n\
   8\n\
71\n\
   2\n\
72\n\
   2\n\
73\n\
   0\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      0\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
      8\n\
66\n\
   8\n\
424\n\
7895160\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
     13\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   0\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
VISUALSTYLE\n\
5\n\
37\n\
102\n\
{ACAD_REACTORS\n\
330\n\
2A\n\
102\n\
}\n\
330\n\
2A\n\
100\n\
AcDbVisualStyle\n\
2\n\
Thicken\n\
70\n\
  13\n\
71\n\
   2\n\
72\n\
   2\n\
73\n\
   0\n\
90\n\
      0\n\
40\n\
-0.6\n\
41\n\
-30.0\n\
62\n\
   5\n\
63\n\
   7\n\
421\n\
16777215\n\
74\n\
   1\n\
91\n\
      4\n\
64\n\
   7\n\
65\n\
 257\n\
75\n\
   1\n\
175\n\
   1\n\
42\n\
1.0\n\
92\n\
     12\n\
66\n\
   7\n\
43\n\
1.0\n\
76\n\
   1\n\
77\n\
   6\n\
78\n\
   2\n\
67\n\
   7\n\
79\n\
   5\n\
170\n\
   0\n\
171\n\
   0\n\
290\n\
   0\n\
174\n\
   0\n\
93\n\
      1\n\
44\n\
0.0\n\
173\n\
   0\n\
291\n\
   1\n\
45\n\
0.0\n\
1001\n\
ACAD\n\
1000\n\
AcDbSavedByObjectVersion\n\
1070\n\
   0\n\
0\n\
ENDSEC\n\
"
