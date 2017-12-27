// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_DFG_TabSearch_DFGPresetSearchWidget__
#define __UI_DFG_TabSearch_DFGPresetSearchWidget__

#include <FabricUI/DFG/DFGTabSearchWidget.h>

#include <set>

class QFrame;
class QScrollArea;

namespace FabricUI
{
  namespace DFG
  {
    namespace TabSearch
    {
      class QueryEdit;
      class DetailsWidget;
      class ResultsView;
      class Toggle;
      struct Query;
      class Result;
    }

    class DFGPresetSearchWidget : public DFGAbstractTabSearchWidget
    {
      Q_OBJECT

    typedef DFGAbstractTabSearchWidget Parent;

    public:
      DFGPresetSearchWidget( FabricCore::DFGHost* host );
      ~DFGPresetSearchWidget();

      void showForSearch( QPoint globalPos ) FTL_OVERRIDE;
      void keyPressEvent( QKeyEvent *event ) FTL_OVERRIDE;
      void hideEvent( QHideEvent * e ) FTL_OVERRIDE;
      bool focusNextPrevChild( bool next ) FTL_OVERRIDE;
      void resizeEvent( QResizeEvent * event ) FTL_OVERRIDE;
      void showEvent( QShowEvent * event ) FTL_OVERRIDE;
      void toggleNewBlocks( bool );

    signals:
      // Emitted when a Result (there are different types) has been chosen
      void selectedPreset( QString preset );
      void selectedBackdrop();
      void selectedNewBlock();
      void selectedCreateNewVariable();
      void selectedGetVariable( const std::string name );
      void selectedSetVariable( const std::string name );
      // Emitted when the widget has completed its actions
      void giveFocusToParent();

    public slots:
      // Will ignore already registered variables
      void registerVariable( const std::string& name, const std::string& type );
      void unregisterVariables();
      void updateResults();

    private slots:
      void onQueryChanged( const TabSearch::Query& query );
      void onResultValidated( const TabSearch::Result& result );
      void validateSelection();
      void hidePreview();
      void setPreview( const TabSearch::Result& preset );
      void close();
      void updateSize();
      void registerStaticEntries();
      void updateDetailsPanelVisibility();
      void toggleDetailsPanel( bool toggled );
      void toggleDetailsPanel() { toggleDetailsPanel( !m_detailsPanelToggled ); }
      void onResultMouseEntered( const TabSearch::Result& );
      void onResultMouseLeft();
      // TODO: use an Enum argument instead of several signals ?
      void onLogError( const std::string& );
      void onLogInstruction( const std::string& );
      void onLogClear();

    protected:

      void maybeReposition();

    private:

      // Will the Query persist between 2 invocations ?
      bool m_clearQueryOnClose;
      bool m_staticEntriesAddedToDB;
      bool m_newBlocksEnabled;

      // Used to query the database
      FabricCore::DFGHost* m_host;
      std::set<std::string> m_registeredVariables;
      QFrame* m_searchFrame;
      class Status;
      Status* m_status; // Inline info about the selected result, at the bottom
      TabSearch::QueryEdit* m_queryEdit;
      TabSearch::ResultsView* m_resultsView;
      TabSearch::DetailsWidget* m_detailsWidget;
      QScrollArea* m_detailsPanel;
      TabSearch::Toggle* m_toggleDetailsButton;
      bool m_detailsPanelToggled;
      class MoveHandle;
    };
  };
};

#endif // __UI_DFG_TabSearch_DFGTabSearchWidget__
