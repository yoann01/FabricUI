//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#ifndef FABRICUI_DFG_DFGNOTIFIER_H
#define FABRICUI_DFG_DFGNOTIFIER_H

#include <FTL/StrRef.h>
#include <QObject>
#include <stdint.h>

namespace FabricUI {
namespace DFG {

class DFGNotifier : public QObject
{
  Q_OBJECT

public:

  virtual ~DFGNotifier() {}

protected:

  DFGNotifier() {}

  static void Callback(
    void *userdata,
    char const *jsonCStr,
    unsigned int jsonSize
    )
  {
    static_cast<DFGNotifier *>(
      userdata
      )->handle(
        FTL::CStrRef( jsonCStr, jsonSize )
        );
  }

private:

  virtual void handle( FTL::CStrRef jsonStr ) = 0;
};

} // namespace DFG
} // namespace FabricUI

#endif // FABRICUI_DFG_DFGNOTIFIER_H
