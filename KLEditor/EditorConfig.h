// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.

#ifndef __UI_KLEditor_EditorConfig__
#define __UI_KLEditor_EditorConfig__

#include <QColor>
#include <QFont>
#include <QTextCharFormat>

namespace FabricUI
{

  namespace KLEditor
  {

    struct EditorConfig
    {
      unsigned int lineNumberFontSize;
      QFont lineNumberFont;
      QColor lineNumberBackgroundColor;
      QColor lineNumberFontColor;

      QColor codeBackgroundColor;
      unsigned int codeFontSize;
      QFont codeFont;
      QColor codeFontColor;
      unsigned int codeTabWidth;

      unsigned int codeCompletionFontSize;
      QFont codeCompletionFont;
      QColor codeCompletionBackgroundColor;
      QColor codeCompletionActiveBackgroundColor;
      QColor codeCompletionFontColor;
      QColor codeCompletionFontColorSuffix;
      unsigned int codeCompletionMargins;
      unsigned int codeCompletionSpacing;

      QTextCharFormat formatForComment;
      QTextCharFormat formatForString;
      QTextCharFormat formatForNumber;
      QTextCharFormat formatForKeyword;
      QTextCharFormat formatForType;
      QTextCharFormat formatForConstant;
      QTextCharFormat formatForMethod;
      QTextCharFormat formatForError;

      bool editorAutoRebuildAST;
      bool editorAlwaysShowCodeCompletion;

      QTextCharFormat formatForHighlight;

      QString docUrlPrefix;

      EditorConfig();
    };

  };

};

#endif // __UI_KLEditor_EditorConfig__
