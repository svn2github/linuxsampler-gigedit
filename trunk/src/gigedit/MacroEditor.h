/*
    Copyright (c) MMXVII Christian Schoenebeck

    This file is part of "gigedit" and released under the terms of the
    GNU General Public License version 2.
*/

#ifndef GIGEDIT_MACROEDITOR_H
#define GIGEDIT_MACROEDITOR_H

#ifdef LIBGIG_HEADER_FILE
# include LIBGIG_HEADER_FILE(gig.h)
# include LIBGIG_HEADER_FILE(Serialization.h)
#else
# include <gig.h>
# include <Serialization.h>
#endif

#include <gtkmm.h>
#if HAVE_CONFIG_H
# include <config.h>
#endif
#include "compat.h"
#include <gtkmm/uimanager.h>
#include <gtkmm/actiongroup.h>
#include "ManagedWindow.h"

/** @brief Editor for gigedit macros.
 *
 * Implements a window which allows to edit the abstract tree of one macro
 * that may be applied with gigedit.
 */
class MacroEditor : public ManagedWindow {
public:
    MacroEditor();
   ~MacroEditor();
    void setMacro(Serialization::Archive* macro);

    // implementation for abstract methods of interface class "ManagedWindow"
    virtual Settings::Property<int>* windowSettingX() { return &Settings::singleton()->macroEditorWindowX; }
    virtual Settings::Property<int>* windowSettingY() { return &Settings::singleton()->macroEditorWindowY; }
    virtual Settings::Property<int>* windowSettingWidth() { return &Settings::singleton()->macroEditorWindowW; }
    virtual Settings::Property<int>* windowSettingHeight() { return &Settings::singleton()->macroEditorWindowH; }

protected:
    Serialization::Archive* m_macroOriginal;
    Serialization::Archive  m_macro;

    Gtk::VBox m_vbox;
    Gtk::HBox m_footerHBox;
    Gtk::HBox m_statusHBox;
    Gtk::HButtonBox m_buttonBoxL;
    Gtk::HButtonBox m_buttonBox;
    Gtk::ScrolledWindow m_scrolledWindow;

    class MacroModel : public Gtk::TreeModel::ColumnRecord {
    public:
        MacroModel() {
            add(m_col_name);
            add(m_col_type);
            add(m_col_value);
            add(m_col_uid);
        }

        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_type;
        Gtk::TreeModelColumn<Glib::ustring> m_col_value;
        Gtk::TreeModelColumn<Serialization::UID> m_col_uid;
    } m_treeModelMacro;

    class MacroTreeStore : public Gtk::TreeStore {
    public:
        static Glib::RefPtr<MacroTreeStore> create(const MacroModel& columns) {
            return Glib::RefPtr<MacroTreeStore>( new MacroTreeStore(columns) );
        }
    protected:
        MacroTreeStore(const MacroModel& columns) : Gtk::TreeStore(columns) {}
    };

    Gtk::TreeView m_treeViewMacro;
    Glib::RefPtr<MacroTreeStore> m_treeStoreMacro;
    bool m_ignoreTreeViewValueChange;

    Gtk::Label m_statusLabel;
    Gtk::Button m_deleteButton;
    Gtk::Button m_inverseDeleteButton;
    Gtk::Button m_applyButton;
    Gtk::Button m_cancelButton;

    bool m_altKeyDown;

    bool isModified() const;
    void onButtonCancel();
    void onButtonApply();
    void onWindowHide();
    bool onWindowDelete(GdkEventAny* e);
    void updateStatus();
    void updateStatusBar();
    void reloadTreeView();
    void buildTreeView(const Gtk::TreeModel::Row& parentRow, const Serialization::Object& parentObject);
    void onTreeViewSelectionChanged();
    void onMacroTreeViewKeyRelease(GdkEventKey* button);
    void onMacroTreeViewRowValueChanged(const Gtk::TreeModel::Path& path,
                                        const Gtk::TreeModel::iterator& iter);
    void deleteSelectedRows();
    void inverseDeleteSelectedRows();
    void deleteRows(const std::vector<Gtk::TreeModel::Path>& rows);
    bool onKeyPressed(GdkEventKey* key);
    bool onKeyReleased(GdkEventKey* key);
};

#endif // GIGEDIT_MACROEDITOR_H
