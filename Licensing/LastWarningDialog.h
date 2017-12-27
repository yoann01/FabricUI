/*
 *  Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
 */

#ifndef __FABRICUI_LICENSING_LASTWARNINGDIALOG_H
#define __FABRICUI_LICENSING_LASTWARNINGDIALOG_H

#include <QCursor>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <FabricUI/Util/LoadFabricStyleSheet.h>

class LastWarningDialog : public QDialog
{
  Q_OBJECT

public:
  LastWarningDialog( QWidget *parent )
    : QDialog( parent )
  {
    // setModal( true );
    setWindowModality( Qt::ApplicationModal );
    setWindowTitle( "Fabric Licensing" );
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QString styleSheet = LoadFabricStyleSheet( "FabricUI.qss" );
    if ( !styleSheet.isEmpty() )
      setStyleSheet( styleSheet );

    setLayout( new QVBoxLayout() );

    QLabel *info = new QLabel( this );
    info->setText( "Fabric will continue to run unlicensed.  However, "
                   "it will pause for 15 seconds every 15 minutes,\nresuming "
                   "automatically after each pause.  These pauses will go "
                   "away once Fabric is licensed." );
    info->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    layout()->addWidget( info );

    QPushButton *continueButton = new QPushButton( "Continue", this );

    QObject::connect( continueButton, SIGNAL( clicked() ), this, SLOT( accept() ) );

    layout()->addWidget( continueButton );
  }
};

#endif // __FABRICUI_LICENSING_LASTWARNINGDIALOG_H
