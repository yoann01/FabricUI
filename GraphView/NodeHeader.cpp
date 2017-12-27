// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#include <FabricUI/GraphView/NodeHeader.h>
#include <FabricUI/GraphView/Node.h>
#include <FabricUI/GraphView/NodeLabel.h>
#include <FabricUI/GraphView/Graph.h>
#include <FabricUI/GraphView/Controller.h>
#include <FabricUI/GraphView/PinCircle.h>
#include <FabricUI/GraphView/NodeHeaderButton.h>

#include <QGraphicsLinearLayout>
#include <QPushButton>
#include <QGraphicsProxyWidget>

using namespace FabricUI::GraphView;

class NodeHeaderLabel : public NodeLabel
{
  NodeHeader * m_header;

public:
  NodeHeaderLabel(
    NodeHeader * header,
    Node* node,
    QString const &text,
    QColor color,
    QColor highlightColor,
    QFont font
  ) : NodeLabel(
    header,
    node,
    text,
    color,
    highlightColor,
    font
  ), m_header(header)
  {
  }

protected:
  void submitEditedText(const QString& text) FTL_OVERRIDE
  {
    Node* node = m_header->node();
    node->graph()->controller()->gvcDoRenameNode(
      node,
      text
    );
  }
};

NodeHeader::NodeHeader(
  Node * parent,
  QString const &text
  )
: ConnectionTarget(parent->mainWidget())
{
  m_node = parent;
  m_nodeButtonsHighlighted = false;
  m_inCircle = NULL;
  m_outCircle = NULL;
  Graph * graph = m_node->graph();

  setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

  QGraphicsLinearLayout * layout = new QGraphicsLinearLayout();
  float contentsMargins = graph->config().nodeHeaderContentMargins;

  setContentsMargins(0, 0, 0, 0);
  layout->setContentsMargins(0, contentsMargins + 1, 0, contentsMargins);
  layout->setSpacing(graph->config().nodeHeaderSpacing);
  layout->setOrientation(Qt::Horizontal);
  setLayout(layout);

  m_title = new NodeHeaderLabel(
    this,
    node(),
    text,
    graph->config().nodeFontColor,
    graph->config().nodeFontHighlightColor,
    graph->config().nodeFont
  );
  setEditable( node()->canEdit() );

  m_inCircle = new PinCircle(this, PortType_Input, m_node->color());
  layout->addItem(m_inCircle);
  layout->setAlignment(m_inCircle, Qt::AlignLeft | Qt::AlignVCenter);

  layout->addStretch(1);
  layout->addItem(m_title);
  layout->setAlignment(m_title, Qt::AlignHCenter | Qt::AlignVCenter);

  layout->addStretch(1);

  // add all buttons
  if(!m_node->isBackDropNode())
  {
    QStringList icons;
    icons.append("node_collapse_1.png");
    icons.append("node_collapse_2.png");
    icons.append("node_collapse_3.png");
    addHeaderButton("node_collapse", icons, (int)node()->collapsedState());
  }
    
  m_outCircle = new PinCircle(this, PortType_Output, m_node->color());
  layout->addItem(m_outCircle);
  layout->setAlignment(m_outCircle, Qt::AlignRight | Qt::AlignVCenter);

  if(!m_node->graph()->config().nodeHeaderAlwaysShowPins)
    setCirclesVisible(m_node->collapsedState() != Node::CollapseState_Expanded);
  else if(m_node->isBackDropNode())
    setCirclesVisible(false);
}

Node * NodeHeader::node()
{
  return m_node;
}

void NodeHeader::setEditable( bool canEdit )
{
  // [FE-7833] Disabled the ability to rename the Node's header label
  // since double-clicking is also used to focus the Value Editor on the Node
  labelWidget()->setEditable( false );
  //labelWidget()->setEditable( canEdit );
}

const Node * NodeHeader::node() const
{
  return m_node;
}

void NodeHeader::setTitle( QString const &title, QString const& suffix )
{
  m_title->setText(title);
  m_title->setSuffix( suffix );
}

bool NodeHeader::highlighted() const
{
  return m_title->highlighted();
}

void NodeHeader::setHighlighted(bool state)
{
  m_title->setHighlighted(state);
}

std::string NodeHeader::path() const
{
  return node()->name();
}

bool NodeHeader::canConnectTo(
  ConnectionTarget * other,
  std::string &failureReason
  ) const
{
  return other != this;
}

QPointF NodeHeader::connectionPos(PortType pType) const
{
  if(pType == PortType_Input)
  {
    if(m_inCircle)
      return m_inCircle->centerInSceneCoords();
  }
  else
  {
    if(m_outCircle)
      return m_outCircle->centerInSceneCoords();
  }
  return QPointF();
}

Graph * NodeHeader::graph()
{
  return node()->graph();
}

const Graph * NodeHeader::graph() const
{
  return node()->graph();
}

QColor NodeHeader::color() const
{
  return node()->color();
}

void NodeHeader::setColor(QColor col)
{
  if(m_inCircle)
    m_inCircle->setColor(col);
  if(m_outCircle)
    m_outCircle->setColor(col);
}

bool NodeHeader::areCirclesVisible() const
{
  return m_outCircle->isVisible();
}

void NodeHeader::setCirclesVisible(bool visible)
{
  m_inCircle->setVisible(visible);
  m_outCircle->setVisible(visible);
}

void NodeHeader::addHeaderButton(QString name, QStringList icons, int state)
{
  QGraphicsLinearLayout * lay = (QGraphicsLinearLayout *)layout();

  NodeHeaderButton * button = new NodeHeaderButton(this, name, icons, state);
  QObject::connect(button, SIGNAL(triggered(FabricUI::GraphView::NodeHeaderButton *)), 
    this, SLOT(onHeaderButtonTriggered(FabricUI::GraphView::NodeHeaderButton *)));
  lay->addItem(button);
  lay->setAlignment(button, Qt::AlignHCenter | Qt::AlignVCenter);
  m_buttons.push_back(button);
}

void NodeHeader::setHeaderButtonState(QString name, int state)
{
  for(size_t i=0;i<m_buttons.size();i++)
  {
    if(m_buttons[i]->name() == name)
      m_buttons[i]->setState(state);
  }
}

void NodeHeader::contextMenuEvent( QGraphicsSceneContextMenuEvent * event )
{
  // [FE-8415] backdrops may only respond to clicks in the header.
  if( this->node()->isBackDropNode() )
  {
    QMenu * menu = this->node()->graph()->getNodeContextMenu( this->node() );
    if ( menu )
    {
      menu->exec( QCursor::pos() );
      menu->setParent( NULL );
      menu->deleteLater();
    }
  }
  else
    Parent::contextMenuEvent( event );
}

void NodeHeader::onHeaderButtonTriggered(FabricUI::GraphView::NodeHeaderButton * button)
{
  emit headerButtonTriggered(button);
}

void NodeHeader::setNodeButtonsHighlighted(bool value)
{
  if(m_nodeButtonsHighlighted == value)
    return;
  m_nodeButtonsHighlighted = value;
  for(size_t i=0;i<m_buttons.size();i++)
    m_buttons[i]->setHighlighted(m_nodeButtonsHighlighted);
}

void NodeHeader::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
  node()->updateHighlightingFromChild( this, event->pos() );
}

void NodeHeader::hoverMoveEvent(QGraphicsSceneHoverEvent * event)
{
  node()->updateHighlightingFromChild( this, event->pos() );
}

void NodeHeader::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
  node()->updateHighlightingFromChild( this, event->pos() );
}
