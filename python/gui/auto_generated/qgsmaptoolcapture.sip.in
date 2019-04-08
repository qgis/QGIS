/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolcapture.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/






class QgsMapToolCapture : QgsMapToolAdvancedDigitizing
{

%TypeHeaderCode
#include "qgsmaptoolcapture.h"
%End
  public:

    enum CaptureMode
    {
      CaptureNone,
      CapturePoint,
      CaptureLine,
      CapturePolygon
    };

    QgsMapToolCapture( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode );
%Docstring
constructor
%End

    ~QgsMapToolCapture();

    virtual void activate();

    virtual void deactivate();


    CaptureMode mode() const;
%Docstring
The capture mode

:return: Capture mode
%End

    int addCurve( QgsCurve *c );
%Docstring
Adds a whole curve (e.g. circularstring) to the captured geometry. Curve must be in map CRS
%End

    void clearCurve( );
%Docstring
Clear capture curve.

.. versionadded:: 3.0
%End

    const QgsCompoundCurve *captureCurve() const;
%Docstring
Gets the capture curve

:return: Capture curve
%End

    QList<QgsPointLocator::Match> snappingMatches() const;
%Docstring
Returns a list of matches for each point on the captureCurve.

.. versionadded:: 3.0
%End

    virtual void cadCanvasMoveEvent( QgsMapMouseEvent *e );


    virtual void keyPressEvent( QKeyEvent *e );

%Docstring
Intercept key events like Esc or Del to delete the last point

:param e: key event
%End

    void deleteTempRubberBand();
%Docstring
Clean a temporary rubberband
%End

    virtual void clean();

%Docstring
convenient method to clean members
%End

  protected:


    int nextPoint( const QgsPoint &mapPoint, QgsPoint &layerPoint );
%Docstring
Converts a map point to layer coordinates

:param mapPoint: the point in map coordinates
\param[in,out] layerPoint the point in layer coordinates

:return:
         0 in case of success
         1 if the current layer is ``None`` or not a vector layer
         2 if the transformation failed
%End


    int nextPoint( QPoint p, QgsPoint &layerPoint, QgsPoint &mapPoint );
%Docstring
Converts a point to map coordinates and layer coordinates

:param p: the input point
\param[in,out] layerPoint the point in layer coordinates
\param[in,out] mapPoint the point in map coordinates

:return:
         0 in case of success
         1 if the current layer is ``None`` or not a vector layer
         2 if the transformation failed
%End


    int fetchLayerPoint( const QgsPointLocator::Match &match, QgsPoint &layerPoint );
%Docstring
Fetches the original point from the source layer if it has the same
CRS as the current layer.

:return: 0 in case of success, 1 if not applicable (CRS mismatch), 2 in case of failure

.. versionadded:: 2.14
%End

    QgsPoint mapPoint( const QgsMapMouseEvent &e ) const;
%Docstring
Creates a QgsPoint with ZM support if necessary (according to the
WkbType of the current layer). If the point is snapped, then the Z
value is took from the snapped point.

:param e: A mouse event

:return: a point with ZM support if necessary


.. versionadded:: 3.0
%End

    QgsPoint mapPoint( const QgsPointXY &point ) const;
%Docstring
Creates a QgsPoint with ZM support if necessary (according to the
WkbType of the current layer).

:param point: A point in 2D

:return: a point with ZM support if necessary


.. versionadded:: 3.0
%End


    int addVertex( const QgsPointXY &point );
%Docstring
Adds a point to the rubber band (in map coordinates) and to the capture list (in layer coordinates)

:return: 0 in case of success, 1 if current layer is not a vector layer, 2 if coordinate transformation failed
%End

    int addVertex( const QgsPointXY &mapPoint, const QgsPointLocator::Match &match );
%Docstring
Variant to supply more information in the case of snapping

:param mapPoint: The vertex to add in map coordinates
:param match: Data about the snapping match. Can be an invalid match, if point not snapped.

.. versionadded:: 2.14
%End

    void undo();
%Docstring
Removes the last vertex from mRubberBand and mCaptureList
%End

    void startCapturing();
%Docstring
Start capturing
%End

    bool isCapturing() const;
%Docstring
Are we currently capturing?

:return: Is the tool in capture mode?
%End

    int size();
%Docstring
Number of points digitized

:return: Number of points
%End

    QVector<QgsPointXY> points() const;
%Docstring
List of digitized points

:return: List of points
%End

    void setPoints( const QVector<QgsPointXY> &pointList );
%Docstring
Set the points on which to work

:param pointList: A list of points
%End

    void closePolygon();
%Docstring
Close an open polygon
%End

  protected slots:

    void stopCapturing();
%Docstring
Stop capturing
%End

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmaptoolcapture.h                                          *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
