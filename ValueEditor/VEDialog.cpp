//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include "VEDialog.h"

#include <QApplication>
#include <QDesktopWidget>

using namespace FabricUI::ValueEditor;

VEDialog::VEDialog( QWidget * parent, Qt::WindowFlags f )
: QDialog( parent, f )
{
  setWindowTitle("Value Editor");
}

void VEDialog::showEvent( QShowEvent * event )
{
  QDialog::showEvent( event );

  // ensure that the dialog is visible
  QRect screen = QApplication::desktop()->availableGeometry( this );
  QPoint pos(this->geometry().x(), this->geometry().y());
  QSize dlgSize = this->size();
  int margin = 20;
  if( dlgSize.width() + pos.x() + margin > screen.width() )
    pos.setX( screen.width() - margin - dlgSize.width() );
  if( dlgSize.height() + pos.y() + margin > screen.height() )
    pos.setY( screen.height() - margin - dlgSize.height() );

  move(pos);
}
