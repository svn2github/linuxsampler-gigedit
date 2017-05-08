/*
    Copyright (c) MMXVII Christian Schoenebeck

    This file is part of "gigedit" and released under the terms of the
    GNU General Public License version 2.
*/

#include "MacrosSetup.h"
#include "global.h"
#include <assert.h>
#include <set>
#include <math.h>
#include <gtkmm/stock.h>

MacrosSetup::MacrosSetup() :
    m_modified(false),
    m_clipboardContent(NULL),
    m_addFromClipboardButton("  " + Glib::ustring(_("From Clipboard")) + "  " + UNICODE_PRIMARY_KEY_SYMBOL + "B"),
    m_addFromSelectionButton("  " + Glib::ustring(_("From Selection")) + "  " + UNICODE_PRIMARY_KEY_SYMBOL + "S"),
    m_statusLabel("",  Gtk::ALIGN_START),
    m_deleteButton(Glib::ustring(_("Delete")) + "  " + UNICODE_PRIMARY_KEY_SYMBOL + UNICODE_ERASE_KEY_SYMBOL),
    m_inverseDeleteButton(Glib::ustring(_("Inverse Delete")) + "  " + UNICODE_ALT_KEY_SYMBOL + UNICODE_ERASE_KEY_SYMBOL),
    m_applyButton(_("_Apply"), true),
    m_cancelButton(_("_Cancel"), true),
    m_altKeyDown(false),
    m_primaryKeyDown(false)
{
    add(m_vbox);

    set_title(_("Setup Macros"));

    set_default_size(800, 600);

    m_addFromClipboardButton.set_image(
        *new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_BUTTON)
    );
    m_addFromSelectionButton.set_image(
        *new Gtk::Image(Gtk::Stock::ADD, Gtk::ICON_SIZE_BUTTON)
    );
    m_addFromClipboardButton.set_tooltip_text(_("Create a new macro from the content currently available on the clipboard."));
    m_addFromSelectionButton.set_tooltip_text(_("Create a new macro from the currently selected dimension region's parameters currently shown on the main window."));
    m_addHBox.pack_start(m_addFromClipboardButton);
    m_addHBox.pack_start(m_addFromSelectionButton);
    m_vbox.pack_start(m_addHBox, Gtk::PACK_SHRINK);

    // create Macro list treeview (including its data model)
    m_treeStoreMacros = MacroListTreeStore::create(m_treeModelMacros);
    m_treeViewMacros.set_model(m_treeStoreMacros);
    m_treeViewMacros.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
    //m_treeViewMacro.set_tooltip_text(_(""));
    m_treeViewMacros.append_column(_("Key"), m_treeModelMacros.m_col_key);
    m_treeViewMacros.append_column_editable(_("Name"), m_treeModelMacros.m_col_name);
    m_treeViewMacros.append_column(_("Created"), m_treeModelMacros.m_col_created);
    m_treeViewMacros.append_column(_("Modified"), m_treeModelMacros.m_col_modified);
    m_treeViewMacros.set_tooltip_column(m_treeModelMacros.m_col_comment.index());
    // make all rows gray text, except of "Name" column
    for (int i = 0; i <= 3; ++i) {
        if (i == m_treeModelMacros.m_col_name.index())
            continue;
        Gtk::TreeViewColumn* column = m_treeViewMacros.get_column(i);
        Gtk::CellRendererText* cellrenderer =
            dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell());
        cellrenderer->property_foreground().set_value("#bababa");
    }
    /*{
        Gtk::TreeViewColumn* column = m_treeViewMacro.get_column(0);
        Gtk::CellRendererText* cellrenderer =
            dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell());
        column->add_attribute(
            cellrenderer->property_foreground(), m_SamplesModel.m_color
        );
    }*/
    /*{
        Gtk::TreeViewColumn* column = m_treeViewMacro.get_column(1);
        Gtk::CellRendererText* cellrenderer =
            dynamic_cast<Gtk::CellRendererText*>(column->get_first_cell());
        column->add_attribute(
            cellrenderer->property_foreground(), m_SamplesModel.m_color
        );
    }*/
    m_treeViewMacros.set_headers_visible(true);
    m_treeViewMacros.get_selection()->signal_changed().connect(
        sigc::mem_fun(*this, &MacrosSetup::onTreeViewSelectionChanged)
    );
    m_treeViewMacros.signal_key_release_event().connect_notify(
        sigc::mem_fun(*this, &MacrosSetup::onMacroTreeViewKeyRelease)
    );
    m_treeStoreMacros->signal_row_changed().connect(
        sigc::mem_fun(*this, &MacrosSetup::onMacroTreeViewRowValueChanged)
    );
    m_ignoreTreeViewValueChange = false;

    m_scrolledWindow.add(m_treeViewMacros);
    m_scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_vbox.pack_start(m_scrolledWindow);

    m_buttonBoxL.set_layout(Gtk::BUTTONBOX_START);
    m_buttonBoxL.pack_start(m_deleteButton);
    m_buttonBoxL.pack_start(m_inverseDeleteButton);
    m_deleteButton.set_sensitive(false);
    m_inverseDeleteButton.set_sensitive(false);

    m_buttonBox.set_layout(Gtk::BUTTONBOX_END);
    m_buttonBox.pack_start(m_applyButton);
    m_buttonBox.pack_start(m_cancelButton);
    m_applyButton.set_can_default();
    m_applyButton.set_sensitive(false);
    m_applyButton.grab_focus();

#if GTKMM_MAJOR_VERSION >= 3
    m_statusLabel.set_margin_left(6);
    m_statusLabel.set_margin_right(6);
#else
    m_statusHBox.set_spacing(6);
#endif

    m_statusHBox.pack_start(m_statusLabel);
    m_statusHBox.show_all_children();

    m_footerHBox.pack_start(m_buttonBoxL, Gtk::PACK_SHRINK);
    m_footerHBox.pack_start(m_statusHBox);
    m_footerHBox.pack_start(m_buttonBox, Gtk::PACK_SHRINK);

    m_vbox.pack_start(m_footerHBox, Gtk::PACK_SHRINK);

    m_addFromClipboardButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MacrosSetup::onButtonAddFromClipboard)
    );

    m_addFromSelectionButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MacrosSetup::onButtonAddFromSelection)
    );

    m_applyButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MacrosSetup::onButtonApply)
    );

    m_cancelButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MacrosSetup::onButtonCancel)
    );

    m_deleteButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MacrosSetup::deleteSelectedRows)
    );

    m_inverseDeleteButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MacrosSetup::inverseDeleteSelectedRows)
    );

    signal_hide().connect(
        sigc::mem_fun(*this, &MacrosSetup::onWindowHide)
    );

    signal_delete_event().connect(
        sigc::mem_fun(*this, &MacrosSetup::onWindowDelete)
    );

    signal_key_press_event().connect(
        sigc::mem_fun(*this, &MacrosSetup::onKeyPressed)
    );
    signal_key_release_event().connect(
        sigc::mem_fun(*this, &MacrosSetup::onKeyReleased)
    );

    show_all_children();
    updateStatus();
}

MacrosSetup::~MacrosSetup() {
    printf("MacrosSetup destruct\n");
}

void MacrosSetup::setMacros(const std::vector<Serialization::Archive>& macros,
                            Serialization::Archive* pClipboardContent,
                            gig::DimensionRegion* pSelectedDimRgn)
{
    // copy for non-destructive editing
    m_macros = macros;

    m_clipboardContent = pClipboardContent;
    m_selectedDimRgn = pSelectedDimRgn;

    reloadTreeView();
}

void MacrosSetup::onButtonAddFromClipboard() {
    printf("+fromClipboard\n");
    if (!m_clipboardContent) return;
    if (!m_clipboardContent->rootObject()) return;
    m_macros.push_back(*m_clipboardContent);
    m_modified = true;
    reloadTreeView();
}

void MacrosSetup::onButtonAddFromSelection() {
    printf("+fromSelection\n");
    if (!m_selectedDimRgn) return;
    std::string errorText;
    try {
        Serialization::Archive archive;
        archive.serialize(m_selectedDimRgn);
        //archive.setName("Unnamed Macro");
        m_macros.push_back(archive);
        m_modified = true;
        reloadTreeView();
    } catch (Serialization::Exception e) {
        errorText = e.Message;
    } catch (...) {
        errorText = _("Unknown exception while creating macro");
    }
    if (!errorText.empty()) {
        Glib::ustring txt = _("Couldn't create macro:\n") + errorText;
        Gtk::MessageDialog msg(*this, txt, false, Gtk::MESSAGE_ERROR);
        msg.run();
    }
}

static Glib::ustring indexToAccKey(uint index) {
    if (index >= 12) return "";
    return "F" + ToString(index+1);
}

static int daysAgo(const tm& t) {
    time_t now;
    time(&now);
    tm* pNow = localtime(&now);
    if (!pNow) return 0;
    if (pNow->tm_year == t.tm_year &&
        pNow->tm_mon  == t.tm_mon &&
        pNow->tm_mday == t.tm_mday) return 0;
    time_t past = mktime((tm*)&t);
    return ceil(difftime(now, past));
}

static Glib::ustring humanShortStr(const tm& t) {
    int iDaysAgo = daysAgo(t);
    char buf[70];
    if (iDaysAgo == 0) {
        // C-Time specification for a time somewhere today (see 'man strftime()').
        if (strftime(buf, sizeof buf, _("%R"), &t))
            return buf;
    } else if (iDaysAgo == 1) {
        // C-Time specification for a time somewhere yesterday (see 'man strftime()').
        if (strftime(buf, sizeof buf, _("Yesterday %R"), &t))
            return buf;
    } else if (iDaysAgo == 2) {
        // C-Time specification for a time somewhere 2 days ago (see 'man strftime()').
        if (strftime(buf, sizeof buf, _("2 days ago %R"), &t))
            return buf;
    } else {
        // C-Time specification for a time far more than 2 days ago (see 'man strftime()').
        if (strftime(buf, sizeof buf, "%d %b %Y", &t))
            return buf;
    }
    return "";
}

void MacrosSetup::reloadTreeView() {
    m_ignoreTreeViewValueChange = true;

    m_treeStoreMacros->clear();

    for (int iMacro = 0; iMacro < m_macros.size(); ++iMacro) {
        const Serialization::Archive& macro = m_macros[iMacro];

        Gtk::TreeModel::iterator iter = m_treeStoreMacros->append();
        Gtk::TreeModel::Row row = *iter;
        row[m_treeModelMacros.m_col_key] = indexToAccKey(iMacro);
        row[m_treeModelMacros.m_col_name] = macro.name().empty() ? _("Unnamed Macro") : gig_to_utf8(macro.name());
        row[m_treeModelMacros.m_col_comment] = macro.comment().empty() ? _("No comment assigned yet.") : gig_to_utf8(macro.comment());
        row[m_treeModelMacros.m_col_created] = humanShortStr(macro.dateTimeCreated());
        row[m_treeModelMacros.m_col_modified] = humanShortStr(macro.dateTimeModified());
        row[m_treeModelMacros.m_col_index] = iMacro;
    }

    m_treeViewMacros.expand_all();

    updateStatus();

    m_ignoreTreeViewValueChange = false;
}

void MacrosSetup::onTreeViewSelectionChanged() {
    std::vector<Gtk::TreeModel::Path> v = m_treeViewMacros.get_selection()->get_selected_rows();
    const bool bValidSelection = !v.empty();
    m_deleteButton.set_sensitive(bValidSelection);
    m_inverseDeleteButton.set_sensitive(bValidSelection);
}

// Cmd key on Mac, Ctrl key on all other OSs
static const guint primaryKeyL =
    #if defined(__APPLE__)
    GDK_KEY_Meta_L;
    #else
    GDK_KEY_Control_L;
    #endif

static const guint primaryKeyR =
    #if defined(__APPLE__)
    GDK_KEY_Meta_R;
    #else
    GDK_KEY_Control_R;
    #endif

bool MacrosSetup::onKeyPressed(GdkEventKey* key) {
    //printf("key down 0x%x\n", key->keyval);
    if (key->keyval == GDK_KEY_Alt_L || key->keyval == GDK_KEY_Alt_R)
        m_altKeyDown = true;
    if (key->keyval == primaryKeyL || key->keyval == primaryKeyR)
        m_primaryKeyDown = true;
    return false;
}

bool MacrosSetup::onKeyReleased(GdkEventKey* key) {
    //printf("key up 0x%x\n", key->keyval);
    if (key->keyval == GDK_KEY_Alt_L || key->keyval == GDK_KEY_Alt_R)
        m_altKeyDown = false;
    if (key->keyval == primaryKeyL || key->keyval == primaryKeyR)
        m_primaryKeyDown = false;
    if (m_primaryKeyDown && key->keyval == GDK_KEY_b)
        onButtonAddFromClipboard();
    if (m_primaryKeyDown && key->keyval == GDK_KEY_s)
        onButtonAddFromSelection();
    return false;
}

void MacrosSetup::onMacroTreeViewKeyRelease(GdkEventKey* key) {
    if (key->keyval == GDK_KEY_BackSpace || key->keyval == GDK_KEY_Delete) {
        if (m_altKeyDown)
            inverseDeleteSelectedRows();
        else if (m_primaryKeyDown)
            deleteSelectedRows();
    }
}

void MacrosSetup::onMacroTreeViewRowValueChanged(const Gtk::TreeModel::Path& path,
                                                 const Gtk::TreeModel::iterator& iter)
{
    if (m_ignoreTreeViewValueChange) return;
    if (!iter) return;
    Gtk::TreeModel::Row row = *iter;
    Glib::ustring name = row[m_treeModelMacros.m_col_name];
    int index = row[m_treeModelMacros.m_col_index];
    m_macros[index].setName(name);
    //reloadTreeView();
    m_modified = true;
    updateStatus();
}

void MacrosSetup::deleteSelectedRows() {
    Glib::RefPtr<Gtk::TreeSelection> sel = m_treeViewMacros.get_selection();
    std::vector<Gtk::TreeModel::Path> rows = sel->get_selected_rows();
    deleteRows(rows);
}

void MacrosSetup::deleteRows(const std::vector<Gtk::TreeModel::Path>& rows) {
    m_modified = !rows.empty();
    std::set<int> macros;
    for (int r = rows.size() - 1; r >= 0; --r) {
        Gtk::TreeModel::iterator it = m_treeStoreMacros->get_iter(rows[r]);
        if (!it) continue;
        Gtk::TreeModel::Row row = *it;
        macros.insert(
            row[m_treeModelMacros.m_col_index]
        );
    }
    for (std::set<int>::const_reverse_iterator it = macros.rbegin();
         it != macros.rend(); ++it)
    {
        m_macros.erase(m_macros.begin() + *it);
    }
    reloadTreeView();
}

static bool _onEachTreeRow(const Gtk::TreeModel::Path& input, std::vector<Gtk::TreeModel::Path>* output) {
    output->push_back(input);
    return false; // continue walking the tree
}

void MacrosSetup::inverseDeleteSelectedRows() {
    // get all rows of tree view
    std::vector<Gtk::TreeModel::Path> rows;
    m_treeViewMacros.get_model()->foreach_path(
        sigc::bind(
            sigc::ptr_fun(&_onEachTreeRow),
            &rows
        )
    );

    // erase all entries from "rows" which are currently selected
    std::vector<Gtk::TreeModel::Path> vSelected = m_treeViewMacros.get_selection()->get_selected_rows();
    for (int i = rows.size() - 1; i >= 0; --i) {
        bool bIsSelected = std::find(vSelected.begin(), vSelected.end(),
                                     rows[i]) != vSelected.end();
        if (bIsSelected)
            rows.erase(rows.begin() + i);
    }

    // delete those 'inverse' selected rows
    deleteRows(rows);
}

void MacrosSetup::updateStatus() {
    m_addFromClipboardButton.set_sensitive(
        m_clipboardContent && m_clipboardContent->rootObject()
    );
    m_addFromSelectionButton.set_sensitive(m_selectedDimRgn);
    m_applyButton.set_sensitive(isModified());
    updateStatusBar();
}

void MacrosSetup::updateStatusBar() {
    // update status text
    std::string txt;
    m_statusLabel.set_markup(txt);
}

sigc::signal<void, const std::vector<Serialization::Archive>& >& MacrosSetup::signal_macros_changed()
{
    return m_macros_changed;
}

bool MacrosSetup::onWindowDelete(GdkEventAny* e) {
    //printf("onWindowDelete\n");

    if (!isModified()) return false; // propagate event further (which will close this window)

    //gchar* msg = g_strdup_printf(_("Apply changes to macro \"%s\" before closing?"),
    //                             m_macroOriginal->Name.c_str());
    gchar* msg = g_strdup_printf(_("Apply changes to macro list before closing?"));
    Gtk::MessageDialog dialog(*this, msg, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);
    g_free(msg);
    dialog.set_secondary_text(_("If you close without applying, your changes will be lost."));
    dialog.add_button(_("Close _Without Applying"), Gtk::RESPONSE_NO);
    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Apply"), Gtk::RESPONSE_YES);
    dialog.set_default_response(Gtk::RESPONSE_YES);
    int response = dialog.run();
    dialog.hide();

    // user decided to close this window without saving
    if (response == Gtk::RESPONSE_NO)
        return false; // propagate event further (which will close this window)

    // user cancelled dialog, thus don't close this window
    if (response == Gtk::RESPONSE_CANCEL) {
        show();
        return true; // drop event (prevents closing this window)
    }

    // user wants to apply the changes, afterwards close window
    if (response == Gtk::RESPONSE_YES) {
        onButtonApply();
        return false; // propagate event further (which will close this window)
    }

    // should never ever make it to this point actually
    return false;
}

bool MacrosSetup::isModified() const {
    if (m_modified) return true;
    bool bModified = false;
    for (int i = 0; i < m_macros.size(); ++i) {
        if (m_macros[i].isModified()) {
            bModified = true;
            break;
        }
    }
    return bModified;
}

void MacrosSetup::onButtonCancel() {
    bool dropEvent = onWindowDelete(NULL);
    if (dropEvent) return;
    hide();
}

void MacrosSetup::onButtonApply() {
    std::string errorText;
    try {
        for (int i = 0; i < m_macros.size(); ++i) {
            if (!m_macros[i].isModified()) continue;
            // enforce re-encoding the abstract object model and resetting the
            // 'modified' state
            m_macros[i].rawData();
        }
        m_modified = false;
    } catch (Serialization::Exception e) {
        errorText = e.Message;
    } catch (...) {
        errorText = _("Unknown exception while applying macro changes");
    }
    if (!errorText.empty()) {
        Glib::ustring txt = _("Couldn't apply macro changes:\n") + errorText;
        Gtk::MessageDialog msg(*this, txt, false, Gtk::MESSAGE_ERROR);
        msg.run();
    } else {
        // update MainWindow with edited list of macros
        m_macros_changed.emit(m_macros);
    }
    updateStatus();
}

void MacrosSetup::onWindowHide() {
    delete this; // this is the end, my friend
}
