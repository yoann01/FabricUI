// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_DFG_DFGUICmd_Connect__
#define __UI_DFG_DFGUICmd_Connect__

#include <FabricUI/DFG/DFGUICmd/DFGUICmd_Exec.h>

FABRIC_UI_DFG_NAMESPACE_BEGIN

class DFGUICmd_Connect
  : public DFGUICmd_Exec
{
public:

  DFGUICmd_Connect(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QStringList srcPaths,
    QStringList dstPaths
    )
    : DFGUICmd_Exec( binding, execPath, exec )
    , m_srcPaths( srcPaths )
    , m_dstPaths( dstPaths )
    {}

  static FTL::CStrRef CmdName()
    { return DFG_CMD_NAME("Connect"); }

protected:
  
  virtual void appendDesc( QString &desc );
  
  virtual void invoke( unsigned &coreUndoCount );

  void invokeAdd(
    FTL::ArrayRef<FTL::CStrRef> srcPaths,
    FTL::ArrayRef<FTL::CStrRef> dstPaths,
    unsigned &coreUndoCount
    );

private:

  QStringList m_srcPaths;
  QStringList m_dstPaths;
};

FABRIC_UI_DFG_NAMESPACE_END

#endif // __UI_DFG_DFGUICmd_Connect__
