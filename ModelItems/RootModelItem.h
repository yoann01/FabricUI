//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#ifndef FABRICUI_MODELITEMS_ROOTMODELITEM_H
#define FABRICUI_MODELITEMS_ROOTMODELITEM_H

#include <FabricUI/ValueEditor/BaseModelItem.h>
#include <FabricUI/ValueEditor/QVariantRTVal.h>
#include <FTL/StrRef.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// The Root-level model item for 
// 

namespace FabricUI
{

  namespace ModelItems
  {

    class RootItemMetadata;

    // Split last word after '.' and return it, remove from path
    // Given "Node.Node.Port", it will return "Port", 
    // and path will be "Node.Node".
    extern std::string SplitLast( std::string& path );

    // Split first word before '.' and return it, remove from path
    // Given "Node.Node.Port", it will return "Node", 
    // and path will be "Node.Port".
    extern std::string SplitFirst( std::string& path );

    typedef std::vector<FabricUI::ValueEditor::BaseModelItem*> ChildVec;
    class RootModelItem : public FabricUI::ValueEditor::BaseModelItem
    {
    protected:

      ChildVec m_children;
      BaseModelItem * pushChild(BaseModelItem * item);

      RootItemMetadata *m_metadata;
      
    public:

      RootModelItem( );
      ~RootModelItem();

      virtual FabricUI::ValueEditor::BaseModelItem *
      createChild( FTL::StrRef name ) { return 0; } /* To override */

      ChildVec::iterator GetChildItrBegin() { return m_children.begin(); }
      ChildVec::iterator GetChildItrEnd() { return m_children.end(); }

      virtual FabricUI::ValueEditor::BaseModelItem *getChild(
        FTL::StrRef childName,
        bool doCreate = true
        ) /*override*/;
      virtual FabricUI::ValueEditor::BaseModelItem *getChild(
        int index,
        bool doCreate = true
        ) /*override*/;

      virtual int getChildIndex( FTL::StrRef name ) /*override*/;
      
      void childRemoved( unsigned index, FTL::CStrRef name );

      virtual bool hasDefault() /*override*/;

      virtual void resetToDefault() /*override*/;

      FabricUI::ValueEditor::BaseModelItem *onPortRenamed(
        unsigned index,
        FTL::CStrRef oldName,
        FTL::CStrRef newName
        );
      
      virtual FabricUI::ValueEditor::ItemMetadata* getMetadata();
    };
  }
}

#endif // FABRICUI_MODELITEMS_ROOTMODELITEM_H
