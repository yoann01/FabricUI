// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_DFG_TabSearch_ItemView__
#define __UI_DFG_TabSearch_ItemView__

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <FTL/Config.h>

#include "Data.h"

class QPushButton;

inline std::string ToStdString( const QString& s )
{
  const QByteArray utf8 = s.toUtf8();
  return std::string( utf8.data(), utf8.size() );
}
inline QString ToQString( const std::string& s ) { return QString::fromUtf8( s.data(), int(s.size()) ); }

// Converts a UTF8 string to a Latin1 string (guaranteeing 1 byte per character)
inline std::string ToLatin1( const QString& s )
{
  const QByteArray cvtd = s.toLatin1();
  assert( cvtd.size() == s.size() ); // There should be as many bytes as characters
  return std::string( cvtd.data(), cvtd.end() );
}
inline std::string ToLatin1( const std::string& s ) { return ToLatin1( ToQString( s ) ); }

namespace FabricUI
{
  namespace DFG
  {
    namespace TabSearch
    {
      class HeatBar;

      void SetWidgetHighlight( QWidget* w, const bool highlighted );

      class TagWidget : public QWidget
      {
        Q_OBJECT

        typedef QWidget Parent;

      public:
        TagWidget( const Query::Tag& tag );
        void setScore( double score );

      static std::string DisplayName( const Query::Tag& result );

      signals:
        void activated( const Query::Tag& tag );

      public:
        void connectToQuery( const Query& );

      public slots:
        void setHighlighted( bool highlighted );

      protected:
        void enterEvent( QEvent * ) FTL_OVERRIDE;
        void leaveEvent( QEvent * ) FTL_OVERRIDE;

      private slots:
        void onActivated();
        void onQueryChanged( const Query& );

      private:
        Query::Tag m_tag;
        QPushButton* m_button;
        bool m_hovered;
        bool m_highlighted;
        bool m_isDisabled;
      };

      class PresetView : public QWidget
      {
        Q_OBJECT

      public:
        PresetView(
          const Result& presetName,
          const std::vector<Query::Tag>& tags = std::vector<Query::Tag>()
        );
        void setScore( double score, double minScore = 0, double maxScore = 1 );

      static std::string DisplayName( const Result& result );

      signals:
        void requestTag( const Query::Tag& tagName );

      public slots:
        void setHighlighted( bool highlighted );

      protected:
        Result m_result;

      private:
        // TODO: Interaction
        std::vector<TagWidget*> m_tagWidgets;
        HeatBar* m_heatBar;
      };

      class Label : public QLabel
      {
        Q_OBJECT

        typedef QLabel Parent;

      public:
        void set( const std::string& text );
        void set( const std::string& text, const Query::Tag& tag );
        Label() { init(); set( "" ); }
        Label( const std::string& text ) { init(); set( text ); };
        Label( const std::string& text, const Query::Tag& tag ) { init(); set( text, tag ); }
        void connectToQuery( const Query& );

      signals:
        void requestTag( const Query::Tag& tagName );

      protected:
        void mouseReleaseEvent( QMouseEvent * ) FTL_OVERRIDE;
        void enterEvent( QEvent * ) FTL_OVERRIDE;
        void leaveEvent( QEvent * ) FTL_OVERRIDE;

      private slots:
        void onQueryChanged( const Query& );

      private:
        void init();
        bool m_isTag;
        bool m_isDisabled;
        Query::Tag m_tag;
      };
    }
  };
};

#endif // __UI_DFG_TabSearch_ItemView__
