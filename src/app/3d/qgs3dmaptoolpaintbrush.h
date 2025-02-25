/***************************************************************************
  qgs3dmaptoolpaintbrush.h
  --------------------------------------
  Date                 : January 2025
  Copyright            : (C) 2025 by Matej Bagar
  Email                : matej dot bagar at lutraconsulting dot co dot uk
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS3DMAPTOOLPAINTBRUSH_H
#define QGS3DMAPTOOLPAINTBRUSH_H
#include "qgs3dmaptool.h"
#include "qgspoint.h"

#include <QMatrix4x4>

struct MapToPixel3D;
class QgsPointCloudIndex;
class QgsChunkNode;
class QgsGeometry;
class QgsPointCloudNodeId;
class QgsRubberBand3D;
class QgsPointCloudLayer;

typedef QHash<QgsPointCloudNodeId, QVector<int> > SelectedPoints;

class Qgs3DMapToolPaintBrush : public Qgs3DMapTool
{
    Q_OBJECT

  public:
    explicit Qgs3DMapToolPaintBrush( Qgs3DMapCanvas *canvas );
    ~Qgs3DMapToolPaintBrush() override;

    void activate() override;

    void deactivate() override;

    QCursor cursor() const override;

    bool allowsCameraControls() const override { return false; }

    //! Reset all saved position data
    void reset();

    void setAttribute( const QString &attribute );

    void setNewValue( double value );

  private slots:
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mouseWheelEvent( QWheelEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

  private:
    //! Process the area selected by user
    void processSelection() const;
    SelectedPoints searchPoints( QgsPointCloudLayer *layer, const QgsGeometry &searchPolygon ) const;
    static QVector<int> selectedPointsInNode( const QgsGeometry &searchPolygon, const QgsChunkNode *ch, const MapToPixel3D &mapToPixel3D, QgsPointCloudIndex &pcIndex );
    static QgsGeometry box3DToPolygonInScreenSpace( const QgsBox3D &box, const MapToPixel3D &mapToPixel3D );
    void generateHighlightArea();

    std::unique_ptr<QgsRubberBand3D> mSelectionRubberBand;
    std::unique_ptr<QgsRubberBand3D> mHighlighterRubberBand;
    QVector<QgsPointXY> mDragPositions = QVector<QgsPointXY>();
    //! Check if mouse was moved between mousePress and mouseRelease
    bool mIsClicked = false;
    //! Check if the tool is selected
    bool mIsActive = false;
    //! Check if the tool or movement is active
    bool mIsMoving = false;
    QString mAttributeName;
    double mNewValue = 0;
};


#endif //QGS3DMAPTOOLPAINTBRUSH_H
