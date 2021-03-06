// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_GraphView_NodeLibrary__
#define __UI_GraphView_NodeLibrary__

#include <QWidget>
#include <QLineEdit>
#include <QScrollArea>

#include <vector>

namespace FabricUI
{

  namespace GraphView
  {
    class NodeLibraryItem;

    class NodeLibrary : public QWidget
    {
      Q_OBJECT

    public:

      NodeLibrary(QWidget * parent);
      virtual ~NodeLibrary() {}

      void init();

      QWidget * itemParent();

      virtual unsigned int itemCount() = 0;
      virtual QString itemPath(unsigned int index) = 0;
      virtual QColor itemColor(unsigned int index) = 0;
      virtual NodeLibraryItem * constructItem(unsigned int index);

      QString searchPattern() const;

    signals:

      void searchPatternChanged(QString);

    public slots:

      void search();
      void clearSearch();

    private:

      QLineEdit * m_searchBox;
      QScrollArea * m_scrollArea;
      QWidget * m_listWidget;
      QString m_prevSearchPattern;
    };

  };

};

#endif // __UI_GraphView_NodeLibrary__
