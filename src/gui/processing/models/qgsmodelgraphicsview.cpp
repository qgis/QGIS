/***************************************************************************
                             qgsmodelgraphicsview.cpp
                             ----------------------------------
    Date                 : March 2020
    Copyright            : (C) 2020 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmodelgraphicsview.h"
#include "qgssettings.h"
#include "qgsmodelviewtool.h"
#include "qgsmodelviewmouseevent.h"
#include "qgsmodelviewtooltemporarykeypan.h"
#include "qgsmodelviewtooltemporarymousepan.h"
#include "qgsmodelviewtooltemporarykeyzoom.h"
#include "qgsmodelcomponentgraphicitem.h"
#include "qgsmodelgraphicsscene.h"
#include "qgsprocessingmodelcomponent.h"
#include "qgsprocessingmodelparameter.h"
#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsxmlutils.h"
#include "qgsprocessingmodelalgorithm.h"
#include <QDragEnterEvent>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QTimer>

///@cond NOT_STABLE

#define MIN_VIEW_SCALE 0.05
#define MAX_VIEW_SCALE 1000.0

QgsModelGraphicsView::QgsModelGraphicsView( QWidget *parent )
  : QGraphicsView( parent )
{
  setResizeAnchor( QGraphicsView::AnchorViewCenter );
  setMouseTracking( true );
  viewport()->setMouseTracking( true );
  setAcceptDrops( true );

  mSpacePanTool = new QgsModelViewToolTemporaryKeyPan( this );
  mMidMouseButtonPanTool = new QgsModelViewToolTemporaryMousePan( this );
  mSpaceZoomTool = new QgsModelViewToolTemporaryKeyZoom( this );

  mSnapper.setSnapToGrid( true );
}

QgsModelGraphicsView::~QgsModelGraphicsView()
{
  emit willBeDeleted();
}

void QgsModelGraphicsView::dragEnterEvent( QDragEnterEvent *event )
{
  if ( event->mimeData()->hasText() || event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.algorithmid" ) ) )
    event->acceptProposedAction();
  else
    event->ignore();
}

void QgsModelGraphicsView::dropEvent( QDropEvent *event )
{
  const QPointF dropPoint = mapToScene( event->pos() );
  if ( event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.algorithmid" ) ) )
  {
    QByteArray data = event->mimeData()->data( QStringLiteral( "application/x-vnd.qgis.qgis.algorithmid" ) );
    QDataStream stream( &data, QIODevice::ReadOnly );
    QString algorithmId;
    stream >> algorithmId;

    QTimer::singleShot( 0, this, [this, dropPoint, algorithmId ]
    {
      emit algorithmDropped( algorithmId, dropPoint );
    } );
    event->accept();
  }
  else if ( event->mimeData()->hasText() )
  {
    const QString itemId = event->mimeData()->text();
    QTimer::singleShot( 0, this, [this, dropPoint, itemId ]
    {
      emit inputDropped( itemId, dropPoint );
    } );
    event->accept();
  }
  else
  {
    event->ignore();
  }
}

void QgsModelGraphicsView::dragMoveEvent( QDragMoveEvent *event )
{
  if ( event->mimeData()->hasText() || event->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.algorithmid" ) ) )
    event->acceptProposedAction();
  else
    event->ignore();
}

void QgsModelGraphicsView::wheelEvent( QWheelEvent *event )
{
  if ( !scene() )
    return;

  if ( mTool )
  {
    mTool->wheelEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
  {
    event->accept();
    wheelZoom( event );
  }
}

void QgsModelGraphicsView::wheelZoom( QWheelEvent *event )
{
  //get mouse wheel zoom behavior settings
  QgsSettings settings;
  double zoomFactor = settings.value( QStringLiteral( "qgis/zoom_factor" ), 2 ).toDouble();

  // "Normal" mouse have an angle delta of 120, precision mouses provide data faster, in smaller steps
  zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 120.0 * std::fabs( event->angleDelta().y() );

  if ( event->modifiers() & Qt::ControlModifier )
  {
    //holding ctrl while wheel zooming results in a finer zoom
    zoomFactor = 1.0 + ( zoomFactor - 1.0 ) / 20.0;
  }

  //calculate zoom scale factor
  bool zoomIn = event->angleDelta().y() > 0;
  double scaleFactor = ( zoomIn ? 1 / zoomFactor : zoomFactor );

  //get current visible part of scene
  QRect viewportRect( 0, 0, viewport()->width(), viewport()->height() );
  QgsRectangle visibleRect = QgsRectangle( mapToScene( viewportRect ).boundingRect() );

  //transform the mouse pos to scene coordinates
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  QPointF scenePoint = mapToScene( event->pos().x(), event->pos().y() );
#else
  QPointF scenePoint = mapToScene( event->position().x(), event->position().y() );
#endif

  //adjust view center
  QgsPointXY oldCenter( visibleRect.center() );
  QgsPointXY newCenter( scenePoint.x() + ( ( oldCenter.x() - scenePoint.x() ) * scaleFactor ),
                        scenePoint.y() + ( ( oldCenter.y() - scenePoint.y() ) * scaleFactor ) );
  centerOn( newCenter.x(), newCenter.y() );

  //zoom layout
  if ( zoomIn )
  {
    scaleSafe( zoomFactor );
  }
  else
  {
    scaleSafe( 1 / zoomFactor );
  }
}

void QgsModelGraphicsView::scaleSafe( double scale )
{
  double currentScale = transform().m11();
  scale *= currentScale;
  scale = std::clamp( scale, MIN_VIEW_SCALE, MAX_VIEW_SCALE );
  setTransform( QTransform::fromScale( scale, scale ) );
}

QPointF QgsModelGraphicsView::deltaForKeyEvent( QKeyEvent *event )
{
  // increment used for cursor key item movement
  double increment = 1.0;
  if ( event->modifiers() & Qt::ShiftModifier )
  {
    //holding shift while pressing cursor keys results in a big step
    increment = 10.0;
  }
  else if ( event->modifiers() & Qt::AltModifier )
  {
    //holding alt while pressing cursor keys results in a 1 pixel step
    double viewScale = transform().m11();
    if ( viewScale > 0 )
    {
      increment = 1 / viewScale;
    }
  }

  double deltaX = 0;
  double deltaY = 0;
  switch ( event->key() )
  {
    case Qt::Key_Left:
      deltaX = -increment;
      break;
    case Qt::Key_Right:
      deltaX = increment;
      break;
    case Qt::Key_Up:
      deltaY = -increment;
      break;
    case Qt::Key_Down:
      deltaY = increment;
      break;
    default:
      break;
  }

  return QPointF( deltaX, deltaY );
}

void QgsModelGraphicsView::mousePressEvent( QMouseEvent *event )
{
  if ( !modelScene() )
    return;

  if ( mTool )
  {
    std::unique_ptr<QgsModelViewMouseEvent> me( new QgsModelViewMouseEvent( this, event, mTool->flags() & QgsModelViewTool::FlagSnaps ) );
    mTool->modelPressEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
  {
    if ( event->button() == Qt::MiddleButton )
    {
      // Pan layout with middle mouse button
      setTool( mMidMouseButtonPanTool );
      event->accept();
    }
    else
    {
      QGraphicsView::mousePressEvent( event );
    }
  }
}

void QgsModelGraphicsView::mouseReleaseEvent( QMouseEvent *event )
{
  if ( !modelScene() )
    return;

  if ( mTool )
  {
    std::unique_ptr<QgsModelViewMouseEvent> me( new QgsModelViewMouseEvent( this, event, mTool->flags() & QgsModelViewTool::FlagSnaps ) );
    mTool->modelReleaseEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseReleaseEvent( event );
}

void QgsModelGraphicsView::mouseMoveEvent( QMouseEvent *event )
{
  if ( !modelScene() )
    return;

  mMouseCurrentXY = event->pos();

  QPointF cursorPos = mapToScene( mMouseCurrentXY );
  if ( mTool )
  {
    std::unique_ptr<QgsModelViewMouseEvent> me( new QgsModelViewMouseEvent( this, event, false ) );
    if ( mTool->flags() & QgsModelViewTool::FlagSnaps )
    {
      me->snapPoint();
    }
    if ( mTool->flags() & QgsModelViewTool::FlagSnaps )
    {
      //draw snapping point indicator
      if ( me->isSnapped() )
      {
        cursorPos = me->snappedPoint();
        if ( mSnapMarker )
        {
          mSnapMarker->setPos( me->snappedPoint() );
          mSnapMarker->setVisible( true );
        }
      }
      else if ( mSnapMarker )
      {
        mSnapMarker->setVisible( false );
      }
    }
    mTool->modelMoveEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseMoveEvent( event );
}

void QgsModelGraphicsView::mouseDoubleClickEvent( QMouseEvent *event )
{
  if ( !modelScene() )
    return;

  if ( mTool )
  {
    std::unique_ptr<QgsModelViewMouseEvent> me( new QgsModelViewMouseEvent( this, event, mTool->flags() & QgsModelViewTool::FlagSnaps ) );
    mTool->modelDoubleClickEvent( me.get() );
    event->setAccepted( me->isAccepted() );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::mouseDoubleClickEvent( event );
}

void QgsModelGraphicsView::keyPressEvent( QKeyEvent *event )
{
  if ( !modelScene() )
    return;

  if ( mTool )
  {
    mTool->keyPressEvent( event );
  }

  if ( mTool && event->isAccepted() )
    return;

  if ( event->key() == Qt::Key_Space && ! event->isAutoRepeat() )
  {
    if ( !( event->modifiers() & Qt::ControlModifier ) )
    {
      // Pan layout with space bar
      setTool( mSpacePanTool );
    }
    else
    {
      //ctrl+space pressed, so switch to temporary keyboard based zoom tool
      setTool( mSpaceZoomTool );
    }
    event->accept();
  }
  else if ( event->key() == Qt::Key_Left
            || event->key() == Qt::Key_Right
            || event->key() == Qt::Key_Up
            || event->key() == Qt::Key_Down )
  {
    QgsModelGraphicsScene *s = modelScene();
    const QList<QgsModelComponentGraphicItem *> itemList = s->selectedComponentItems();
    if ( !itemList.empty() )
    {
      QPointF delta = deltaForKeyEvent( event );

      itemList.at( 0 )->aboutToChange( tr( "Move Items" ) );
      for ( QgsModelComponentGraphicItem *item : itemList )
      {
        item->moveComponentBy( delta.x(), delta.y() );
      }
      itemList.at( 0 )->changed();
    }
    event->accept();
  }
}

void QgsModelGraphicsView::keyReleaseEvent( QKeyEvent *event )
{
  if ( !modelScene() )
    return;

  if ( mTool )
  {
    mTool->keyReleaseEvent( event );
  }

  if ( !mTool || !event->isAccepted() )
    QGraphicsView::keyReleaseEvent( event );
}

void QgsModelGraphicsView::setModelScene( QgsModelGraphicsScene *scene )
{
  setScene( scene );

  // IMPORTANT!
  // previous snap markers, snap lines are owned by previous layout - so don't delete them here!
  mSnapMarker = new QgsModelViewSnapMarker();
  mSnapMarker->hide();
  scene->addItem( mSnapMarker );
}

QgsModelGraphicsScene *QgsModelGraphicsView::modelScene() const
{
  return qobject_cast< QgsModelGraphicsScene * >( QgsModelGraphicsView::scene() );
}

QgsModelViewTool *QgsModelGraphicsView::tool()
{
  return mTool;
}

void QgsModelGraphicsView::setTool( QgsModelViewTool *tool )
{
  if ( !tool )
    return;

  if ( mTool )
  {
    mTool->deactivate();
    disconnect( mTool, &QgsModelViewTool::itemFocused, this, &QgsModelGraphicsView::itemFocused );
  }

  // activate new tool before setting it - gives tools a chance
  // to respond to whatever the current tool is
  tool->activate();
  mTool = tool;
  connect( mTool, &QgsModelViewTool::itemFocused, this, &QgsModelGraphicsView::itemFocused );
  emit toolSet( mTool );
}

void QgsModelGraphicsView::unsetTool( QgsModelViewTool *tool )
{
  if ( mTool && mTool == tool )
  {
    mTool->deactivate();
    emit toolSet( nullptr );
    setCursor( Qt::ArrowCursor );
  }
}

QgsModelSnapper *QgsModelGraphicsView::snapper()
{
  return &mSnapper;
}

void QgsModelGraphicsView::startMacroCommand( const QString &text )
{
  emit macroCommandStarted( text );
}

void QgsModelGraphicsView::endMacroCommand()
{
  emit macroCommandEnded();
}

void QgsModelGraphicsView::snapSelected()
{
  QgsModelGraphicsScene *s = modelScene();
  const QList<QgsModelComponentGraphicItem *> itemList = s->selectedComponentItems();
  startMacroCommand( tr( "Snap Items" ) );
  if ( !itemList.empty() )
  {
    bool prevSetting = mSnapper.snapToGrid();
    mSnapper.setSnapToGrid( true );
    for ( QgsModelComponentGraphicItem *item : itemList )
    {
      bool wasSnapped = false;
      QRectF snapped = mSnapper.snapRectWithResize( item->mapRectToScene( item->itemRect( ) ), transform().m11(), wasSnapped );
      if ( wasSnapped )
      {
        item->setItemRect( snapped );
      }
    }
    mSnapper.setSnapToGrid( prevSetting );
  }
  endMacroCommand();
}

void QgsModelGraphicsView::copySelectedItems( QgsModelGraphicsView::ClipboardOperation operation )
{
  copyItems( modelScene()->selectedComponentItems(), operation );
}

void QgsModelGraphicsView::copyItems( const QList<QgsModelComponentGraphicItem *> &items, QgsModelGraphicsView::ClipboardOperation operation )
{
  if ( !modelScene() )
    return;

  QgsReadWriteContext context;
  QDomDocument doc;
  QDomElement documentElement = doc.createElement( QStringLiteral( "ModelComponentClipboard" ) );
  if ( operation == ClipboardCut )
  {
    emit macroCommandStarted( tr( "Cut Items" ) );
    emit beginCommand( QString() );
  }

  QList< QVariant > paramComponents;
  QList< QVariant > groupBoxComponents;
  QList< QVariant > algComponents;

  QList< QgsModelComponentGraphicItem * > selectedCommentParents;
  QList< QgsProcessingModelOutput > selectedOutputs;
  QList< QgsProcessingModelOutput > selectedOutputsComments;
  for ( QgsModelComponentGraphicItem *item : items )
  {
    if ( const QgsModelCommentGraphicItem *commentItem = dynamic_cast< QgsModelCommentGraphicItem * >( item ) )
    {
      selectedCommentParents << commentItem->parentComponentItem();
      if ( const QgsModelOutputGraphicItem *outputItem = dynamic_cast< QgsModelOutputGraphicItem * >( commentItem->parentComponentItem() ) )
      {
        selectedOutputsComments << *( static_cast< const QgsProcessingModelOutput *>( outputItem->component() ) );
      }
    }
    else if ( const QgsModelOutputGraphicItem *outputItem = dynamic_cast< QgsModelOutputGraphicItem * >( item ) )
    {
      selectedOutputs << *( static_cast< const QgsProcessingModelOutput *>( outputItem->component() ) );
    }
  }

  for ( QgsModelComponentGraphicItem *item : items )
  {
    if ( const QgsProcessingModelParameter *param = dynamic_cast< QgsProcessingModelParameter * >( item->component() ) )
    {
      QgsProcessingModelParameter component = *param;

      // was comment selected?
      if ( !selectedCommentParents.contains( item ) )
      {
        // no, so drop comment
        component.comment()->setDescription( QString() );
      }

      QVariantMap paramDef;
      paramDef.insert( QStringLiteral( "component" ), component.toVariant() );
      const QgsProcessingParameterDefinition *def = modelScene()->model()->parameterDefinition( component.parameterName() );
      paramDef.insert( QStringLiteral( "definition" ), def->toVariantMap() );

      paramComponents << paramDef;
    }
    else if ( QgsProcessingModelGroupBox *groupBox = dynamic_cast< QgsProcessingModelGroupBox * >( item->component() ) )
    {
      groupBoxComponents << groupBox->toVariant();
    }
    else if ( const QgsProcessingModelChildAlgorithm *alg = dynamic_cast< QgsProcessingModelChildAlgorithm * >( item->component() ) )
    {
      QgsProcessingModelChildAlgorithm childAlg = *alg;

      // was comment selected?
      if ( !selectedCommentParents.contains( item ) )
      {
        // no, so drop comment
        childAlg.comment()->setDescription( QString() );
      }

      // don't copy outputs which weren't selected either
      QMap<QString, QgsProcessingModelOutput> clipboardOutputs;
      const QMap<QString, QgsProcessingModelOutput> existingOutputs = childAlg.modelOutputs();
      for ( auto it = existingOutputs.constBegin(); it != existingOutputs.constEnd(); ++ it )
      {
        bool found = false;
        for ( const QgsProcessingModelOutput &candidate : selectedOutputs )
        {
          if ( candidate.childId() == childAlg.childId() && candidate.name() == it.value().name() && candidate.childOutputName() == it.value().childOutputName() )
          {
            found = true;
            break;
          }
        }
        if ( found )
        {
          // should we also copy the comment?
          bool commentFound = false;
          for ( const QgsProcessingModelOutput &candidate : selectedOutputsComments )
          {
            if ( candidate.childId() == childAlg.childId() && candidate.name() == it.value().name() && candidate.childOutputName() == it.value().childOutputName() )
            {
              commentFound = true;
              break;
            }
          }

          QgsProcessingModelOutput output = it.value();
          if ( !commentFound )
            output.comment()->setDescription( QString() );

          clipboardOutputs.insert( it.key(), output );
        }
      }
      childAlg.setModelOutputs( clipboardOutputs );

      algComponents << childAlg.toVariant();
    }
  }
  QVariantMap components;
  components.insert( QStringLiteral( "parameters" ), paramComponents );
  components.insert( QStringLiteral( "groupboxes" ), groupBoxComponents );
  components.insert( QStringLiteral( "algs" ), algComponents );
  doc.appendChild( QgsXmlUtils::writeVariant( components, doc ) );
  if ( operation == ClipboardCut )
  {
    emit deleteSelectedItems();
    emit endCommand();
    emit macroCommandEnded();
  }

  QMimeData *mimeData = new QMimeData;
  mimeData->setData( QStringLiteral( "text/xml" ), doc.toByteArray() );
  mimeData->setText( doc.toByteArray() );
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setMimeData( mimeData );
}

void QgsModelGraphicsView::pasteItems( QgsModelGraphicsView::PasteMode mode )
{
  if ( !modelScene() )
    return;

  QDomDocument doc;
  QClipboard *clipboard = QApplication::clipboard();
  if ( doc.setContent( clipboard->mimeData()->data( QStringLiteral( "text/xml" ) ) ) )
  {
    QDomElement docElem = doc.documentElement();
    QVariantMap res = QgsXmlUtils::readVariant( docElem ).toMap();

    if ( res.contains( QStringLiteral( "parameters" ) ) && res.contains( QStringLiteral( "algs" ) ) )
    {
      QPointF pt;
      switch ( mode )
      {
        case PasteModeCursor:
        case PasteModeInPlace:
        {
          // place items at cursor position
          pt = mapToScene( mapFromGlobal( QCursor::pos() ) );
          break;
        }
        case PasteModeCenter:
        {
          // place items in center of viewport
          pt = mapToScene( viewport()->rect().center() );
          break;
        }
      }

      emit beginCommand( tr( "Paste Items" ) );

      QRectF pastedBounds;

      QList< QgsProcessingModelGroupBox > pastedGroups;
      for ( const QVariant &v : res.value( QStringLiteral( "groupboxes" ) ).toList() )
      {
        QgsProcessingModelGroupBox box;
        // don't restore the uuid -- we need them to be unique in the model
        box.loadVariant( v.toMap(), true );

        pastedGroups << box;

        modelScene()->model()->addGroupBox( box );

        if ( !pastedBounds.isValid( ) )
          pastedBounds = QRectF( box.position() - QPointF( box.size().width() / 2.0, box.size().height() / 2.0 ), box.size() );
        else
          pastedBounds = pastedBounds.united( QRectF( box.position() - QPointF( box.size().width() / 2.0, box.size().height() / 2.0 ), box.size() ) );
      }

      QStringList pastedParameters;
      for ( const QVariant &v : res.value( QStringLiteral( "parameters" ) ).toList() )
      {
        QVariantMap param = v.toMap();
        QVariantMap componentDef = param.value( QStringLiteral( "component" ) ).toMap();
        QVariantMap paramDef = param.value( QStringLiteral( "definition" ) ).toMap();

        std::unique_ptr< QgsProcessingParameterDefinition > paramDefinition( QgsProcessingParameters::parameterFromVariantMap( paramDef ) );

        QgsProcessingModelParameter p;
        p.loadVariant( componentDef );

        // we need a unique name for the parameter
        QString name = p.parameterName();
        QString description = paramDefinition->description();
        int next = 1;
        while ( modelScene()->model()->parameterDefinition( name ) )
        {
          next++;
          name = QStringLiteral( "%1 (%2)" ).arg( p.parameterName() ).arg( next );
          description = QStringLiteral( "%1 (%2)" ).arg( paramDefinition->description() ).arg( next );
        }
        paramDefinition->setName( name );
        paramDefinition->setDescription( description );
        p.setParameterName( name );

        modelScene()->model()->addModelParameter( paramDefinition.release(), p );
        pastedParameters << p.parameterName();

        if ( !pastedBounds.isValid( ) )
          pastedBounds = QRectF( p.position() - QPointF( p.size().width() / 2.0, p.size().height() / 2.0 ), p.size() );
        else
          pastedBounds = pastedBounds.united( QRectF( p.position() - QPointF( p.size().width() / 2.0, p.size().height() / 2.0 ), p.size() ) );

        if ( !p.comment()->description().isEmpty() )
          pastedBounds = pastedBounds.united( QRectF( p.comment()->position() - QPointF( p.comment()->size().width() / 2.0, p.comment()->size().height() / 2.0 ), p.comment()->size() ) );
      }

      QStringList pastedAlgorithms;
      for ( const QVariant &v : res.value( QStringLiteral( "algs" ) ).toList() )
      {
        QgsProcessingModelChildAlgorithm alg;
        alg.loadVariant( v.toMap() );

        // ensure algorithm id is unique
        if ( modelScene()->model()->childAlgorithms().contains( alg.childId() ) )
        {
          alg.generateChildId( *modelScene()->model() );
        }
        alg.reattach();

        pastedAlgorithms << alg.childId();

        if ( !pastedBounds.isValid( ) )
          pastedBounds = QRectF( alg.position() - QPointF( alg.size().width() / 2.0, alg.size().height() / 2.0 ), alg.size() );
        else
          pastedBounds = pastedBounds.united( QRectF( alg.position() - QPointF( alg.size().width() / 2.0, alg.size().height() / 2.0 ), alg.size() ) );

        if ( !alg.comment()->description().isEmpty() )
          pastedBounds = pastedBounds.united( QRectF( alg.comment()->position() - QPointF( alg.comment()->size().width() / 2.0, alg.comment()->size().height() / 2.0 ), alg.comment()->size() ) );

        const QMap<QString, QgsProcessingModelChildAlgorithm> existingAlgs = modelScene()->model()->childAlgorithms();

        const QMap<QString, QgsProcessingModelOutput> outputs = alg.modelOutputs();
        QMap<QString, QgsProcessingModelOutput> pastedOutputs;
        for ( auto it = outputs.constBegin(); it != outputs.constEnd(); ++it )
        {
          QString name = it.value().name();
          int next = 1;
          bool unique = false;
          while ( !unique )
          {
            unique = true;
            for ( auto algIt = existingAlgs.constBegin(); algIt != existingAlgs.constEnd(); ++algIt )
            {
              const QMap<QString, QgsProcessingModelOutput> algOutputs = algIt->modelOutputs();
              for ( auto outputIt = algOutputs.constBegin(); outputIt != algOutputs.constEnd(); ++outputIt )
              {
                if ( outputIt.value().name() == name )
                {
                  unique = false;
                  break;
                }
              }
              if ( !unique )
                break;
            }
            if ( unique )
              break;
            next++;
            name = QStringLiteral( "%1 (%2)" ).arg( it.value().name() ).arg( next );
          }

          QgsProcessingModelOutput newOutput = it.value();
          newOutput.setName( name );
          newOutput.setDescription( name );
          pastedOutputs.insert( name, newOutput );

          pastedBounds = pastedBounds.united( QRectF( newOutput.position() - QPointF( newOutput.size().width() / 2.0, newOutput.size().height() / 2.0 ), newOutput.size() ) );

          if ( !alg.comment()->description().isEmpty() )
            pastedBounds = pastedBounds.united( QRectF( newOutput.comment()->position() - QPointF( newOutput.comment()->size().width() / 2.0, newOutput.comment()->size().height() / 2.0 ), newOutput.comment()->size() ) );
        }
        alg.setModelOutputs( pastedOutputs );

        modelScene()->model()->addChildAlgorithm( alg );
      }

      QPointF offset( 0, 0 );
      switch ( mode )
      {
        case PasteModeInPlace:
          break;

        case PasteModeCursor:
        case PasteModeCenter:
        {
          offset = pt - pastedBounds.topLeft();
          break;
        }
      }

      if ( !offset.isNull() )
      {
        for ( QgsProcessingModelGroupBox pastedGroup : std::as_const( pastedGroups ) )
        {
          pastedGroup.setPosition( pastedGroup.position() + offset );
          modelScene()->model()->addGroupBox( pastedGroup );
        }
        for ( const QString &pastedParam : std::as_const( pastedParameters ) )
        {
          modelScene()->model()->parameterComponent( pastedParam ).setPosition( modelScene()->model()->parameterComponent( pastedParam ).position() + offset );
          modelScene()->model()->parameterComponent( pastedParam ).comment()->setPosition( modelScene()->model()->parameterComponent( pastedParam ).comment()->position() + offset );
        }
        for ( const QString &pastedAlg : std::as_const( pastedAlgorithms ) )
        {
          modelScene()->model()->childAlgorithm( pastedAlg ).setPosition( modelScene()->model()->childAlgorithm( pastedAlg ).position() + offset );
          modelScene()->model()->childAlgorithm( pastedAlg ).comment()->setPosition( modelScene()->model()->childAlgorithm( pastedAlg ).comment()->position() + offset );

          const QMap<QString, QgsProcessingModelOutput> outputs = modelScene()->model()->childAlgorithm( pastedAlg ).modelOutputs();
          for ( auto it = outputs.begin(); it != outputs.end(); ++it )
          {
            modelScene()->model()->childAlgorithm( pastedAlg ).modelOutput( it.key() ).setPosition( modelScene()->model()->childAlgorithm( pastedAlg ).modelOutput( it.key() ).position() + offset );
            modelScene()->model()->childAlgorithm( pastedAlg ).modelOutput( it.key() ).comment()->setPosition( modelScene()->model()->childAlgorithm( pastedAlg ).modelOutput( it.key() ).comment()->position() + offset );
          }
        }
      }

      emit endCommand();
    }
  }

  modelScene()->rebuildRequired();
}

QgsModelViewSnapMarker::QgsModelViewSnapMarker()
  : QGraphicsRectItem( QRectF( 0, 0, 0, 0 ) )
{
  QFont f;
  QFontMetrics fm( f );
  mSize = fm.horizontalAdvance( 'X' );
  setPen( QPen( Qt::transparent, mSize ) );

  setFlags( flags() | QGraphicsItem::ItemIgnoresTransformations );
  setZValue( QgsModelGraphicsScene::ZSnapIndicator );
}

void QgsModelViewSnapMarker::paint( QPainter *p, const QStyleOptionGraphicsItem *, QWidget * )
{
  QPen pen( QColor( 255, 0, 0 ) );
  pen.setWidth( 0 );
  p->setPen( pen );
  p->setBrush( Qt::NoBrush );

  double halfSize = mSize / 2.0;
  p->drawLine( QLineF( -halfSize, -halfSize, halfSize, halfSize ) );
  p->drawLine( QLineF( -halfSize, halfSize, halfSize, -halfSize ) );
}


///@endcond


