/***************************************************************************
                             qgscreateannotationitemmaptool_impl.h
                             ------------------------
    Date                 : September 2021
    Copyright            : (C) 2021 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCREATEANNOTATIONITEMMAPTOOLIMPL_H
#define QGSCREATEANNOTATIONITEMMAPTOOLIMPL_H

#include "qgis_gui.h"
#include "qgscreateannotationitemmaptool.h"
#include "qgsmaptoolcapture.h"

#define SIP_NO_FILE

class QgsSettingsEntryString;
class QgsSettingsTreeNode;


/**
 * \class QgsMapToolCaptureAnnotationItem
 * \ingroup gui
 *
 * \brief A base class to digitize annotation items using QgsMapToolCapture
 *
 * \note Not available in Python bindings.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsMapToolCaptureAnnotationItem : public QgsMapToolCapture, public QgsCreateAnnotationItemMapToolInterface
{
    Q_OBJECT
  public:
    //! Constructor
    QgsMapToolCaptureAnnotationItem( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget, CaptureMode mode );
    QgsCreateAnnotationItemMapToolHandler *handler() const override;
    QgsMapTool *mapTool() override;
    QgsMapLayer *layer() const override;
    QgsMapToolCapture::Capabilities capabilities() const override;
    bool supportsTechnique( Qgis::CaptureTechnique technique ) const override;

  protected:
    QgsCreateAnnotationItemMapToolHandler *mHandler = nullptr;
};

/**
 * \class QgsCreatePointTextItemMapTool
 * \ingroup gui
 *
 * \brief A map tool to digitize point text items.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsCreatePointTextItemMapTool : public QgsMapToolAdvancedDigitizing, public QgsCreateAnnotationItemMapToolInterface
{
    Q_OBJECT

  public:
    //! Constructor
    QgsCreatePointTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );
    ~QgsCreatePointTextItemMapTool() override;

    void cadCanvasPressEvent( QgsMapMouseEvent *event ) override;
    QgsCreateAnnotationItemMapToolHandler *handler() const override;
    QgsMapTool *mapTool() override;

  private:
    QgsCreateAnnotationItemMapToolHandler *mHandler = nullptr;
};


/**
 * \class QgsCreateMarkerItemMapTool
 * \ingroup gui
 *
 * \brief A map tool to digitize marker items.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsCreateMarkerItemMapTool : public QgsMapToolCaptureAnnotationItem
{
    Q_OBJECT

  public:
    //! Constructor
    QgsCreateMarkerItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    void cadCanvasReleaseEvent( QgsMapMouseEvent *event ) override;
};

/**
 * \class QgsCreateLineItemMapTool
 * \ingroup gui
 *
 * \brief A map tool to digitize line items.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsCreateLineItemMapTool : public QgsMapToolCaptureAnnotationItem
{
    Q_OBJECT

  public:
    //! Constructor
    QgsCreateLineItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

  private slots:
    void lineCaptured( const QgsCurve *line ) override;
};

/**
 * \class QgsCreatePolygonItemMapTool
 * \ingroup gui
 *
 * \brief A map tool to digitize polygon items.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsCreatePolygonItemMapTool : public QgsMapToolCaptureAnnotationItem
{
    Q_OBJECT

  public:
    //! Constructor
    QgsCreatePolygonItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

  private slots:
    void polygonCaptured( const QgsCurvePolygon *polygon ) override;
};

/**
 * \class QgsCreateRectangleTextItemMapTool
 * \ingroup gui
 *
 * \brief A map tool to digitize rectangle text items.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsCreateRectangleTextItemMapTool : public QgsMapToolAdvancedDigitizing, public QgsCreateAnnotationItemMapToolInterface
{
    Q_OBJECT

  public:
    //! Constructor
    QgsCreateRectangleTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    void cadCanvasPressEvent( QgsMapMouseEvent *event ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

    QgsCreateAnnotationItemMapToolHandler *handler() const override;
    QgsMapTool *mapTool() override;

  private:
    QgsCreateAnnotationItemMapToolHandler *mHandler = nullptr;

    QRectF mRect;
    QgsPointXY mFirstPoint;
    QObjectUniquePtr<QgsRubberBand> mRubberBand;
};

/**
 * \class QgsCreatePictureItemMapTool
 * \ingroup gui
 *
 * \brief A map tool to digitize picture items.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsCreatePictureItemMapTool : public QgsMapToolAdvancedDigitizing, public QgsCreateAnnotationItemMapToolInterface
{
    Q_OBJECT

  public:
    static inline QgsSettingsTreeNode *sTreePicture = QgsCreateAnnotationItemMapToolInterface::sTreeAnnotationTools->createChildNode( QStringLiteral( "picture-item" ) );
    static const QgsSettingsEntryString *settingLastSourceFolder;

    //! Constructor
    QgsCreatePictureItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    void cadCanvasPressEvent( QgsMapMouseEvent *event ) override;
    void cadCanvasMoveEvent( QgsMapMouseEvent *event ) override;
    void keyPressEvent( QKeyEvent *event ) override;

    QgsCreateAnnotationItemMapToolHandler *handler() const override;
    QgsMapTool *mapTool() override;

  private:
    //! Constructor
    QgsCreateAnnotationItemMapToolHandler *mHandler = nullptr;

    QRectF mRect;
    QgsPointXY mFirstPoint;
    QObjectUniquePtr<QgsRubberBand> mRubberBand;
};

/**
 * \class QgsCreateLineTextItemMapTool
 * \ingroup gui
 *
 * \brief A map tool to digitize line text items.
 *
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsCreateLineTextItemMapTool : public QgsMapToolCaptureAnnotationItem
{
    Q_OBJECT

  public:
    //! Constructor
    QgsCreateLineTextItemMapTool( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

  private slots:
    void lineCaptured( const QgsCurve *line ) override;
};


#endif // QGSCREATEANNOTATIONITEMMAPTOOLIMPL_H
