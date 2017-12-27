// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#include "GraphViewWidget.h"

#include <QDebug>
#include <QPainter>
#include <QGLWidget>
#include <QMimeData>
#include <QRect>
#include <QGraphicsSceneEvent>

#include <FabricUI/GraphView/MainPanel.h>
#include <FabricUI/GraphView/SidePanel.h>
#include <FabricUI/GraphView/Graph.h>
#include <FabricUI/GraphView/MouseGrabber.h>

#ifdef FABRICUI_TIMERS
  #include <Util/Timer.h>
#endif

#include <stdlib.h>
#include <math.h>

using namespace FabricUI::GraphView;

GraphViewWidget::GraphViewWidget(
  QWidget * parent,
  const GraphConfig & config,
  Graph * graph
  )
  : QGraphicsView(parent)
  , m_altWasHeldAtLastMousePress( false )
  , m_uiGraphZoomBeforeQuickZoom( 0.0f )
{
  setRenderHint(QPainter::Antialiasing);
  // setRenderHint(QPainter::HighQualityAntialiasing);
  setRenderHint(QPainter::TextAntialiasing);
  setOptimizationFlag(DontSavePainterState);

  setStyleSheet( "QGraphicsView { border-style: none; }" );

  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  setViewportUpdateMode(SmartViewportUpdate);

  setAcceptDrops( true );

  // use opengl for rendering with multi sampling
  if(config.useOpenGL)
  {
    char const *useCanvasOpenGL = ::getenv( "FABRIC_USE_CANVAS_OPENGL" );
    if ( !!useCanvasOpenGL && !!useCanvasOpenGL[0] )
    {
      QGLFormat format;
      format.setSampleBuffers(true);
      QGLContext * context = new QGLContext(format);
      QGLWidget * glWidget = new QGLWidget(context);
      setViewport(glWidget);
    }
  }

  setGraph(graph);

  setMouseTracking(true);
}

Graph * GraphViewWidget::graph()
{
  return m_graph;
}

const Graph * GraphViewWidget::graph() const
{
  return m_graph;
}

void GraphViewWidget::setGraph(Graph * graph)
{
  m_scene = new GraphViewScene( graph );
  setScene(m_scene);

  QObject::connect(m_scene, SIGNAL(changed(const QList<QRectF> &)), this, SLOT(onSceneChanged()));
  QObject::connect(
    m_scene, SIGNAL(urlDropped(QUrl, bool, bool, QPointF)),
    this, SIGNAL(urlDropped(QUrl, bool, bool, QPointF))
    );

  m_graph = graph;
  if(m_graph)
  {
    m_graph->setGeometry(0, 0, size().width(), size().height());
    m_scene->addItem(m_graph);
    m_graph->updateOverlays(rect().width(), rect().height());
  }
}

void GraphViewWidget::resizeEvent(QResizeEvent * event)
{
  setSceneRect(0, 0, event->size().width(), event->size().height());
  if (m_graph)
  {
    m_graph->setGeometry(0, 0, event->size().width(), event->size().height());
    m_graph->updateOverlays(event->size().width(), event->size().height());
  }
}

void GraphViewWidget::mousePressEvent(QMouseEvent * event)
{
  m_altWasHeldAtLastMousePress =
    event->modifiers().testFlag( Qt::AltModifier );

  QGraphicsView::mousePressEvent(event);
}

void GraphViewWidget::mouseMoveEvent(QMouseEvent * event)
{
  m_lastEventPos = event->pos();
  if (getUiGraphZoomBeforeQuickZoom() > 0)
    update();
  QGraphicsView::mouseMoveEvent(event);
}

void GraphViewWidget::keyPressEvent(QKeyEvent * event)
{
  if (event->key() == Qt::Key_Escape)
  {
    MouseGrabber *mouseGrabber = graph()->getMouseGrabber();
    if (mouseGrabber)
    {
      mouseGrabber->performUngrab(NULL);
      graph()->resetMouseGrabber();
      event->accept();
      return;
    }
  }
  QGraphicsView::keyPressEvent(event);
}

void GraphViewWidget::contextMenuEvent(QContextMenuEvent * event)
{
  if ( m_altWasHeldAtLastMousePress )
  {
    // [pz 20170113] FE-7941: we don't pop up a context menu if Alt was
    // held when the right mouse button was pressed

    event->accept();
    return;
  }

  QGraphicsView::contextMenuEvent(event);
}

QPoint GraphViewWidget::lastEventPos() const
{
  return m_lastEventPos;
}

void GraphViewWidget::onSceneChanged()
{
#ifdef FABRICUI_TIMERS
  Util::TimerPtr overAllTimer = Util::Timer::getTimer("FabricUI::GraphViewWidget");

  std::map<std::string, Util::TimerPtr> & timers = Util::Timer::getAllTimers();
  std::map<std::string, Util::TimerPtr>::iterator it;
  for(it = timers.begin(); it != timers.end(); it++)
  {
    QString message;
    message += it->second->title();
    if(message.left(8) != "FabricUI")
      continue;
    double elapsed = it->second->getElapsedMS();
    if(elapsed == 0.0)
      continue;
    message += " " + QString::number(elapsed, 'g', 3);
    message += "ms";
    printf("%s\n", message.toUtf8().constData());
    it->second->reset();
  }

  overAllTimer->resume();

#endif  
  emit sceneChanged();
}

bool GraphViewWidget::focusNextPrevChild(bool next)
{
  // avoid focus switching
  return false;
}

void GraphViewWidget::drawBackground(QPainter *painter, const QRectF &exposedRect)
{
  // prepare.
  painter->save();
  GraphView::MainPanel   *mainPanel = graph()->mainPanel();
  GraphView::GraphConfig &config    = graph()->config();
  std::vector<QLineF>    &lines     = m_lines;
  QRectF rect = this->rect();
  rect.setLeft(graph()->sidePanel(GraphView::PortType_Output)->rect().right());

  // fill the background.
  if (getUiGraphZoomBeforeQuickZoom() > 0)
  {
    painter->fillRect(rect, config.mainPanelHotkeyZoomBackgroundColor);

    QPointF pos = lastEventPos();
    QSizeF size = mainPanel->canvasZoom() * rect.size() / getUiGraphZoomBeforeQuickZoom();
    QRectF zoomRect;
    zoomRect.setRect(pos.x() - 0.5f * size.width() , pos.y() - 0.5f * size.height(), size.width(), size.height());

    QPainterPath path;
    path.addRoundedRect(zoomRect, 5, 5);
    QPen pen(config.mainPanelHotkeyZoomBorderColor, 1.5f, Qt::DashLine);
    painter->setPen(pen);
    painter->fillPath(path, config.mainPanelBackgroundColor);
    painter->drawPath(path);
  }
  else
  {
    painter->fillRect(rect, config.mainPanelBackgroundColor);
  }

  // draw the grid.
  if (config.mainPanelDrawGrid)
  {
    // get the view's pan and zoom.
    QPointF pan  = mainPanel->canvasPan();
    qreal   zoom = mainPanel->canvasZoom();

    // draw the grid lines.
    qreal gridStepMin = config.mainPanelGridSpan / 4;
    qreal gridStepMax = config.mainPanelGridSpan;
    for (int pass=0;pass<2;pass++)
    {
      qreal gridStep = zoom * (pass == 0 ? 1 : 10) * config.mainPanelGridSpan;
      if (gridStep > gridStepMin)
      {
        lines.clear();
        qreal x = rect.left() + fmod(pan.rx(), gridStep);
        qreal y = rect.top()  + fmod(pan.ry(), gridStep);
        for (;x<rect.right(); x+=gridStep)  lines.push_back(QLineF(x, rect.top(), x, rect.bottom()));
        for (;y<rect.bottom();y+=gridStep)  lines.push_back(QLineF(rect.left(), y, rect.right(), y));

        // calculate the pen width for the lines
        // based on the grid step: the smaller
        // the grid step the thinner the width.
        qreal penWidth = config.mainPanelGridPen.widthF();
        if (gridStep < gridStepMax)
          penWidth *= 0.5 * (gridStep - gridStepMin) / (gridStepMax - gridStepMin);
        else if (gridStep < 10 * gridStepMax)
          penWidth *= 0.5 + 0.5 * (gridStep - gridStepMax) / (10 * gridStepMax - gridStepMax);

        // draw lines.
        painter->setPen(QPen(config.mainPanelGridPen.color(), penWidth));
        painter->drawLines(lines.data(), lines.size());
      }
    }
  }

  // clean up.
  painter->restore();
}

GraphViewScene::GraphViewScene( Graph * graph ) {
  m_graph = graph;
}

void GraphViewScene::dragEnterEvent( QGraphicsSceneDragDropEvent *event )
{
  QMimeData const *mimeData = event->mimeData();
  if ( mimeData->hasUrls() )
  {
    QList<QUrl> urls = mimeData->urls();
    if ( urls.count() == 1 )
    {
      event->acceptProposedAction();
    }
  }

  if ( !event->isAccepted() )
    QGraphicsScene::dragEnterEvent( event );
}

void GraphViewScene::dropEvent( QGraphicsSceneDragDropEvent *event )
{
  QGraphicsScene::dropEvent( event );
  
  QMimeData const *mimeData = event->mimeData();
  if ( mimeData->hasUrls() )
  {
    QList<QUrl> urls = mimeData->urls();
    if ( urls.count() == 1 )
    {
      QUrl url = urls.front();
      bool ctrlPressed = event->modifiers().testFlag( Qt::ControlModifier );
      bool altPressed = event->modifiers().testFlag( Qt::AltModifier );
      emit urlDropped( url, ctrlPressed, altPressed, event->pos() );
    }
  }
}

QPointF GraphViewWidget::mapToGraph( QPoint const &globalPos ) const
{
  MainPanel *mainPanel = m_graph->mainPanel();
  QPointF pos =
    mainPanel->mapFromScene( mapToScene( mapFromGlobal( globalPos ) ) );
  QPointF pan  = mainPanel->canvasPan();
  pos -= pan;
  if ( float zoom = mainPanel->canvasZoom() )
    pos /= zoom;
  return pos;
}
