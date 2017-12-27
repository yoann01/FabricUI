// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_DFG_DFGUICmd_Paste__
#define __UI_DFG_DFGUICmd_Paste__

#include <FabricUI/DFG/DFGUICmd/DFGUICmd_Exec.h>
#include <QPoint>

FABRIC_UI_DFG_NAMESPACE_BEGIN

class DFGUICmd_Paste
  : public DFGUICmd_Exec
{
public:

  DFGUICmd_Paste(
    FabricCore::DFGBinding const &binding,
    QString execPath,
    FabricCore::DFGExec const &exec,
    QString json,
    QPointF cursorPos
    )
    : DFGUICmd_Exec(
      binding,
      execPath,
      exec
      )
    , m_json( json.trimmed() )
    , m_cursorPos( cursorPos )
  {}

  static FTL::CStrRef CmdName()
    { return DFG_CMD_NAME("Paste"); }

  QStringList getPastedItemNames()
  {
    assert( wasInvoked() );
    return m_pastedItemNames;
  }

protected:
  
  virtual void appendDesc( QString &desc );
  
  virtual void invoke( unsigned &coreUndoCount );

  void invoke(
    FTL::CStrRef json,
    unsigned &coreUndoCount
    );

private:

  QString m_json;
  QPointF m_cursorPos;

  QStringList m_pastedItemNames;
};

FABRIC_UI_DFG_NAMESPACE_END

#endif // __UI_DFG_DFGUICmd_Paste__
