//
// Copyright (c) 2010-2017 Fabric Software Inc. All rights reserved.
//

#ifndef FABRICUI_VALUEEDITOR_BASEVIEWITEM_H
#define FABRICUI_VALUEEDITOR_BASEVIEWITEM_H

#include "ItemMetadata.h"
#include <vector>
#include <QObject>
#include <QVariant>
#include <FabricCore.h>
#include <QCheckBox>

class QTreeWidget;
class QTreeWidgetItem;
class QString;


namespace FabricUI {
namespace ValueEditor {
  
class BaseModelItem;
class BaseViewItem;
class ItemMetadata;

// The base item for the view-side of the equation.
// A BaseViewItem essentially represents a row in the
// treeview.  The base class handles communicating with
// the core, and should be implemented to handle specific
// UI duties (ie, showing widgets, etc).  

class BaseViewItem : public QObject
{
  // Friend the Factory to allow it to set our modelItem
  friend class ViewItemFactory;

  Q_OBJECT

  Q_DISABLE_COPY(BaseViewItem);
  
  // Every ViewItem has a name, which can be displayed 
  // in the TreeView, and may also be used to identify
  // this ViewItem.
  QString m_name;

  // Tooltip displayed over the name
  QString m_toolTip;

  // A ViewItem may or may not have a ModelItem
  BaseModelItem *m_modelItem;

protected:

  // 

  // We cache our metadata for passing on to our children
  ViewItemMetadata m_metadata;

protected:

  // This value is only setable by ViewitemFactory
  virtual void setBaseModelItem( BaseModelItem* item );

protected:
  // It is not legal to directly delete this
  // class, call deleteMe function instead
  virtual ~BaseViewItem();

public:

  BaseViewItem( QString const &name, ItemMetadata* metadata );

  // Returns the number of live ViewItems
  static int numInstances();

  // Returns a matching ModelItem for this ViewItem
  // May be null.
  BaseModelItem* getModelItem()
    { return m_modelItem; }

  // Get the name of this ViewItem
  QString getName() const
    { return m_name; }

  // Set the tooltip of this ViewItem
  void setToolTip( const QString& tooltip )
    { m_toolTip = tooltip; }

  // Implement this function to build the widgets to
  // display the value represented by your class
  virtual QWidget *getWidget() { return 0; }

  // Indicate if this ViewItem will return any 
  // ViewItems in the appendChildViewItems function
  virtual bool hasChildren() const;
  
  // Implement this function to add additional ViewItem
  // as children.  Default implementation adds ViewItems
  // for each child of the model item (if present)
  virtual void appendChildViewItems( QList<BaseViewItem *>& items);

  // Add the viewItems widgets to the tree item.  The default
  // implementation simply calls GetWidget and adds this
  // to the tree, however a ViewItem can override this function 
  // if more control is desired.
  virtual void setWidgetsOnTreeItem(
    QTreeWidget* treeWidget,
    QTreeWidgetItem * treeWidgetItem );

  // This function should be called by the UI when the
  // item name is edited (by the user)
  void renameItem( QString newName );

  // Update our name from core 
  void onRenamed( QTreeWidgetItem* item );

  // Implement this function if ViewItem uses metadata to
  // set its behaviour.  This function may be called at any
  // time if the metadata associated with this item changes.
  virtual void metadataChanged( );

  // This virtual function is called to release this object.
  // It is required for external parties to use this function
  // instead of directly deleting the object, as that ensures
  // the memory is released in the same Dll as it was allocated in
  virtual void deleteMe() {}

  // We need to define a metadata syntax for 
  // additional type-info.  For example, it should
  // be possible using metadata to request a value
  // be displayed in a certain way.
  inline const ItemMetadata* getMetadata() const { return &m_metadata; };

public slots:

  // Implement this slot to update the UI to the
  // passed variant.  If necessary, pass the update
  // down to this items children as well.
  virtual void onModelValueChanged( QVariant const &value ) {}
    
  void emitRefreshViewport() { emit refreshViewport(); }
  
  void emitToggleManipulation(bool toggle);

signals:

  // Triggered before a user interaction begins
  void interactionBegin();

  // When this items widgets change, it should emit this signal
  void viewValueChanged( QVariant value );

  void interactionEnd( bool accept );

  // A view item may emit this signal to request 
  // its children be rebuilt.  The item being passed
  // should be the item who requires children rebuilt
  void rebuildChildren(FabricUI::ValueEditor::BaseViewItem* item);

  // Refreshes the viewport, if a klTool
  // has been activated-deactivated.
  void refreshViewport();

  void toggleManipulation(bool);
};

} // namespace FabricUI 
} // namespace ValueEditor 

#endif // FABRICUI_VALUEEDITOR_BASEVIEWITEM_H
