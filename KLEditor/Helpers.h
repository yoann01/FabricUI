// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_KLEditor_Helpers__
#define __UI_KLEditor_Helpers__

#include <QString>
#include <string>

namespace FabricUI
{

  namespace KLEditor
  {

    inline std::string QStringToStl(QString text)
    {
      return text.toUtf8().constData();
    }

  };

};

#endif // __UI_KLEditor_Helpers__
