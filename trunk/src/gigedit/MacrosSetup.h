/*
    Copyright (c) MMXVII Christian Schoenebeck

    This file is part of "gigedit" and released under the terms of the
    GNU General Public License version 2.
*/

#ifndef GIGEDIT_MACROSSETUP_H
#define GIGEDIT_MACROSSETUP_H

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

/** @brief Setup all gigedit macros.
 *
 * Shows a list with all gigedit macros configured by the user. It allows to
 * add and remove macros, and to reorder the list of macros for altering the
 * keyboard accelerators (F1 - F12) associated with the individual macros.
 */
class MacrosSetup : public ManagedWindow {
public:
    MacrosSetup();
   ~MacrosSetup();
    void setMacros(const std::vector<Serialization::Archive>& macros,
                   Serialization::Archive* pClipboardContent,
                   gig::DimensionRegion* pSelectedDimRgn);

    sigc::signal<void, const std::vector<Serialization::Archive>& >& signal_macros_changed();

    // implementation for abstract methods of interface class "ManagedWindow"
    virtual Settings::Property<int>* windowSettingX() { return &Settings::singleton()->macrosSetupWindowX; }
    virtual Settings::Property<int>* windowSettingY() { return &Settings::singleton()->macrosSetupWindowY; }
    virtual Settings::Property<int>* windowSettingWidth() { return &Settings::singleton()->macrosSetupWindowW; }
    virtual Settings::Property<int>* windowSettingHeight() { return &Settings::singleton()->macrosSetupWindowH; }

protected:
    bool m_modified;
    std::vector<Serialization::Archive> m_macros;
    Serialization::Archive* m_clipboardContent;
    gig::DimensionRegion* m_selectedDimRgn;

    sigc::signal<void, const std::vector<Serialization::Archive>& > m_macros_changed;

    Gtk::VBox m_vbox;
    Gtk::HBox m_addHBox;
    Gtk::HBox m_footerHBox;
    Gtk::HBox m_statusHBox;
    Gtk::HButtonBox m_buttonBoxL;
    Gtk::HButtonBox m_buttonBox;
    Gtk::ScrolledWindow m_scrolledWindow;

    class MacroListModel : public Gtk::TreeModel::ColumnRecord {
    public:
        MacroListModel() {
            add(m_col_key);
            add(m_col_name);
            add(m_col_comment);
            add(m_col_created);
            add(m_col_modified);
            add(m_col_index);
        }

        Gtk::TreeModelColumn<Glib::ustring> m_col_key;
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_comment;
        Gtk::TreeModelColumn<Glib::ustring> m_col_created;
        Gtk::TreeModelColumn<Glib::ustring> m_col_modified;
        Gtk::TreeModelColumn<int> m_col_index;
    } m_treeModelMacros;

    class MacroListTreeStore : public Gtk::TreeStore {
    public:
        static Glib::RefPtr<MacroListTreeStore> create(const MacroListModel& columns) {
            return Glib::RefPtr<MacroListTreeStore>( new MacroListTreeStore(columns) );
        }
    protected:
        MacroListTreeStore(const MacroListModel& columns) : Gtk::TreeStore(columns) {}
    };

    Gtk::TreeView m_treeViewMacros;
    Glib::RefPtr<MacroListTreeStore> m_treeStoreMacros;
    bool m_ignoreTreeViewValueChange;

    Gtk::Button m_addFromClipboardButton;
    Gtk::Button m_addFromSelectionButton;
    Gtk::Label m_statusLabel;
    Gtk::Button m_deleteButton;
    Gtk::Button m_inverseDeleteButton;
    Gtk::Button m_applyButton;
    Gtk::Button m_cancelButton;

    bool m_altKeyDown;
    bool m_primaryKeyDown; // on Mac: Cmd key, on all other OSs: Ctrl key

    bool isModified() const;
    void onButtonAddFromClipboard();
    void onButtonAddFromSelection();
    void onButtonCancel();
    void onButtonApply();
    void onWindowHide();
    bool onWindowDelete(GdkEventAny* e);
    void updateStatus();
    void updateStatusBar();
    void reloadTreeView();
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

#endif // GIGEDIT_MACROSSETUP_H
