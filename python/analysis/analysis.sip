%Module(name=qgis._analysis,
        version=0,
        keyword_arguments="Optional")

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtXml/QtXmlmod.sip

%Import core/core.sip

%Include vector/qgsgeometryanalyzer.sip
%Include vector/qgsoverlayanalyzer.sip
%Include vector/qgspointsample.sip
%Include vector/qgstransectsample.sip
%Include vector/qgszonalstatistics.sip

// %Include interpolation/Bezier3D.sip
// %Include interpolation/CloughTocherInterpolator.sip
// %Include interpolation/DualEdgeTriangulation.sip
// %Include interpolation/HalfEdge.sip
// %Include interpolation/LinTriangleInterpolator.sip
// %Include interpolation/Line3D.sip
// %Include interpolation/Node.sip
// %Include interpolation/NormVecDecorator.sip
// %Include interpolation/ParametricLine.sip
// %Include interpolation/Point3D.sip
// %Include interpolation/TriangleInterpolator.sip
// %Include interpolation/Triangulation.sip
// %Include interpolation/TriDecorator.sip
// %Include interpolation/Vector3D.sip

%Include interpolation/qgsgridfilewriter.sip
%Include interpolation/qgsinterpolator.sip
%Include interpolation/qgsidwinterpolator.sip
%Include interpolation/qgstininterpolator.sip

%Include raster/qgsderivativefilter.sip
%Include raster/qgsaspectfilter.sip
%Include raster/qgshillshadefilter.sip
%Include raster/qgsninecellfilter.sip
%Include raster/qgsrastercalcnode.sip
%Include raster/qgsrastercalculator.sip
%Include raster/qgsrastermatrix.sip
%Include raster/qgsrelief.sip
%Include raster/qgsruggednessfilter.sip
%Include raster/qgsslopefilter.sip
%Include raster/qgstotalcurvaturefilter.sip
