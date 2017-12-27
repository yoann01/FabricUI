/*
 *  Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
 */

#ifndef __UI_SCENEHUB_DFG_CANVAS_H__
#define __UI_SCENEHUB_DFG_CANVAS_H__

#include <QList>
#include <QString>
#include <QStringList>
#include <FabricCore.h>
#include <FTL/JSONEnc.h>
#include <FTL/OwnedPtr.h>
#include <FTL/JSONValue.h>
#include <FabricUI/DFG/DFGController.h>
#include <FabricUI/SceneHub/SHStates.h>

namespace FabricUI {
namespace SceneHub {
    
class SHDFGBinding : public QObject {

  Q_OBJECT

  public:
    SHDFGBinding(
      FabricCore::DFGBinding &binding, 
      FabricUI::DFG::DFGController *controller,
      FabricUI::SceneHub::SHStates *shStates);
    
    void setCanvasOperator(FabricCore::RTVal &canvasOperator);

    FabricCore::RTVal getCanvasOperator();

    FabricCore::RTVal getCanvasOperatorParameterObject();

    void setMainBinding(FabricCore::DFGBinding &binding);

    QList<bool> setDirty();


  signals:
    void sceneChanged();
    

  public slots:
    void onInspectChanged();

    /// Drives a node input-ports with an user-event.
    /// \param event The user event.
    void onDriveNodeInputPorts(FabricCore::RTVal event);
    

  protected slots:
    void onArgInserted(unsigned index, FTL::CStrRef name, FTL::CStrRef typeName);

    void onArgRemoved(unsigned index, FTL::CStrRef name);

    void onArgTypeChanged(unsigned index, FTL::CStrRef name, FTL::CStrRef newTypeName);


  protected:         
    void connectBindingNotifier();

    FabricCore::DFGBinding m_binding;
    FabricUI::DFG::DFGController *m_controller;
    FabricUI::SceneHub::SHStates *m_shStates;
    FabricCore::RTVal m_dfgCanvasOperator;
    FabricCore::RTVal m_computeContextVal;
};

} // namespace SceneHub
} // namespace FabricUI


#endif // __UI_SCENEHUB_DFG_CANVAS_H__
