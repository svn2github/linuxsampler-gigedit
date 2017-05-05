/*
    Copyright (c) MMXVII Christian Schoenebeck

    This file is part of "gigedit" and released under the terms of the
    GNU General Public License version 2.
*/

#include "MacroEditor.h"
#include "global.h"
#include <assert.h>

MacroEditor::MacroEditor() :
    m_macroOriginal(NULL),
    m_statusLabel("",  Gtk::ALIGN_START),
    m_applyButton(_("_Apply"), true),
    m_cancelButton(_("_Cancel"), true)
{
    add(m_vbox);

    set_default_size(800, 600);

    // create Macro treeview (including its data model)
    m_treeStoreMacro = MacroTreeStore::create(m_treeModelMacro);
    m_treeViewMacro.set_model(m_treeStoreMacro);
    m_treeViewMacro.get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
    //m_treeViewMacro.set_tooltip_text(_(""));
    m_treeViewMacro.append_column(_("Key"), m_treeModelMacro.m_col_name);
    m_treeViewMacro.append_column(_("Type"), m_treeModelMacro.m_col_type);
    m_treeViewMacro.append_column_editable(_("Value"), m_treeModelMacro.m_col_value);
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
    m_treeViewMacro.set_headers_visible(true);
    /*m_treeViewMacro.signal_button_press_event().connect_notify(
        sigc::mem_fun(*this, &MainWindow::on_sample_treeview_button_release)
    );*/
    /*m_refSamplesTreeModel->signal_row_changed().connect(
        sigc::mem_fun(*this, &MainWindow::sample_name_changed)
    );*/

    m_scrolledWindow.add(m_treeViewMacro);
    m_scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    m_vbox.pack_start(m_scrolledWindow);

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

    m_footerHBox.pack_start(m_statusHBox);
    m_footerHBox.pack_start(m_buttonBox, Gtk::PACK_SHRINK);

    m_vbox.pack_start(m_footerHBox, Gtk::PACK_SHRINK);

    m_applyButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MacroEditor::onButtonApply)
    );

    m_cancelButton.signal_clicked().connect(
        sigc::mem_fun(*this, &MacroEditor::onButtonCancel)
    );

    signal_hide().connect(
        sigc::mem_fun(*this, &MacroEditor::onWindowHide)
    );

    signal_delete_event().connect(
        sigc::mem_fun(*this, &MacroEditor::onWindowDelete)
    );

    show_all_children();
    updateStatus();
}

MacroEditor::~MacroEditor() {
    printf("MacroEditor destruct\n");
}

void MacroEditor::setMacro(Serialization::Archive* macro) {
    m_macroOriginal = macro;
    if (!macro) {
        set_title(_("No Macro"));
        return;
    }

    //set_title(std::string(_("Macro Editor:")) + " \"" + macro->name() + "\"");
    set_title(std::string(_("Macro Editor:")));

    // copy for non-destructive editing
    m_macro = *macro;

    reloadTreeView();
}

void MacroEditor::buildTreeView(const Gtk::TreeModel::Row& parentRow, const Serialization::Object& parentObject) {
    for (int iMember = 0; iMember < parentObject.members().size(); ++iMember) {
        const Serialization::Member& member = parentObject.members()[iMember];
        const Serialization::Object& object = m_macro.objectByUID(member.uid());
        Gtk::TreeModel::iterator iterRow = m_treeStoreMacro->append(parentRow.children());
        Gtk::TreeModel::Row row = *iterRow;
        row[m_treeModelMacro.m_col_name] = gig_to_utf8(member.name());
        row[m_treeModelMacro.m_col_type] = gig_to_utf8(member.type().asLongDescr());
        row[m_treeModelMacro.m_col_uid]  = object.uid();
        if (object.type().isClass()) {
            row[m_treeModelMacro.m_col_value] = "(class)";
            buildTreeView(row, object);
        } else {
            row[m_treeModelMacro.m_col_value] = m_macro.valueAsString(object);
        }
    }
}

void MacroEditor::reloadTreeView() {
    m_treeStoreMacro->clear();

    const Serialization::Object& rootObject = m_macro.rootObject();

    Gtk::TreeModel::iterator iterRoot = m_treeStoreMacro->append();
    Gtk::TreeModel::Row rowRoot = *iterRoot;
    rowRoot[m_treeModelMacro.m_col_name]  = "(Root)";
    rowRoot[m_treeModelMacro.m_col_type]  = gig_to_utf8(rootObject.type().asLongDescr());
    rowRoot[m_treeModelMacro.m_col_value] = "";
    rowRoot[m_treeModelMacro.m_col_uid]   = rootObject.uid();

    buildTreeView(rowRoot, rootObject);

    m_treeViewMacro.expand_all();

    updateStatus();
}

void MacroEditor::updateStatus() {
    m_applyButton.set_sensitive(isModified());
    updateStatusBar();
}

void MacroEditor::updateStatusBar() {
    // update status text
    std::string txt;
    m_statusLabel.set_markup(txt);
}

bool MacroEditor::onWindowDelete(GdkEventAny* e) {
    //printf("onWindowDelete\n");

    if (!isModified()) return false; // propagate event further (which will close this window)

    //gchar* msg = g_strdup_printf(_("Apply changes to macro \"%s\" before closing?"),
    //                             m_macroOriginal->Name.c_str());
    gchar* msg = g_strdup_printf(_("Apply changes to macro before closing?"));
    Gtk::MessageDialog dialog(*this, msg, false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_NONE);
    g_free(msg);
    dialog.set_secondary_text(_("If you close without applying, your changes will be lost."));
    dialog.add_button(_("Close _Without Applying"), Gtk::RESPONSE_NO);
    dialog.add_button(_("_Cancel"), Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("_Apply"), Gtk::RESPONSE_YES);
    dialog.set_default_response(Gtk::RESPONSE_YES);
    int response = dialog.run();
    dialog.hide();

    // user decided to close macro editor without saving
    if (response == Gtk::RESPONSE_NO)
        return false; // propagate event further (which will close this window)

    // user cancelled dialog, thus don't close macro editor
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

bool MacroEditor::isModified() const {
    return m_macro.isModified();
}

void MacroEditor::onButtonCancel() {
    bool dropEvent = onWindowDelete(NULL);
    if (dropEvent) return;
    hide();
}

void MacroEditor::onButtonApply() {
    //m_macro.encode();
    *m_macroOriginal = m_macro;
}

void MacroEditor::onWindowHide() {
    delete this; // this is the end, my friend
}
