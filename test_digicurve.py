from itertools import count

l = QgsVectorLayer("CompoundCurve?crs=epsg:4326", "test layer", "memory")
QgsProject.instance().addMapLayer(l)


f1 = QgsFeature()
f1.setGeometry(QgsGeometry.fromWkt("LINESTRING(0 0, 5 5, 10 0, 15 5, 20 0)"))
f2 = QgsFeature()
f2.setGeometry(QgsGeometry.fromWkt("COMPOUNDCURVE((0 10, 5 15), CIRCULARSTRING(5 15, 10 10, 15 15), (15 15, 20 10))"))
f3 = QgsFeature()
f3.setGeometry(QgsGeometry.fromWkt("COMPOUNDCURVE(CIRCULARSTRING(0 20, 5 25, 10 20, 15 25, 20 20))"))
f4 = QgsFeature()
f4.setGeometry(QgsGeometry.fromWkt("COMPOUNDCURVE(CIRCULARSTRING(0 30, 5 35, 10 30), (10 30, 15 35, 20 30))"))
f5 = QgsFeature()
f5.setGeometry(QgsGeometry.fromWkt("COMPOUNDCURVE((0 50, 5 55), (5 55, 10 50, 15 55, 20 50))"))
f6 = QgsFeature()
f6.setGeometry(QgsGeometry.fromWkt("COMPOUNDCURVE(CIRCULARSTRING(0 60, 5 65, 10 60), (10 60, 15 65), CIRCULARSTRING(15 65, 20 60, 25 65))"))
l.dataProvider().addFeatures([f1, f2, f3, f4, f5, f6])


for f in l.getFeatures():
    print(f"Feature {f.id()}")
    print("  QgsCompoundCurve::nextVertex()")
    print(f"    {f.geometry().constGet().__class__.__name__} has {f.geometry().constGet().numPoints()} points")

    id = QgsVertexId()
    for i in count():
        exists, point = f.geometry().constGet().nextVertex(id)
        if not exists:
            break
        print(f"      Point id: <{id.part}/{id.ring}/{id.vertex}> point: <{point.x()};{point.y()}>")

    print("  QgsCompoundCurve::nCurves() then QgsCurve::nextVertex()")
    if not hasattr(f.geometry().constGet(), 'nCurves'):
        print(f"  {f.geometry().constGet().__class__.__name__} has not nCurves")
    else:
        for n in range(f.geometry().constGet().nCurves()):
            curve = f.geometry().constGet().curveAt(n)
            print(f"    {curve.__class__.__name__} has {curve.numPoints()} points")
            id = QgsVertexId()
            for i in count():
                exists, point = curve.nextVertex(id)
                if not exists:
                    break
                print(f"      Curve {n} / Point id: <{id.part}/{id.ring}/{id.vertex}> point: <{point.x()};{point.y()}>")
