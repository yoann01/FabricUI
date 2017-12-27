// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_DFG_PresetTreeItem__
#define __UI_DFG_PresetTreeItem__

#include <FabricUI/TreeView/TreeItem.h>

namespace FabricUI
{

  namespace DFG
  {

    class PresetTreeItem : public TreeView::TreeItem
    {

    public:

      PresetTreeItem( FTL::CStrRef name );

      virtual FTL::CStrRef type() const { return FTL_STR("Preset"); }
      
      virtual Qt::ItemFlags flags();
      
      virtual QString mimeDataAsText();
      
      virtual QVariant data(int role);

    };

  };

};

#endif // __UI_DFG_PresetTreeItem__
