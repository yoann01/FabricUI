//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include <FabricUI/DFG/DFGUICmd/DFGUICmd_CreatePreset.h>
#include <FabricUI/DFG/DFGUICmdHandler.h>

#include <FabricUI/Util/QString_Conversion.h>

FABRIC_UI_DFG_NAMESPACE_BEGIN

void DFGUICmd_CreatePreset::appendDesc( QString &desc )
{
  if ( m_updateOrigPreset )
    desc += "Update";
  else
    desc += "Create";
  desc += " preset '";
  desc += m_presetName;
  desc += "' from node ";
  appendDesc_NodeName( m_nodeName, desc );
}

void DFGUICmd_CreatePreset::invoke( unsigned &coreUndoCount )
{
  m_pathname =
    invoke(
      m_nodeName.toUtf8().constData(),
      m_presetDirPath.toUtf8().constData(),
      m_presetName.toUtf8().constData(),
      m_updateOrigPreset,
      coreUndoCount
      );
}

QString DFGUICmd_CreatePreset::invoke(
  FTL::CStrRef nodeName,
  FTL::CStrRef presetDirPath,
  FTL::CStrRef presetName,
  bool updateOrigPreset,
  unsigned &coreUndoCount
  )
{
  FabricCore::DFGHost host = getHost();
  FabricCore::DFGExec &exec = getExec();

  FabricCore::DFGExec subExec = exec.getSubExec( nodeName.c_str() );

  QString pathname =
    DFGUICmdHandler::NewPresetPathname(
      host,
      m_presetDirPath,
      m_presetName
      );

  if ( !subExec.isPreset() )
  {
    // [pz 20161114] I'm not sure this is necessary but can't hurt
    subExec.maybeSplitFromPreset();
    ++coreUndoCount;

    exec.promoteInstBlockExecsToDefault( nodeName.c_str() );
    ++coreUndoCount;

    subExec.setTitle( presetName.c_str() );
    ++coreUndoCount;

    FTL::CStrRef uiNodeColor =
      exec.getNodeMetadata( nodeName.c_str(), "uiNodeColor" );
    if ( !uiNodeColor.empty() )
    {
      subExec.setMetadata( "uiNodeColor", uiNodeColor.c_str() );
      ++coreUndoCount;
    }
    FTL::CStrRef uiHeaderColor =
      exec.getNodeMetadata( nodeName.c_str(), "uiHeaderColor" );
    if ( !uiHeaderColor.empty() )
    {
      subExec.setMetadata( "uiHeaderColor", uiHeaderColor.c_str() );
      ++coreUndoCount;
    }
    FTL::CStrRef uiTextColor =
      exec.getNodeMetadata( nodeName.c_str(), "uiTextColor" );
    if ( !uiTextColor.empty() )
    {
      subExec.setMetadata( "uiTextColor", uiTextColor.c_str() );
      ++coreUndoCount;
    }
    FTL::CStrRef uiTooltip =
      exec.getNodeMetadata( nodeName.c_str(), "uiTooltip" );
    if ( !uiTooltip.empty() )
    {
      subExec.setMetadata( "uiTooltip", uiTooltip.c_str() );
      ++coreUndoCount;
    }
    FTL::CStrRef uiDocUrl =
      exec.getNodeMetadata( nodeName.c_str(), "uiDocUrl" );
    if ( !uiDocUrl.empty() )
    {
      subExec.setMetadata( "uiDocUrl", uiDocUrl.c_str() );
      ++coreUndoCount;
    }
    FTL::CStrRef uiAlwaysShowDaisyChainPorts =
      exec.getNodeMetadata( nodeName.c_str(), "uiAlwaysShowDaisyChainPorts" );
    if ( !uiAlwaysShowDaisyChainPorts.empty() )
    {
      subExec.setMetadata(
        "uiAlwaysShowDaisyChainPorts",
        uiAlwaysShowDaisyChainPorts.c_str(),
        true,
        true
        );
      ++coreUndoCount;
    }

    // copy all defaults
    unsigned execPortCount = subExec.getExecPortCount();
    for ( unsigned i = 0; i < execPortCount; ++i )
    {
      std::string pinPath = nodeName;
      pinPath += ".";
      pinPath += subExec.getExecPortName(i);

      FTL::StrRef rType = exec.getNodePortResolvedType(pinPath.c_str());
      if(rType.size() == 0 || rType.find('$') != rType.end())
        rType = subExec.getExecPortResolvedType(i);
      if(rType.size() == 0 || rType.find('$') != rType.end())
        rType = subExec.getExecPortTypeSpec(i);
      if(rType.size() == 0 || rType.find('$') != rType.end())
        continue;

      if ( FabricCore::RTVal defaultValueRTVal =
        exec.getInstPortResolvedDefaultValue( pinPath.c_str(), rType.data() ) )
      {
        subExec.setPortDefaultValue(
          subExec.getExecPortName(i),
          defaultValueRTVal.copy()
          );
        ++coreUndoCount;
      }
    }

    // Important: set the preset file association BEFORE getting the JSON,
    // as with this association the Core will generate a presetGUID.
    subExec.setImportPathname( pathname.toUtf8().constData() );
    ++coreUndoCount;
  }

  if ( updateOrigPreset )
  {
    std::string presetPath;
    if ( !presetDirPath.empty() )
    {
      presetPath += presetDirPath;
      presetPath += '.';
    }
    presetPath += presetName;

    FabricCore::String presetFileGUID =
      host.getPresetFileGUID( presetPath.c_str() );
    subExec.setPresetGUID( presetFileGUID.getCStr() );
    ++coreUndoCount;
  }

  subExec.attachPresetFile(
    presetDirPath.c_str(),
    presetName.c_str(),
    true // replaceExisting
    );
  ++coreUndoCount;

  FabricCore::String presetJSON = subExec.exportJSON();
  FILE *file = fopen( QSTRING_TO_CONST_CHAR_FILE( pathname ), "wb" );
  if ( file )
  {
    fwrite( presetJSON.getCString(), presetJSON.getSize(), 1, file );
    fclose( file );
  }
  else pathname = QString();

  return pathname;
}

FABRIC_UI_DFG_NAMESPACE_END
