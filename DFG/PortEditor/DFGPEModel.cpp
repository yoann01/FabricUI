//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include <FabricUI/DFG/PortEditor/DFGPEModel.h>

namespace FabricUI {
namespace DFG {

DFGPEModel::DFGPEModel(
  QString elementDesc,
  QString elementDescCapitalized
  )
  : m_elementDesc( elementDesc )
  , m_elementDescCapitalized( elementDescCapitalized )
{
}

void DFGPEModel::reorderElementsEventually(
  QList<int> newIndices
  )
{
  new DFGPEModel_ReorderPortsEventallyTimer( this, newIndices );
}

DFGPEModel_ReorderPortsEventallyTimer::DFGPEModel_ReorderPortsEventallyTimer(
  DFGPEModel *model,
  QList<int> newIndices
  )
  : QTimer( model )
  , m_model( model )
  , m_newIndices( newIndices )
{
  setSingleShot( true );
  connect(
    this, SIGNAL(timeout()),
    this, SLOT(onTimeout())
    );
  start();
}

void DFGPEModel_ReorderPortsEventallyTimer::onTimeout()
{
  m_model->reorderElements( m_newIndices );
}

} // namespace DFG
} // namespace FabricUI
