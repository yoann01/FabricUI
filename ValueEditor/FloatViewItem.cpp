//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include "FloatViewItem.h"
#include "QVariantRTVal.h"
#include "VEDoubleSpinBox.h"

#include <FabricUI/Util/UIRange.h>
#include <float.h>
#include <QHBoxLayout>

using namespace FabricUI::ValueEditor;

FloatViewItem::FloatViewItem(
  QString const &name,
  QVariant const &value,
  ItemMetadata* metadata
  )
  : BaseViewItem( name, metadata )
{
  m_spinBox = new VEDoubleSpinBox;
  m_spinBox->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );
  onModelValueChanged( value );

  QHBoxLayout *layout = new QHBoxLayout;
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  layout->addWidget( m_spinBox );
  layout->addStretch();

  m_widget = new QWidget;
  m_widget->setObjectName( "VEFloatViewItem" );
  m_widget->setLayout( layout );

  connect(
    m_spinBox, SIGNAL(interactionBegin()), 
    this, SIGNAL(interactionBegin())
    );
  connect(
    m_spinBox, SIGNAL(valueChanged(double)), 
    this, SLOT(onSpinBoxValueChanged(double))
    );
  connect(
    m_spinBox, SIGNAL(interactionEnd(bool)), 
    this, SIGNAL(interactionEnd(bool))
    );

  metadataChanged();
}

FloatViewItem::~FloatViewItem()
{
}

QWidget *FloatViewItem::getWidget()
{
  return m_widget;
}

void FloatViewItem::onModelValueChanged( QVariant const &v )
{
  m_spinBox->setValue( getQVariantRTValValue<double>(v) );
}

void FloatViewItem::onSpinBoxValueChanged( double value )
{
  emit viewValueChanged(
    QVariant::fromValue<double>( value )
    );
}

void FloatViewItem::metadataChanged()
{
  FTL::StrRef uiRangeString = m_metadata.getString( "uiHardRange" );
  
  double minValue, maxValue;
  if ( FabricUI::DecodeUIRange( uiRangeString, minValue, maxValue ) )
    m_spinBox->setRange( minValue, maxValue );
}

//////////////////////////////////////////////////////////////////////////
// 
BaseViewItem* FloatViewItem::CreateItem(
  QString const &name,
  QVariant const &value,
  ItemMetadata* metadata
  )
{
  if (RTVariant::isType<double>(value) || RTVariant::isType<float>(value))
  {
    return new FloatViewItem( name, value, metadata );
  }
  return 0;
}

const int FloatViewItem::Priority = 3;
