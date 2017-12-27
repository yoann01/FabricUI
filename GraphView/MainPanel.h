// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_GraphView_MainPanel__
#define __UI_GraphView_MainPanel__

#include <FTL/Config.h>

#include <QGraphicsWidget>
#include <QPen>
#include <QColor>
#include <vector>

namespace FabricUI
{

  namespace GraphView
  {
    // forward declarations
    class Graph;
    class Node;
    class SelectionRect;

    class MainPanel : public QGraphicsWidget
    {
      Q_OBJECT

    public:

      enum ManipulationMode
      {
        ManipulationMode_None,
        ManipulationMode_Select,
        ManipulationMode_Pan,
        ManipulationMode_Zoom
      };

      MainPanel(Graph * parent);
      virtual ~MainPanel() {}

      Graph * graph();
      const Graph * graph() const;
      QGraphicsWidget * itemGroup();
      const QGraphicsWidget * itemGroup() const;

      float canvasZoom() const;
      QPointF canvasPan() const;
 
      float mouseWheelZoomRate() const;
      void setMouseWheelZoomRate(float rate);
      ManipulationMode manipulationMode() const;
      void setManipulationMode(ManipulationMode mode);

      virtual QRectF boundingRect() const;
      virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
      virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * event);
      virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
      virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
      virtual void wheelEvent(QGraphicsSceneWheelEvent * event);
      virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget);
      virtual void resizeEvent(QGraphicsSceneResizeEvent * event);

      virtual void dragEnterEvent( QGraphicsSceneDragDropEvent *event );
      virtual void dropEvent( QGraphicsSceneDragDropEvent *event );

      // used by controller
      void setCanvasZoom(float state, bool quiet = false);
      void setCanvasPan(QPointF pos, bool quiet = false);

      // returns true if the children of the MainPanel should ignore the event
      // because the MainPanel will use it (to pan, for example)
      // TODO : refactor so that the children QGraphicsItems don't have to call this function
      static bool filterMousePressEvent( const QGraphicsSceneMouseEvent * event );

      void performZoom(
        float zoomFactor,
        QPointF zoomCenter
        );

    signals:

      void canvasZoomChanged(float zoom);
      void canvasPanChanged(QPointF pos);
      void doubleClicked(Qt::KeyboardModifiers);

    protected:

      void contextMenuEvent( QGraphicsSceneContextMenuEvent * event ) FTL_OVERRIDE;

    private:

      static const float s_minZoom;
      static const float s_maxZoom;
      static const float s_minZoomForOne;
      static const float s_maxZoomForOne;

      Graph * m_graph;
      float m_mouseWheelZoomRate;
      float m_mouseAltZoomState;
      float m_mouseWheelZoomState;
      ManipulationMode m_manipulationMode;
      QGraphicsWidget * m_itemGroup;
      QPointF m_lastPanPoint;
      SelectionRect * m_selectionRect;
      std::vector<Node*> m_ongoingSelection;
      QRectF m_boundingRect;
    };

  };

};

#endif // __UI_GraphView_MainPanel__
