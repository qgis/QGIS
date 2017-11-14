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

    virtual ~QgsMapToolCapture();

    virtual void activate();
    virtual void deactivate();

    CaptureMode mode() const;
%Docstring
 The capture mode

 :return: Capture mode
 :rtype: CaptureMode
%End

    int addCurve( QgsCurve *c );
%Docstring
Adds a whole curve (e.g. circularstring) to the captured geometry. Curve must be in map CRS
 :rtype: int
%End

    void clearCurve( );
%Docstring
 Clear capture curve.

.. versionadded:: 3.0
%End

    const QgsCompoundCurve *captureCurve() const;
%Docstring
 Get the capture curve

 :return: Capture curve
 :rtype: QgsCompoundCurve
%End

    QList<QgsPointLocator::Match> snappingMatches() const;
%Docstring
 Return a list of matches for each point on the captureCurve.

.. versionadded:: 3.0
 :rtype: list of QgsPointLocator.Match
%End

    virtual void cadCanvasMoveEvent( QgsMapMouseEvent *e );

    virtual void keyPressEvent( QKeyEvent *e );
%Docstring
 Intercept key events like Esc or Del to delete the last point
 \param e key event
%End

    virtual bool eventFilter( QObject *obj, QEvent *e );

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
 :rtype: int
%End

    int nextPoint( QPoint p, QgsPoint &layerPoint, QgsPoint &mapPoint );
%Docstring
 :rtype: int
%End

    int fetchLayerPoint( const QgsPointLocator::Match &match, QgsPoint &layerPoint );
%Docstring
 :rtype: int
%End

    int addVertex( const QgsPointXY &point );
%Docstring
 :rtype: int
%End

    int addVertex( const QgsPointXY &mapPoint, const QgsPointLocator::Match &match );
%Docstring
 Variant to supply more information in the case of snapping
 \param mapPoint The vertex to add in map coordinates
 \param match Data about the snapping match. Can be an invalid match, if point not snapped.
.. versionadded:: 2.14
 :rtype: int
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
 :rtype: bool
%End

    int size();
%Docstring
 Number of points digitized

 :return: Number of points
 :rtype: int
%End

    QVector<QgsPointXY> points();
%Docstring
 List of digitized points
 :return: List of points
 :rtype: list of QgsPointXY
%End

    void setPoints( const QVector<QgsPointXY> &pointList );
%Docstring
 Set the points on which to work

 \param pointList A list of points
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
