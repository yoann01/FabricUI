//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include "DFGPathValueResolver.h"
#include <FabricUI/Util/RTValUtil.h>
#include <FabricUI/DFG/DFGController.h>
#include <FabricUI/Application/FabricException.h>
#include <FabricUI/Application/FabricApplicationStates.h>
#include <FabricServices/Persistence/RTValToJSONEncoder.hpp>
#include <iostream>

using namespace FabricUI;
using namespace DFG;
using namespace Util;
using namespace Commands;
using namespace FabricCore;
using namespace Application;
 
DFGPathValueResolver::DFGPathValueResolver()
  : BasePathValueResolver()
  , m_id("")
{
}
 
DFGPathValueResolver::~DFGPathValueResolver()
{
}
 
void DFGPathValueResolver::registrationCallback(
  QString const&name,
  void *userData)
{
  if(userData != 0)
  {
    DFGController* controller = static_cast<DFGController*>(userData);
    onBindingChanged(controller->getBinding());
  
    QObject::connect(
      controller,
      SIGNAL(bindingChanged(FabricCore::DFGBinding const &)),
      this,
      SLOT(onBindingChanged(FabricCore::DFGBinding const &))
      );
  }
}

void DFGPathValueResolver::onBindingChanged(
  DFGBinding const &binding)
{
  m_binding = binding;
  m_id = binding.getMetadata("resolver_id");
}

bool DFGPathValueResolver::knownPath(
  RTVal pathValue)
{
  QString path = RTValUtil::toRTVal(pathValue).maybeGetMember(
    "path").getStringCString();
  
  bool knownBinding = false;

  int index = path.indexOf(".");
  if(index != -1)
  {
    QString id = path.mid(0, index);
    if(id == QString::number(m_binding.getBindingID()) || id == m_id)
      knownBinding = true;
  }
  knownBinding = !knownBinding ? m_id.isEmpty() : true;

  if(knownBinding)
  {
    DFGPortPaths dfgPortPaths;
    DFGType dfgType = DFGUnknow;

    getDFGPortPathsAndType(
      pathValue,
      dfgPortPaths,
      dfgType
      );

    return dfgType != DFGUnknow;
  }

  return false;
}

QString DFGPathValueResolver::getType(
  RTVal pathValue)
{
  QString type;

  FABRIC_CATCH_BEGIN();

  DFGPortPaths dfgPortPaths;
  DFGType dfgType = DFGUnknow;

  FabricCore::DFGExec subExec = getDFGPortPathsAndType(
    pathValue,
    dfgPortPaths,
    dfgType
    );

  if(dfgType == DFGVar)
    type = m_binding.getExec().getVarType(
      dfgPortPaths.getAbsolutePortPath(false).toUtf8().constData()
      );
  
  else
    type = subExec.getPortResolvedType(
      dfgPortPaths.getRelativePortPath().toUtf8().constData()
      );

  FABRIC_CATCH_END("DFGPathValueResolver::getType");

  return type;
}

void DFGPathValueResolver::getValue(
  RTVal pathValue)
{
  FABRIC_CATCH_BEGIN();
    
  RTVal value;

  DFGPortPaths dfgPortPaths;
  DFGType dfgType = DFGUnknow;

  FabricCore::DFGExec subExec = getDFGPortPathsAndType(
    pathValue,
    dfgPortPaths,
    dfgType
    );

  if(dfgType == DFGVar)
    value = m_binding.getExec().getVarValue(
      dfgPortPaths.getAbsolutePortPath(false).toUtf8().constData()
      );

  else 
  {
    try
    {
      if(dfgType == DFGPort)
        value = subExec.getPortResolvedDefaultValue( 
          dfgPortPaths.getRelativePortPath().toUtf8().constData(), 
          subExec.getPortResolvedType(dfgPortPaths.getRelativePortPath().toUtf8().constData())
          );

      else if(dfgType == DFGArg)
        value = m_binding.getArgValue(
          dfgPortPaths.getRelativePortPath().toUtf8().constData()
          );
    }

    catch(FabricCore::Exception &e)
    {
      FabricException::Throw(
        "DFGPathValueResolver::arePathValueAndDFGItemTypeEqual",
        "The port value at path '" + dfgPortPaths.getAbsolutePortPath() + "' is undefined"
        );
    }
  }
  
  if(value.isValid())
  {
    if(value.isArray() && dfgPortPaths.isArrayElement())
    {
      RTVal valueElet = value.getArrayElementRef(dfgPortPaths.arrayIndex);
      pathValue.setMember("value", valueElet);

    }
    else
    pathValue.setMember("value", value);
  }
  
  else
  {
    QString type = RTValUtil::getType(value);
    QString path = RTValUtil::toRTVal(pathValue).maybeGetMember(
      "path").getStringCString();

    FabricException::Throw(
      "DFGPathValueResolver::getValue",
      "Invalid data at path : " + path + ", data type : " + type.toUtf8()
      );
  }

  castPathToHRFormat( pathValue);

  FABRIC_CATCH_END("DFGPathValueResolver::getValue");
}

inline void arePathValueAndDFGItemTypeEqual(
  QString const &dfgItemPath,
  QString const &dfgItemType,
  QString const &pvValueType, // PathValue's value type
  QString const &dfgValueType) // From a port, arg or var
{ 
  Client client = FabricApplicationStates::GetAppStates()->getClient();
 
  if(pvValueType != "None" && !client.areTypesEqual(pvValueType.toUtf8().constData(), dfgValueType.toUtf8().constData()))
    FabricException::Throw(
      "DFGPathValueResolver::arePathValueAndDFGItemTypeEqual",
      "Cannot set " + dfgItemType + " at path '" + dfgItemPath + "'",
      "The type of the PathValue's value '" + pvValueType + "' and " + dfgItemType + " '" + dfgValueType + "' are not equal"
      );
}

void DFGPathValueResolver::setValue(
  RTVal pathValue)
{
  FABRIC_CATCH_BEGIN();

  RTVal value = RTValUtil::toRTVal(
    RTValUtil::toRTVal(pathValue).maybeGetMember("value"));

  if( !value.isValid() )
    return; // no value specified

  DFGPortPaths dfgPortPaths;
  DFGType dfgType = DFGUnknow;

  FabricCore::DFGExec subExec = getDFGPortPathsAndType(
    pathValue,
    dfgPortPaths,
    dfgType
    );

  if( ( value.isObject() || value.isInterface() ) && !value.isNullObject() ) {
    // Get the most specialized type.
    // In particular, port values must be persisted, and loading a graph
    // will first create an object of the type of the port.
    // For this to work well, it must be the actual specialized Object type
    // that is set as the default value.
    RTVal specializedTypeName = value.invokeMethod( "type", 0, 0 ).getDesc();
    RTVal specializedValue = RTVal::Create(
                  value.getContext(),
                  specializedTypeName.getStringCString(),
                  1, &value );
    value = specializedValue;
  }

  QString pvValueType = RTValUtil::getType(value).toUtf8().constData();

  if(dfgType == DFGVar)
  { 
    if(dfgPortPaths.isArrayElement())
    {
      RTVal arrayVal = m_binding.getExec().getVarValue(
        dfgPortPaths.getAbsolutePortPath(false).toUtf8().constData()
        );

      arrayVal.setArrayElement(
        dfgPortPaths.arrayIndex,
        value
        );

      m_binding.getExec().setVarValue( 
        dfgPortPaths.getAbsolutePortPath(false).toUtf8().constData(), 
        arrayVal);
    }

    else
    {
      QString varValueType = m_binding.getExec().getVarType(
        dfgPortPaths.getAbsolutePortPath(false).toUtf8().constData()
        );

      arePathValueAndDFGItemTypeEqual(
        dfgPortPaths.getAbsolutePortPath(),
        "variable",
        pvValueType,
        varValueType
        );

      m_binding.getExec().setVarValue( 
        dfgPortPaths.getAbsolutePortPath(false).toUtf8().constData(), 
        value);
    }
  }

  else
  {
    if(!dfgPortPaths.isArrayElement())
    {
      QString portValueType = subExec.getPortResolvedType(
        dfgPortPaths.getRelativePortPath().toUtf8().constData()
        );

      arePathValueAndDFGItemTypeEqual(
        dfgPortPaths.getAbsolutePortPath(),
        "port",
        pvValueType,
        portValueType
        );
    }

    if(dfgType == DFGPort)
    {
      if(dfgPortPaths.isArrayElement())
      {
        RTVal arrayVal = subExec.getPortResolvedDefaultValue( 
          dfgPortPaths.getRelativePortPath().toUtf8().constData(), 
          subExec.getPortResolvedType(dfgPortPaths.getRelativePortPath().toUtf8().constData())
          );

        arrayVal.setArrayElement(
          dfgPortPaths.arrayIndex,
          value);

        subExec.setPortDefaultValue( 
          dfgPortPaths.getRelativePortPath().toUtf8().constData(), 
          arrayVal, 
          false);
      }

      else
        subExec.setPortDefaultValue( 
          dfgPortPaths.getRelativePortPath().toUtf8().constData(), 
          value, 
          false);
    }
 
    else if( dfgType == DFGArg )
    {
 
      { // Code copy-pasted from FabricUI/DFG/DFGUICmd_SetArgValue.cpp :

        // Automatically set as "persistable" arg values that were explicitly set by the user
        // NOTE: metadata additions are not properly undone, however in this case it's not a big issue
        //       since we mostly want to avoid "too big values" like meshes to be persisted, which
        //       shouldn't be the case here.
        m_binding.getExec().setExecPortMetadata(
          dfgPortPaths.getRelativePortPath().toUtf8().constData(),
          DFG_METADATA_UIPERSISTVALUE,
          "true",
          false
        );
      }
 
      if(dfgPortPaths.isArrayElement())
      {
        RTVal arrayVal = m_binding.getArgValue(
          dfgPortPaths.getRelativePortPath().toUtf8().constData()
          );

        arrayVal.setArrayElement(
          dfgPortPaths.arrayIndex,
          value);

        m_binding.setArgValue(
          dfgPortPaths.getRelativePortPath().toUtf8().constData(),
          arrayVal,
          false );
      }

      else
        m_binding.setArgValue(
          dfgPortPaths.getRelativePortPath().toUtf8().constData(),
          value,
          false );
    }   
  }

  FABRIC_CATCH_END("DFGPathValueResolver::setValue");
}

QString DFGPathValueResolver::getPathWithoutBindingOrSolverID(
  RTVal pathValue,
  int &arrayIndex,
  bool removeArrayElement)
{
  QString path;

  FABRIC_CATCH_BEGIN();

  arrayIndex = -1;

  path = RTValUtil::toRTVal(pathValue).maybeGetMember(
    "path").getStringCString();

  int index = path.indexOf(".");
  if(index != -1)
  {
    QString id = path.mid(0, index);
    if(id == QString::number(m_binding.getBindingID()) || id == m_id)
      path = path.mid(index+1);
  }
  
  if(removeArrayElement)
  {
    index = path.lastIndexOf("[");
    if(index != -1)
    {
      arrayIndex = path.mid(index+1, 1).toInt();
      path = path.mid(0, index);
    }
  }
 
  FABRIC_CATCH_END("DFGPathValueResolver::getPathWithoutBindingOrSolverID");

  return path;
}

FabricCore::DFGExec DFGPathValueResolver::getDFGPortPathsAndType(
  FabricCore::RTVal pathValue,
  DFGPortPaths &dfgPortPaths,
  DFGPathValueResolver::DFGType &dfgType)
{
  DFGExec exec;
  dfgType = DFGUnknow;

  FABRIC_CATCH_BEGIN();
  
  exec = getDFGPortPaths(
    pathValue,
    dfgPortPaths);

  // Vars
  if(m_binding.getExec().hasVar(dfgPortPaths.getAbsolutePortPath(false).toUtf8().constData()))
    dfgType = DFGVar;

  // Args or Ports
  else
  {
    if(dfgPortPaths.isExecArg() && exec.haveExecPort(dfgPortPaths.portName.toUtf8().constData()))
      dfgType = DFGArg; 

    else
    {
      DFGExec subExec;

      if(dfgPortPaths.isExecBlockPort())
        subExec = exec.getInstBlockExec(
          dfgPortPaths.nodeName.toUtf8().constData(), 
          dfgPortPaths.blockName.toUtf8().constData()
          );

      else
        subExec = exec.getSubExec(dfgPortPaths.nodeName.toUtf8().constData());

      if(subExec.haveExecPort(dfgPortPaths.portName.toUtf8().constData()))
        dfgType = DFGPort;
    }
  }

  FABRIC_CATCH_END("DFGPathValueResolver::getDFGPortPathsAndType");

  return exec;
}
 
DFGExec DFGPathValueResolver::getDFGPortPaths(
  RTVal pathValue, 
  DFGPortPaths &dfgPortPaths)  
{
  DFGExec exec;
  FABRIC_CATCH_BEGIN();

  int arrayIndex = 1;
  QString path = getPathWithoutBindingOrSolverID(
    pathValue, 
    arrayIndex
    );

  dfgPortPaths.id = m_id;
  dfgPortPaths.arrayIndex = arrayIndex;

  int index = path.lastIndexOf(".");

  QString subExecPath = ".";

  // Port (if Inst or Block)
  if(index != -1)
  {
    QString nodePath = path.mid(0, index);
    dfgPortPaths.portName = path.mid(index+1);

    index = nodePath.lastIndexOf(".");
    if(index != -1)
      subExecPath = nodePath.mid(0, index);

    // test for Block
    QString blockName, nodeName, execBlockPath;

    // Ports of a block exec have the structure
    // ...nodeName.blockName.portName
    if(nodePath.count(".") >= 2)
    {
      index = subExecPath.lastIndexOf(".");
      if(index != -1)
        execBlockPath = subExecPath.mid(0, index);

      index = nodePath.lastIndexOf(".");
      blockName = nodePath.mid(index+1);
      QString temp = nodePath.mid(0, index);
      
      index = temp.lastIndexOf(".");
      nodeName = temp.mid(index+1);
    }

    else
    { 
      index = nodePath.lastIndexOf(".");
      blockName = nodePath.mid(index+1);
      nodeName = nodePath.mid(0, index);
    }
      
    // Block Inst
    exec = m_binding.getExec().getSubExec(
      execBlockPath.toUtf8().constData()
      );

    bool isBlock = false;

    // FE-8759
    try
    {
      isBlock = (!exec.isExecBlock(nodeName.toUtf8().constData()) && exec.isInstBlock(
          nodeName.toUtf8().constData(), 
          blockName.toUtf8().constData()));
      // {
      //   dfgPortPaths.blockName = blockName;
      //   dfgPortPaths.nodeName = nodeName;
      // }

    }

    catch(FabricCore::Exception &e)
    {
    }

    if(isBlock)
    {
      dfgPortPaths.blockName = blockName;
      dfgPortPaths.nodeName = nodeName;
    }

    // Inst
    else
    {
      index = nodePath.lastIndexOf(".");
      dfgPortPaths.nodeName = nodePath.mid(index+1);
      exec = m_binding.getExec().getSubExec(
        subExecPath.toUtf8().constData()
        );
    }
  }

  // Arg
  else
  {
    exec = m_binding.getExec();
    dfgPortPaths.portName = path;
  }

  // std::cout 
  //   << "DFGPathValueResolver::getDFGPortPaths " 
  //   << " portName "
  //   << dfgPortPaths.portName.toUtf8().constData()
  //   << " blockName "
  //   << dfgPortPaths.blockName.toUtf8().constData()
  //   << " nodeName "
  //   << dfgPortPaths.nodeName.toUtf8().constData()
  //   << " subExecPath "
  //   << subExecPath.toUtf8().constData()
  //   << " block "
  //   << dfgPortPaths.isExecBlockPort()
  //   << std::endl;

  FabricCore::String execPath = exec.getExecPath();
  dfgPortPaths.execPath = QString(std::string(execPath.getCStr(), execPath.getSize()).c_str());

  FABRIC_CATCH_END("DFGPathValueResolver::getDFGPortPaths");

  return exec;
}

DFGBinding DFGPathValueResolver::getDFGBinding() const 
{
  return m_binding;
}

void DFGPathValueResolver::castPathToHRFormat(
  FabricCore::RTVal pathValue)
{
  FABRIC_CATCH_BEGIN();

  int arrayIndex;
  QString path = getPathWithoutBindingOrSolverID(
    pathValue, 
    arrayIndex,
    false
    );

  path = !m_id.isEmpty() ? m_id + "." + path : path;

  RTVal pathVal = RTVal::ConstructString(
    pathValue.getContext(),
    path.toUtf8().constData()
    );
  
  pathValue = RTValUtil::toRTVal(pathValue);
  pathValue.setMember("path", pathVal);

  FABRIC_CATCH_END("DFGPathValueResolver::castPathToHRFormat");
}

bool DFGPathValueResolver::DFGPortPaths::isExecBlockPort() 
{
  return !blockName.isEmpty();
}

bool DFGPathValueResolver::DFGPortPaths::isExecArg() 
{
  return !isExecBlockPort() && nodeName.isEmpty();
}

QString DFGPathValueResolver::DFGPortPaths::getRelativePortPath() 
{
  if(isExecBlockPort())
    return nodeName + "." + blockName + "." + portName;
  else if(isExecArg())
    return portName;
  else if(!nodeName.isEmpty())
    return nodeName + "." + portName;
  return "";
}
 
QString DFGPathValueResolver::DFGPortPaths::getAbsolutePortPath(
  bool addBindingID) 
{
  QString absPath = execPath.isEmpty()
    ? getRelativePortPath()
    : execPath + "." + getRelativePortPath();

  return addBindingID && !id.isEmpty()
    ? id + "." + absPath
    : absPath;
}

QString DFGPathValueResolver::DFGPortPaths::getFullItemPath(
  bool addBindingID) 
{
  QString absPath = getAbsolutePortPath(addBindingID);
  return isArrayElement()
    ? absPath + "[" + QString::number(arrayIndex) + "]"
    : absPath;
}

QString DFGPathValueResolver::DFGPortPaths::getAbsoluteNodePath(
  bool addBindingID) 
{
  if(!nodeName.isEmpty())
  {
    QString absPath = execPath.isEmpty()
      ? nodeName
      : execPath + "." + nodeName;

    return addBindingID && !id.isEmpty()
      ? id + "." + absPath
      : absPath;
  }
   
  return "";
}
