//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include "ToolManager.h"
#include <FabricUI/Util/RTValUtil.h>
#include <FabricUI/Application/FabricException.h>
#include <FabricUI/Commands/PathValueResolverRegistry.h>
#include <FabricUI/Application/FabricApplicationStates.h>

using namespace FabricUI;
using namespace Util;
using namespace Tools;
using namespace Commands;
using namespace FabricCore;
using namespace Application;

inline RTVal pathToPathValue(
  QString const&toolPath,
  bool resolve)
{
  RTVal pathValue;

  FABRIC_CATCH_BEGIN();

  FabricApplicationStates* appStates = FabricApplicationStates::GetAppStates();
 
  RTVal toolPathVal = RTVal::ConstructString(
    appStates->getClient(), 
    toolPath.toUtf8().constData()
    );

  pathValue = RTVal::Construct(
    appStates->getClient(), 
    "PathValue", 
    1, 
    &toolPathVal);

     
  // FE-8918. Check that the type of the port RTVal is valid.
  // The port RTVal can be invalid if its type is undefined 
  // or the extension it belongs to is not loaded.
  QString type = PathValueResolverRegistry::getRegistry()->getType(pathValue);
  if(resolve && !type.isEmpty())
    PathValueResolverRegistry::getRegistry()->getValue(pathValue);
  
  FABRIC_CATCH_END("ToolManager::pathToPathValue");

  return pathValue;
}

inline RTVal pathToPathValue(
  QString const&toolPath)
{
  return pathToPathValue(
    toolPath, 
    true
    );
}

inline bool isToolRenderSetupValid()
{
  bool res = false;

  try
  {
    /// Load the extension if not done already.
    FabricApplicationStates::GetAppStates()->getClient().loadExtension("Manipulation", "", false);

    RTVal renderSetup = RTVal::Create(
      FabricApplicationStates::GetAppStates()->getContext(),
      "Tool::InlineDrawingRender::AppRenderSetup",
      0,
      0);

    res = renderSetup.callMethod(
      "Boolean",
      "isRenderEngineValid",
      0,
      0).getBoolean();
  }

  catch(FabricCore::Exception &e)
  {
    FabricUI::Application::FabricException::Throw(
      "ToolManager::isToolRenderSetupValid",
      e.getDesc_cstr(),
      "",
      FabricUI::Application::FabricException::LOG
      );
  }

  return res;
}

inline RTVal getKLToolManager()
{
  RTVal toolRegistry;

  FABRIC_CATCH_BEGIN();

  toolRegistry = RTVal::Create(
    FabricApplicationStates::GetAppStates()->getContext(),
    "Tool::AppToolsManager",
    0,
    0);

  toolRegistry = toolRegistry.callMethod(
    "Tool::ToolsManager",
    "getToolsManager",
    0,
    0);

  FABRIC_CATCH_END("ToolManager::getKLToolManager");

  return toolRegistry;
}

bool ToolManager::canCreatePathValueTool(
  QString const&toolPath)
{
 	return canCreatePathValueTool(
 		pathToPathValue(toolPath)
 		);
}

bool ToolManager::canCreatePathValueTool(
  RTVal value)
{
  if(!isToolRenderSetupValid())
    return false;

  FABRIC_CATCH_BEGIN();
  
  return getKLToolManager().callMethod(
  	"Boolean", 
  	"canCreatePathValueTool", 
  	1, 
  	&value).getBoolean();
 
  FABRIC_CATCH_END("ToolManager::canCreatePathValueTool");

  return false;
}

RTVal ToolManager::createPathValueTool(
  QString const& toolPath)
{
 	return createPathValueTool(
 		pathToPathValue(toolPath)
 		);
}

RTVal ToolManager::createPathValueTool(
  RTVal pathValue)
{
	RTVal pathValueTool;

  FABRIC_CATCH_BEGIN();
  
  if(!canCreatePathValueTool(pathValue))
    FabricException::Throw(
      "ToolManager::createPathValueTool",
      "Cannot createPathValueTool a tool for un-register type '" + 
      RTValUtil::getType(RTValUtil::toRTVal(pathValue).maybeGetMember("value")) + "'"
      );
   
  pathValueTool = getKLToolManager().callMethod(
    "Tool::BaseTool",
    "createPathValueTool",
    1,
    &pathValue);
 	
  FABRIC_CATCH_END("ToolManager::createPathValueTool");

  return pathValueTool;
}

RTVal ToolManager::getPathValueTool(
  QString const& toolPath)
{
  return getPathValueTool(
    pathToPathValue(toolPath)
    );
}

RTVal ToolManager::getPathValueTool(
  RTVal pathValue)
{
  RTVal pathValueTool;

  FABRIC_CATCH_BEGIN();
 
  pathValueTool = getKLToolManager().callMethod(
    "Tool::BaseTool",
    "getPathValueTool",
    1,
    &pathValue);
  
  FABRIC_CATCH_END("ToolManager::getPathValueTool");

  return pathValueTool;
}

void ToolManager::deleteAllPathValueTools()
{
  FABRIC_CATCH_BEGIN();
 
  getKLToolManager().callMethod(
    "", 
    "deleteAllPathValueTools", 
    0, 
    0);
  
  FABRIC_CATCH_END("ToolManager::deleteAllPathValueTools");
}

void ToolManager::deletePathValueTool(
  QString const& toolPath)
{
  deletePathValueTool(
    pathToPathValue(toolPath, false)
    );
}

void ToolManager::deletePathValueTool(
	FabricCore::RTVal pathValue)
{
  FABRIC_CATCH_BEGIN();
 
  getKLToolManager().callMethod(
  	"", 
  	"deletePathValueTool", 
  	1, 
  	&pathValue);
  
  FABRIC_CATCH_END("ToolManager::deletePathValueTool");
}

void ToolManager::renamePathValueToolPath(
	QString const& oldToolPath,
	QString const& newToolPath)
{
  renamePathValueToolPath(
    pathToPathValue(oldToolPath, false),
    pathToPathValue(newToolPath, false)   
    );
}

void ToolManager::renamePathValueToolPath(
  FabricCore::RTVal oldPathValue,
  FabricCore::RTVal newPathValue)
{
  FABRIC_CATCH_BEGIN();
 
  RTVal args[2] = { 
    oldPathValue,
    newPathValue
  };

  getKLToolManager().callMethod(
    "", 
    "renamePathValueToolPath", 
    2, 
    args);   

  FABRIC_CATCH_END("ToolManager::renamePathValueToolPath");
}

void ToolManager::setPathValueToolValue(
	QString const& toolPath)
{
  setPathValueToolValue(
    pathToPathValue(toolPath)
    );
}

void ToolManager::setPathValueToolValue(
  FabricCore::RTVal pathValue)
{
  FABRIC_CATCH_BEGIN();

  getKLToolManager().callMethod(
    "",
    "setPathValueToolValue",
    1,
    &pathValue);

  FABRIC_CATCH_END("ToolManager::setPathValueToolValue");
}
