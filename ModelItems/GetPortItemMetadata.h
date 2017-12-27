//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#ifndef FABRICUI_MODELITEMS_GETPORTITEMMETADATA_H
#define FABRICUI_MODELITEMS_GETPORTITEMMETADATA_H

#include "ItemPortItemMetadata.h"

#include <FabricUI/ModelItems/GetPortModelItem.h>

namespace FabricUI {
namespace ModelItems {

class GetPortItemMetadata : public ItemPortItemMetadata
{
public:

  GetPortItemMetadata( GetPortModelItem *getPortModelItem )
    : ItemPortItemMetadata( getPortModelItem ) {}

  virtual const char* getString( const char* key ) const /*override*/
  {
    if ( key == VENotInspectableKey )
    {
      return FTL_STR("1").c_str();
    }

    return ItemPortItemMetadata::getString( key );
  }
};

} // namespace ModelItems
} // namespace FabricUI

#endif // FABRICUI_MODELITEMS_GETPORTITEMMETADATA_H
