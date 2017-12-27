//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include <FabricUI/DFG/DFGUICmdHandler.h>
#include <FabricUI/DFG/DFGWidget.h>
#include <FabricUI/DFG/PortEditor/DFGPEModel_ExecPorts.h>
#include <FTL/JSONValue.h>

namespace FabricUI {
namespace DFG {

DFGPEModel_ExecPorts::DFGPEModel_ExecPorts(
  DFGUICmdHandler *cmdHandler,
  FabricCore::DFGBinding binding,
  FTL::StrRef execPath,
  FabricCore::DFGExec exec,
  QSharedPointer<DFG::DFGExecNotifier> execNotifier
  )
  : DFGPEModel( "port", "Port" )
  , m_cmdHandler( cmdHandler )
  , m_binding( binding )
  , m_execPathQS( QString::fromUtf8( execPath.data(), execPath.size() ) )
  , m_exec( exec )
  , m_notifier( execNotifier )
{
  connect(
    m_notifier.data(), SIGNAL(editWouldSplitFromPresetMayHaveChanged()),
    this, SLOT(onEditWouldSplitFromPresetMayHaveChanged())
    );
  connect(
    m_notifier.data(), SIGNAL(execPortInserted(unsigned, FTL::CStrRef, FTL::JSONObject const *)),
    this, SLOT(onExecPortInserted(unsigned, FTL::CStrRef, FTL::JSONObject const *))
    );
  connect(
    m_notifier.data(), SIGNAL(execPortRenamed(unsigned, FTL::CStrRef, FTL::CStrRef)),
    this, SLOT(onExecPortRenamed(unsigned, FTL::CStrRef, FTL::CStrRef))
    );
  connect(
    m_notifier.data(), SIGNAL(execPortTypeChanged(unsigned, FTL::CStrRef, FabricCore::DFGPortType)),
    this, SLOT(onExecPortTypeChanged(unsigned, FTL::CStrRef, FabricCore::DFGPortType))
    );
  connect(
    m_notifier.data(), SIGNAL(execPortTypeSpecChanged(unsigned, FTL::CStrRef, FTL::CStrRef)),
    this, SLOT(onExecPortTypeSpecChanged(unsigned, FTL::CStrRef, FTL::CStrRef))
    );
  connect(
    m_notifier.data(), SIGNAL(execPortRemoved(unsigned, FTL::CStrRef)),
    this, SLOT(onExecPortRemoved(unsigned, FTL::CStrRef))
    );
  connect(
    m_notifier.data(), SIGNAL(execPortsReordered(FTL::ArrayRef<unsigned>)),
    this, SLOT(onExecPortsReordered(FTL::ArrayRef<unsigned>))
    );

  init();
}

bool DFGPEModel_ExecPorts::hasPortType()
{
  return !m_exec.isInstBlockExec();
}

bool DFGPEModel_ExecPorts::computeIsReadOnly()
{
  return m_exec.editWouldSplitFromPreset();
}

int DFGPEModel_ExecPorts::getElementCount()
{
  return m_exec.getExecPortCount();
}

QString DFGPEModel_ExecPorts::getElementName( int index )
{
  FTL::CStrRef result = m_exec.getExecPortName( index );
  return QString::fromUtf8( result.data(), result.size() );
}

FabricCore::DFGPortType DFGPEModel_ExecPorts::getElementPortType( int index )
{
  return m_exec.getExecPortType( index );
}

QString DFGPEModel_ExecPorts::getElementTypeSpec( int index )
{
  FTL::CStrRef result = m_exec.getExecPortTypeSpec( index );
  return QString::fromUtf8( result.data(), result.size() );
}

bool DFGPEModel_ExecPorts::isElementReadOnlyImpl( int index )
{
  return m_exec.isDepsExecPort( index );
}

void DFGPEModel_ExecPorts::insertElement(
  int index,
  QString desiredPortName,
  FabricCore::DFGPortType portType,
  QString portTypeSpec
  )
{
  m_cmdHandler->dfgDoAddPort(
    m_binding,
    m_execPathQS,
    m_exec,
    desiredPortName,
    portType,
    portTypeSpec,
    QString(), // portToConnect
    QString(), // extDep
    QString() // metaData
    );
}

void DFGPEModel_ExecPorts::inspectElement(
  int index,
  DFGWidget *dfgWidget
  )
{
  dfgWidget->editPort( m_exec.getExecPortName( index ), false /* duplicatePort */ );
}

void DFGPEModel_ExecPorts::renameElement(
  int index,
  QString newPortName
  )
{
  QString oldPortName = getElementName( index );

  m_cmdHandler->dfgDoEditPort(
    m_binding,
    m_execPathQS,
    m_exec,
    oldPortName,
    newPortName,
    getElementPortType( index ),
    getElementTypeSpec( index ),
    QString(), // extDep
    QString() // uiMetadata
    );
}

void DFGPEModel_ExecPorts::setElementPortType(
  int index,
  FabricCore::DFGPortType portType
  )
{
  QString portName = getElementName( index );

  m_cmdHandler->dfgDoEditPort(
    m_binding,
    m_execPathQS,
    m_exec,
    portName,
    portName,
    portType,
    getElementTypeSpec( index ),
    QString(), // extDep
    QString() // uiMetadata
    );
}

void DFGPEModel_ExecPorts::setElementTypeSpec(
  int index,
  QString newPortTypeSpec
  )
{
  QString portName = getElementName( index );

  m_cmdHandler->dfgDoEditPort(
    m_binding,
    m_execPathQS,
    m_exec,
    portName,
    portName,
    getElementPortType( index ),
    newPortTypeSpec,
    QString(), // extDep
    QString() // uiMetadata
    );
}

void DFGPEModel_ExecPorts::removeElements(
  QList<int> indices
  )
{
  qSort( indices );
  for ( int i = indices.size(); i--; )
  {
    int index = indices[i];
    m_cmdHandler->dfgDoRemovePort(
      m_binding,
      m_execPathQS,
      m_exec,
      QStringList( getElementName( index ) )
      );
  }
}

void DFGPEModel_ExecPorts::reorderElements(
  QList<int> newIndices
  )
{
  m_cmdHandler->dfgDoReorderPorts(
    m_binding,
    m_execPathQS,
    m_exec,
    QString(), // itemPath
    newIndices
    );
}

void DFGPEModel_ExecPorts::onEditWouldSplitFromPresetMayHaveChanged()
{
  updateIsReadOnly();
}

static FabricCore::DFGPortType PortTypeStrToDFGPortType( FTL::StrRef portTypeStr )
{
  if ( portTypeStr == FTL_STR("Out") )
    return FabricCore::DFGPortType_Out;
  else if ( portTypeStr == FTL_STR("IO") )
    return FabricCore::DFGPortType_IO;
  else
    return FabricCore::DFGPortType_In;
}

void DFGPEModel_ExecPorts::onExecPortInserted(
  unsigned portIndex,
  FTL::CStrRef portName,
  FTL::JSONObject const *portDesc
  )
{
  FTL::StrRef typeSpec = portDesc->getStringOrEmpty( FTL_STR("typeSpec") );
  emit elementInserted(
    portIndex,
    QString::fromUtf8( portName.data(), portName.size() ),
    PortTypeStrToDFGPortType( portDesc->getStringOrEmpty( FTL_STR("execPortType") ) ),
    QString::fromUtf8( typeSpec.data(), typeSpec.size() )
    );
}

void DFGPEModel_ExecPorts::onExecPortRenamed(
  unsigned portIndex,
  FTL::CStrRef oldPortName,
  FTL::CStrRef newPortName
  )
{
  emit elementRenamed(
    portIndex,
    QString::fromUtf8( newPortName.data(), newPortName.size() )
    );
}

void DFGPEModel_ExecPorts::onExecPortTypeChanged(
  unsigned portIndex,
  FTL::CStrRef portName,
  FabricCore::DFGPortType newPortType
  )
{
  emit elementPortTypeChanged( portIndex, newPortType );
}

void DFGPEModel_ExecPorts::onExecPortTypeSpecChanged(
  unsigned portIndex,
  FTL::CStrRef portName,
  FTL::CStrRef newPortTypeSpec
  )
{
  emit elementTypeSpecChanged(
    portIndex,
    QString::fromUtf8( newPortTypeSpec.data(), newPortTypeSpec.size() )
    );
}

void DFGPEModel_ExecPorts::onExecPortRemoved(
  unsigned portIndex,
  FTL::CStrRef portName
  )
{
  emit elementRemoved( portIndex );
}

void DFGPEModel_ExecPorts::onExecPortsReordered(
  FTL::ArrayRef<unsigned> newOrder
  )
{
  QList<int> newIndices;
  for ( size_t i = 0; i < newOrder.size(); ++i )
    newIndices.push_back( newOrder[i] );
  emit elementsReordered( newIndices );
}

} // namespace DFG
} // namespace FabricUI
