// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_DFG_DFGTabSearchWidget__
#define __UI_DFG_DFGTabSearchWidget__

#include "DFGConfig.h"
#include <SplitSearch/SplitSearch.hpp>

#include <QWidget>
#include <QFocusEvent>

namespace FabricUI
{

  namespace DFG
  {
    class DFGWidget;

    class DFGAbstractTabSearchWidget : public QWidget
    {
      Q_OBJECT

    public:
      virtual void showForSearch( QPoint globalPos ) = 0;

    signals :
      void enabled( bool );

    };

    class DFGTabSearchWidget : public DFGAbstractTabSearchWidget 
    {
      Q_OBJECT

    public:

      DFGTabSearchWidget(DFGWidget * parent, const DFGConfig & config);
      virtual ~DFGTabSearchWidget();

      void showForSearch( QPoint globalPos );
      void showForSearch();

      virtual void mousePressEvent(QMouseEvent * event);
      virtual void mouseMoveEvent(QMouseEvent * event);
      virtual void keyPressEvent(QKeyEvent * event);
      virtual void paintEvent(QPaintEvent * event);
      virtual void hideEvent(QHideEvent * event);

    signals: 

      void enabled(bool);

    protected:

      virtual void focusOutEvent(QFocusEvent * event);
      virtual bool focusNextPrevChild(bool next);
      virtual void updateSearch();
      virtual int margin() const;

      QString resultLabel(unsigned int index) const;
      int indexFromPos(QPoint pos);
      int widthFromResults() const;
      int heightFromResults() const;
      void updateGeometry();

      char const *getHelpText() const;

    private:

      void addNodeForIndex( unsigned index );

      QPoint m_originalLocalPos;

      DFGWidget * m_parent;
      DFGConfig m_config;
      QFontMetrics m_queryMetrics;
      QFontMetrics m_resultsMetrics;
      QFontMetrics m_helpMetrics;

      int m_currentIndex;
      QString m_search;
      FabricServices::SplitSearch::Matches m_results;
    };

  };

};

#endif // __UI_DFG_DFGTabSearchWidget__
