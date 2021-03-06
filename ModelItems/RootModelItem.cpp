//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include <FabricUI/ModelItems/RootModelItem.h>
#include <FabricUI/ModelItems/RootItemMetadata.h>
#include <assert.h>

namespace FabricUI {
namespace ModelItems {

//////////////////////////////////////////////////////////////////////////
RootModelItem::RootModelItem()
  : m_metadata( 0 )
{
}

RootModelItem::~RootModelItem()
{
  for ( ChildVec::iterator it = m_children.begin();
    it != m_children.end(); ++it )
  {
    ValueEditor::BaseModelItem *child = *it;
    delete child;
  }

  delete m_metadata;
}

ValueEditor::BaseModelItem * RootModelItem::pushChild( BaseModelItem * item )
{
  m_children.push_back(item); 
  return item;
}

ValueEditor::BaseModelItem *RootModelItem::getChild(
  FTL::StrRef childName,
  bool doCreate
  )
{
  int numExisting = m_children.size();
  for (int i = 0; i < numExisting; i++)
  {
    if ( m_children[i]->getName() == childName )
      return m_children[i];
  }
  if (doCreate)
  {
    // Ensure this child exists, we can't assume its valid
    if ( getChildIndex( childName ) >= 0 )
    {
      return createChild( childName );
    }
  }
  return NULL;
}

ValueEditor::BaseModelItem *RootModelItem::getChild( int index, bool doCreate )
{
  FTL::CStrRef childName = getChildName( index );
  return getChild( childName, doCreate );
}

int RootModelItem::getChildIndex( FTL::StrRef name )
{
  int numChildren = getNumChildren();
  for (int i = 0; i < numChildren; i++)
  {
    if ( getChildName( i ) == name )
    {
      return i;
    }
  }
  return -1;
}

ValueEditor::BaseModelItem *RootModelItem::onPortRenamed(
  unsigned index,
  FTL::CStrRef oldName,
  FTL::CStrRef newName
  )
{
  for ( ChildVec::iterator it = m_children.begin();
    it != m_children.end(); ++it )
  {
    ValueEditor::BaseModelItem *childModelItem = *it;
    if ( childModelItem->getName() == oldName )
    {
      childModelItem->onRenamed(
        oldName,
        newName
        );
      return childModelItem;
    }
  }

  return 0;
}

bool RootModelItem::hasDefault()
{
  return false;
}

void RootModelItem::resetToDefault()
{
  assert( false );
}


void RootModelItem::childRemoved( unsigned index, FTL::CStrRef name )
{
  for ( ChildVec::iterator itr = m_children.begin();
    itr != m_children.end(); itr++ )
  {
    FabricUI::ValueEditor::BaseModelItem *childModelItem = *itr;
    if ( childModelItem->getName() == name )
    {
      delete childModelItem;
      m_children.erase( itr );
      break;
    }
  }
}

FabricUI::ValueEditor::ItemMetadata* RootModelItem::getMetadata()
{
  if ( !m_metadata )
    m_metadata = new RootItemMetadata( this );
  return m_metadata;
}

//////////////////////////////////////////////////////////////////////////

std::string SplitLast( std::string& path )
{
  size_t split = path.rfind( '.' );
  std::string res = path.substr( split + 1 );
  if (split == std::string::npos)
  {
    path.clear();
  }
  else
  {
    path = path.substr( 0, split );
  }
  return res;
}

std::string SplitFirst( std::string& path )
{
  size_t split = path.find( '.' );
  std::string res = path.substr( 0, split + 1 );
  if (split == std::string::npos)
  {
    path.clear();
  }
  else
  {
    path = path.substr( split, 0 );
  }
  return res;
}

} // namespace ModelItems
} // namespace FabricUI
