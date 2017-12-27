from PySide import QtCore, QtGui
from FabricEngine import Core, FabricUI, Util, CAPI
from FabricEngine.FabricUI import *
from FabricEngine.Util import *
from FabricEngine.CAPI import *
from FabricEngine.SceneHub.SHTreeView import SHTreeView


class SHTreeViewsManager(FabricUI.SceneHub.SHBaseTreeViewsManager):

    """SHTreeViewsManager

    SHTreeViewsManager specializes SceneHub.SHBaseTreeViewsManager for python applications.

    Arguments:
        client (FabricEngine.Core.Client): A reference to the FabricCore.Client.
        dfgWidget (DFG.DFGWidget): A reference to the DFGWidget.
        shStates (SceneHub.SHStates): A reference to the SHStates.
        shMainGLScene (SceneHub.SHGLScene): A reference to a main SHGLScene (Can be None).
    """

    def __init__(self, client, dfgWidget, shStates, shMainGLScene = None):
        
        super(SHTreeViewsManager, self).__init__(client, dfgWidget, shStates)
        
        self.m_shTreeView = SHTreeView(client, self.m_shStates, shMainGLScene)
        layout = QtGui.QVBoxLayout()
        layout.addWidget(self.m_comboBox)
        layout.addWidget(self.m_shTreeView)
        self.setLayout(layout)
 
        self.m_shTreeView.selectionCleared.connect(self.onSelectionCleared)
        self.m_shTreeView.itemSelected.connect(self.onTreeItemSelected)
        self.m_shTreeView.itemDeselected.connect(self.onTreeItemDeselected)
        self.m_shTreeView.itemDoubleClicked.connect(self.onTreeItemDoubleClicked)

        # Shows the main-scene (owns by the application) in the treeView.
        self.shMainGLScene = shMainGLScene
        self.m_comboBox.addItem("Main Scene")
        self.onConstructScene("Main Scene")

    def onConstructScene(self, sceneName):
        """ Implementation of SceneHub.SHBaseTreeViewsManager
        """

        if self.shMainGLScene is not None and str(sceneName) == "Main Scene":
            self.m_shGLScene.setSHGLScene(self.shMainGLScene)
            self.constructTree()
            self.activeSceneChanged.emit(self.m_shGLScene)
        
        elif self.m_dfgWidget.getDFGController().getBinding().getExec().hasVar(str(sceneName)):
            self.m_shGLScene.setSHGLScene(self.m_dfgWidget.getDFGController().getBinding(), sceneName)
            self.constructTree()
            self.activeSceneChanged.emit(self.m_shGLScene)
        
        else:
            self.m_comboBox.clear()
            self.resetTree()
    
    def onUpdateSceneList(self):
        """ Implementation of SceneHub.SHBaseTreeViewsManager
        """

        self.m_comboBox.clear()
        binding = self.m_dfgWidget.getDFGController().getBinding()

        typeList = []
        typeList.append('SHGLScene')
        sceneNameList = FabricUI.DFG.DFGBindingUtils.getVariableWordsFromBinding(binding, '.', typeList)

        if len(sceneNameList) == 0 and not self.m_shGLScene.hasSG(): 
            self.resetTree()

        if self.shMainGLScene is not None and self.shMainGLScene.hasSG(): 
            self.m_comboBox.addItem("Main Scene")

        for sceneName in sceneNameList: 
            self.m_comboBox.addItem(sceneName)
