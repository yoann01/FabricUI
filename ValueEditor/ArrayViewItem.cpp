//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#include "ArrayViewItem.h"
#include "ViewItemFactory.h"
#include "QVariantRTVal.h"
#include "VEIntSpinBox.h"
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QString>

using namespace FabricUI::ValueEditor;

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

ArrayViewItem::ArrayViewItem( QString name,
                              const FabricCore::RTVal& value,
                              ItemMetadata* metadata )
  : BaseComplexViewItem( name, metadata )
  , m_val( value )
  , m_min( 0 )
  , m_max( 0 )
  , m_widget( NULL )
  , m_minIndexEdit( NULL )
  , m_maxIndexEdit( NULL )
  , m_arraySizeEdit( NULL )
{
  m_widget = new QWidget();

  if (m_val.isValid())
  {
    int arraySize = m_val.getArraySize();
    // By default don't display more than 100 elements
    m_max = min( arraySize, 100 );
  }

  m_minIndexEdit = new VEIntSpinBox;
  m_minIndexEdit->setObjectName( "VELeft" );
  m_minIndexEdit->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

  QLabel *toLabel = new QLabel( "to" );
  toLabel->setObjectName( "VEMiddle" );
  toLabel->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );

  m_maxIndexEdit = new VEIntSpinBox;
  m_maxIndexEdit->setObjectName( "VEMiddle" );
  m_maxIndexEdit->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

  QLabel *ofLabel = new QLabel( "of" );
  ofLabel->setObjectName( "VEMiddle" );
  ofLabel->setSizePolicy( QSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum ) );

  m_arraySizeEdit = new VEIntSpinBox;
  m_arraySizeEdit->setObjectName( "VERight" );
  m_arraySizeEdit->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Minimum ) );

  updateWidgets();

  connect(
    m_minIndexEdit, SIGNAL( valueChanged( int ) ),
    this, SLOT( onMinIndexChanged( int ) )
    );
  connect(
    m_maxIndexEdit, SIGNAL( valueChanged( int ) ),
    this, SLOT( onMaxIndexChanged( int ) )
    );
  connect(
    m_arraySizeEdit, SIGNAL( valueChanged( int ) ),
    this, SLOT( onArraySizeChanged( int ) )
    );

  QHBoxLayout *layout = new QHBoxLayout( m_widget );
  layout->setContentsMargins( 0, 0, 0, 0 );
  layout->setSpacing( 0 );
  layout->addWidget( m_minIndexEdit );
  layout->addWidget( toLabel );
  layout->addWidget( m_maxIndexEdit );
  layout->addWidget( ofLabel );
  layout->addWidget( m_arraySizeEdit );
  layout->addStretch();
}

class ItemMetadataProxy : public ItemMetadata
{
protected:
  ItemMetadata* m_parent;

public:
  ItemMetadataProxy( ItemMetadata* parent ) : m_parent( parent ) {}

  const char* getString( const char* key ) const FTL_OVERRIDE { return m_parent->getString( key ); }
  int getSInt32( const char* key ) const FTL_OVERRIDE { return m_parent->getSInt32( key ); }
  double getFloat64( const char* key ) const FTL_OVERRIDE { return m_parent->getFloat64( key ); }
  const FTL::JSONObject* getDict( const char* key ) const FTL_OVERRIDE { return m_parent->getDict( key ); }
  const FTL::JSONArray* getArray( const char* key ) const FTL_OVERRIDE { return m_parent->getArray( key ); }

};

class ArrayViewItem::ArrayItemMetadata : public ItemMetadataProxy
{
  bool m_null;
  std::string m_path;
 
  typedef ItemMetadataProxy Parent;

public:
  ArrayItemMetadata( ItemMetadata* parent, int index )
    : Parent( parent )
    , m_null( true )
  {
    const char* parentPath = parent->getString( VEPathKey.data() );
    if( parentPath != NULL )
    {
      m_path = (QString(parentPath) + "[" + QString::number(index) + "]").toUtf8().constData();
      m_null = false;
    }
  }

  const char* getString( const char* key ) const FTL_OVERRIDE
  {
    if( key == VEPathKey )
      return m_null ? NULL : m_path.data();
    return Parent::getString( key );
  }
};

ArrayViewItem::~ArrayViewItem()
{
  for( ArrayItemMetadataMap::iterator it = m_itemsMetadata.begin(); it != m_itemsMetadata.end(); it++ )
    delete it->second;
}

void ArrayViewItem::doAppendChildViewItems( QList<BaseViewItem *>& items )
{
  try
  {
    // Construct a child for each instance between min & max
    ViewItemFactory* factory = ViewItemFactory::GetInstance();
    char childName[64];
    for (int i = m_min; i < m_max; ++i)
    {
      snprintf( childName, 64, "[%d]", i );

      FabricCore::RTVal childVal = m_val.getArrayElementRef( i );

      // TODO : update when the parent path (through metadata) changed
      if( m_itemsMetadata.find( i ) == m_itemsMetadata.end() )
        m_itemsMetadata.insert( ArrayItemMetadataMap::value_type( i,
          new ArrayItemMetadata( &m_metadata, i ) ) );

      BaseViewItem* childItem =
        factory->createViewItem(
          childName,
          toVariant( childVal ),
          m_itemsMetadata.find( i )->second
          );

      if (childItem != NULL)
      {
        connectChild( i - m_min, childItem );
        items.push_back( childItem );
      }
    }
  }
  catch (FabricCore::Exception e)
  {
    const char* error = e.getDesc_cstr();
    printf( "%s", error );
  }
}

void ArrayViewItem::onChildViewValueChanged( int index, QVariant value )
{
  if (m_val.isValid() && m_val.isArray())
  {
    // FE-8822, we need to clone the array 'm_val' 
    // since it references directly the model array.
    FabricCore::RTVal cloneVal = m_val.clone();

    // We cannot simply create a new RTVal based on the QVariant type, as 
    // we have to set the type exactly the same as the original.  Get the
    // original child value to ensure the new value matches the internal type
    int arrayIndex = index + m_min;
    FabricCore::RTVal oldChildVal = cloneVal.getArrayElementRef( arrayIndex );
    RTVariant::toRTVal( value, oldChildVal );
    cloneVal.setArrayElement( arrayIndex, oldChildVal );
    emit viewValueChanged( toVariant( cloneVal ) );
  }
}

QWidget * ArrayViewItem::getWidget()
{
  return m_widget;
}

void ArrayViewItem::onModelValueChanged( QVariant const &value )
{
  int oldSize = m_val.getArraySize();
  RTVariant::toRTVal( value, m_val );
  int arraySize = m_val.getArraySize();

  // If array size changed, 
  // rebuild the whole widget. 
  if (arraySize != oldSize)
  {
    m_max = arraySize;
    m_min = min( m_max, m_min );

    updateWidgets();
    emit rebuildChildren( this );
  }
   
  for (int i = m_min; i < m_max; ++i)
  {
    FabricCore::RTVal childVal = m_val.getArrayElementRef( i );
    routeModelValueChanged( i - m_min, toVariant( childVal ) );
  }  
}

void ArrayViewItem::updateWidgets()
{
  int arraySize = 0;
  if (m_val.isValid() && m_val.isArray())
    arraySize = m_val.getArraySize();

  m_max = min( m_max, arraySize );
  m_min = min( m_min, m_max );

  m_minIndexEdit->setRange( 0, arraySize );
  m_maxIndexEdit->setRange( 0, arraySize );
  bool isVariableArray = m_val.isVariableArray();
  if (isVariableArray)
    m_arraySizeEdit->setRange(0, INT_MAX);
  m_arraySizeEdit->setEnabled(isVariableArray);
  
  m_minIndexEdit->setValue( m_min );
  m_maxIndexEdit->setValue( m_max );
  m_arraySizeEdit->setValue( arraySize );
}

//bool ArrayViewItem::updateAndLimitMinMax()
//{
//  m_min = m_max = 0;
//  if (!m_val.isValid())
//    return false;
//
//  bool ok = true;
//  int min = m_minIndexEdit->text().toInt( &ok );
//    return false;
//
//  int max = m_maxIndexEdit->text().toInt( &ok );
//  if (ok)
//    return false;
//
//  unsigned childCount = m_val.getArraySize();
//  // Limit max/min to appropriate size
//  max = (max > childCount) ? childCount : max;
//  min = (min < max) ? min : max;
//
//  if (min != m_min || max != m_max)
//  {
//    m_min = min;
//    m_max = max;
//    return true;
//  }
//  return false;
//}



void ArrayViewItem::onMinIndexChanged( int newMin )
{
  if (newMin == m_min)
    return;
  m_min = newMin;
  updateWidgets();

  emit rebuildChildren( this );
}

void ArrayViewItem::onMaxIndexChanged( int newMax )
{
  if (newMax == m_max)
    return;

  m_max = newMax;
  updateWidgets();

  emit rebuildChildren( this );
}

void ArrayViewItem::onArraySizeChanged( int newSize )
{
  if (m_val.isValid() && m_val.isVariableArray())
  {
    // FE-8822, we need to clone the array 'm_val' 
    // since it references directly the model array.
    FabricCore::RTVal cloneVal = m_val.clone();
    cloneVal.setArraySize( newSize );
    emit viewValueChanged( toVariant( cloneVal ) );
  }
}

//////////////////////////////////////////////////////////
//
BaseViewItem* ArrayViewItem::CreateItem(
  QString const &name,
  QVariant const &value,
  ItemMetadata* metadata
  )
{
  if (value.type() != QVariant::UserType)
    return NULL;
  if (value.userType() != qMetaTypeId<FabricCore::RTVal>())
    return NULL;

  FabricCore::RTVal rtVal = value.value<FabricCore::RTVal>();
  if (rtVal.isValid() && rtVal.isArray())
  {
    ArrayViewItem* pViewItem = new ArrayViewItem( QString( name ), rtVal, metadata );
    return pViewItem;
  }
  return NULL;
}

const int ArrayViewItem::Priority = 5;
