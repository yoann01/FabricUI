// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_GraphView_SidePanelItemGroup__
#define __UI_GraphView_SidePanelItemGroup__

#include <QGraphicsWidget>
#include <QGraphicsSceneResizeEvent>

namespace FabricUI
{

  namespace GraphView
  {
    class SidePanelItemGroup : public QGraphicsWidget
    {
      Q_OBJECT

    public:

      SidePanelItemGroup(QGraphicsWidget * parent);

    signals:
      void resized();

    protected:
      virtual void resizeEvent(QGraphicsSceneResizeEvent * event);
    };

  };

};

#endif // __UI_GraphView_SidePanelItemGroup__
