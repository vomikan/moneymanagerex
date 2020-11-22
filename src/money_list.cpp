/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2014-2020 Nikolay Akimov

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ********************************************************/

#include "stockspanel.h"
#include "money_list.h"
#include "paths.h"
#include "constants.h"
#include "util.h"
#include "mmex.h"
#include "mmframe.h"
#include "mmSimpleDialogs.h"
#include "splittransactionsdialog.h"
#include "transdialog.h"
#include "validators.h"
#include "attachmentdialog.h"
#include "model/allmodel.h"
#include "billsdepositsdialog.h"
#include "images_list.h"
#include <wx/clipbrd.h>

#include <wx/srchctrl.h>
#include <algorithm>
#include <wx/sound.h>

//----------------------------------------------------------------------------

wxBEGIN_EVENT_TABLE(MoneyListCtrl, mmListCtrl)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, MoneyListCtrl::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, MoneyListCtrl::OnListItemActivated)
    EVT_RIGHT_DOWN(MoneyListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(MoneyListCtrl::OnListLeftClick)
    EVT_LIST_KEY_DOWN(wxID_ANY, MoneyListCtrl::OnListKeyDown)
    EVT_MENU_RANGE(MENU_TREEPOPUP_MIN, MENU_TREEPOPUP_MAX, MoneyListCtrl::OnMenuHandler)
    EVT_CHAR(MoneyListCtrl::OnChar)
wxEND_EVENT_TABLE();

//----------------------------------------------------------------------------

MoneyListCtrl::MoneyListCtrl(
    mmStocksPanel* sp,
    wxWindow *parent,
    const wxWindowID id
) :
    mmListCtrl(parent, id),
    m_sp(sp),
    m_selectedForCopy(-1),
    m_attr1(new wxListItemAttr(*wxBLACK, mmColors::listAlternativeColor0, wxNullFont)),
    m_attr2(new wxListItemAttr(*wxBLACK, mmColors::listAlternativeColor1, wxNullFont)),
    m_attr3(new wxListItemAttr(mmColors::listFutureDateColor, mmColors::listAlternativeColor0, wxNullFont)),
    m_attr4(new wxListItemAttr(mmColors::listFutureDateColor, mmColors::listAlternativeColor1, wxNullFont)),
    m_imageList(0),
    m_sortCol(COL_DEF_SORT),
    g_sortcol(COL_DEF_SORT),
    m_prevSortCol(COL_DEF_SORT),
    g_asc(true),
    m_selectedID(-1),
    m_topItemIndex(-1)
{
    wxASSERT(m_sp);

    const wxAcceleratorEntry entries[] =
    {
        wxAcceleratorEntry(wxACCEL_CTRL, 'C', MENU_ON_COPY_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_CTRL, 'V', MENU_ON_PASTE_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_ALT,  'N', MENU_ON_NEW_TRANSACTION),
        wxAcceleratorEntry(wxACCEL_CTRL, 'D', MENU_ON_DUPLICATE_TRANSACTION)
    };

    wxAcceleratorTable tab(sizeof(entries)/sizeof(*entries), entries);
    SetAcceleratorTable(tab);

    int x = Option::instance().getIconSize();
    m_imageList.reset(new wxImageList(x, x));
    m_imageList->Add(mmBitmap(png::RECONCILED));
    m_imageList->Add(mmBitmap(png::VOID_STAT));
    m_imageList->Add(mmBitmap(png::FOLLOW_UP));
    m_imageList->Add(mmBitmap(png::EMPTY));
    m_imageList->Add(mmBitmap(png::DUPLICATE_STAT));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));
    SetImageList(m_imageList.get(), wxIMAGE_LIST_SMALL);

    createColumns(*this);

    // load the global variables
    m_selected_col = Model_Setting::instance().GetIntSetting("STOCKS_SORT_COL", col_sort());
    m_asc = Model_Setting::instance().GetBoolSetting("STOCKS_ASC", true);

    m_columns.push_back(PANEL_COLUMN(" ", 25, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Type"), 70, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Date"), 112, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Payee"), 150, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Status"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Value"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Notes"), 250, wxLIST_FORMAT_LEFT));

    m_col_width = "MONEY_COL%d_WIDTH";

    for (const auto& entry : m_columns)
    {
        int count = GetColumnCount();
        InsertColumn(count
            , entry.HEADER
            , entry.FORMAT
            , Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, count), entry.WIDTH));
    }

    m_default_sort_column = COL_DEF_SORT;
    m_today = wxDateTime::Today().FormatISODate();

    SetSingleStyle(wxLC_SINGLE_SEL, false);

    // load the global variables
    long val = COL_DEF_SORT;
    wxString strVal = Model_Setting::instance().GetStringSetting("MONEY_SORT_COL", wxString() << val);
    if (strVal.ToLong(&val)) g_sortcol = toEColumn(val);
    // --
    val = 1; // asc sorting default
    strVal = Model_Setting::instance().GetStringSetting("MONEY_ASC", wxString() << val);
    if (strVal.ToLong(&val)) g_asc = (val != 0);

    // --
    setSortColumn(g_sortcol);
    setSortOrder(g_asc);
    setColumnImage(getSortColumn()
        , getSortOrder() ? ICON_ASC : ICON_DESC); // asc\desc sort mark (arrow)

    SetFocus();
}

MoneyListCtrl::~MoneyListCtrl()
{}


void MoneyListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    m_sp->OnListItemSelected(m_selected_row);
}
//----------------------------------------------------------------------------

void MoneyListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selected_row = -1;
        m_sp->OnListItemSelected(m_selected_row);
    }
    event.Skip();
}

void MoneyListCtrl::updateExtraTransactionData(int selIndex)
{

}

void MoneyListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0)
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    updateExtraTransactionData(m_selected_row);

    bool hide_menu_item = (m_selected_row < 0);
    bool multiselect = (GetSelectedItemCount() > 1);
    bool type_transfer = false;
    bool have_category = false;
    if (m_selected_row > -1)
    {
        const Model_Checking::Data& tran = m_money.at(m_selected_row);
        if (Model_Checking::type(tran.TRANSCODE) == Model_Checking::TRANSFER)
        {
            type_transfer = true;
        }

            have_category = true;

    }
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW_WITHDRAWAL, _("&New Withdrawal"));
    menu.Append(MENU_TREEPOPUP_NEW_DEPOSIT, _("&New Deposit"));
    if (Model_Account::instance().all_account_names(true).size() > 1)
        menu.Append(MENU_TREEPOPUP_NEW_TRANSFER, _("&New Transfer"));

    menu.AppendSeparator();

    menu.Append(MENU_TREEPOPUP_EDIT2, _("&Edit Transaction"));
    if (hide_menu_item || multiselect) menu.Enable(MENU_TREEPOPUP_EDIT2, false);

    menu.Append(MENU_ON_COPY_TRANSACTION, _("&Copy Transaction"));
    if (hide_menu_item) menu.Enable(MENU_ON_COPY_TRANSACTION, false);

    menu.Append(MENU_ON_PASTE_TRANSACTION, _("&Paste Transaction"));
    if (m_selectedForCopy < 0) menu.Enable(MENU_ON_PASTE_TRANSACTION, false);

    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _("D&uplicate Transaction"));
    if (hide_menu_item || multiselect) menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);

    menu.AppendSeparator();

    wxMenu* subGlobalOpMenuDelete = new wxMenu();
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE2, _("&Delete Transaction"));
    if (hide_menu_item) subGlobalOpMenuDelete->Enable(MENU_TREEPOPUP_DELETE2, false);
    subGlobalOpMenuDelete->AppendSeparator();
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_VIEWED, _("Delete all transactions in current view"));
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_FLAGGED, _("Delete Viewed \"Follow Up\" Trans."));
    subGlobalOpMenuDelete->Append(MENU_TREEPOPUP_DELETE_UNRECONCILED, _("Delete Viewed \"Unreconciled\" Trans."));
    menu.Append(MENU_TREEPOPUP_DELETE2, _("&Delete "), subGlobalOpMenuDelete);

    PopupMenu(&menu, event.GetPosition());
    this->SetFocus();
}
//----------------------------------------------------------------------------

void MoneyListCtrl::OnMarkTransaction(wxCommandEvent& event)
{
    if (GetSelectedItemCount() > 1) return;



}
//----------------------------------------------------------------------------

void MoneyListCtrl::OnMarkAllTransactions(wxCommandEvent& event)
{
    int evt =  event.GetId();
    wxString status = "";
    if (evt ==  MENU_TREEPOPUP_MARKRECONCILED_ALL)             status = "R";
    else if (evt == MENU_TREEPOPUP_MARKVOID_ALL)               status = "V";
    else  wxASSERT(false);


    doRefreshItems();
}
//----------------------------------------------------------------------------

void MoneyListCtrl::OnColClick(wxListEvent& event)
{
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT)
        ColumnNr = event.GetColumn();
    else
        ColumnNr = m_ColumnHeaderNbr;

    if (0 > ColumnNr || ColumnNr >= COL_MAX || ColumnNr == COL_IMGSTATUS) return;

    if(g_sortcol == ColumnNr && event.GetId() != MENU_HEADER_SORT) {
        m_asc = !m_asc; // toggle sort order
    }
    g_asc = m_asc;

    m_prevSortCol = m_sortCol;
    m_sortCol = toEColumn(ColumnNr);
    g_sortcol = m_sortCol;

    Model_Setting::instance().Set("MONEY_ASC", (g_asc ? 1 : 0));
    Model_Setting::instance().Set("MONEY_SORT_COL", g_sortcol);

    doRefreshItems(m_selectedID, false);

}

//----------------------------------------------------------------------------

wxString MoneyListCtrl::OnGetItemText(long item, long column) const
{

    int from_account_id = m_money[item].ACCOUNTID;
    Model_Account::Data* fa = Model_Account::instance().get(from_account_id);
    Model_Currency::Data* currency = Model_Currency::instance().get(fa->CURRENCYID);

    const Model_Checking::Full_Data& tran = this->m_money.at(item);

    switch (column)
    {
    case COL_TYPE:
        return tran.TRANSCODE;

    case COL_DATE:
        return mmGetDateForDisplay(tran.TRANSDATE);

    case COL_STATUS:
        return tran.STATUS;

    case COL_VALUE:
        return Model_Currency::toCurrency(tran.AMOUNT, currency);

    case COL_PAYEE_STR:
        return tran.PAYEENAME;

    case COL_NOTES:
        return tran.NOTES;
    }


    return "";
}
//----------------------------------------------------------------------------


/*
    Failed wxASSERT will hang application if active modal dialog presents on screen.
    Assertion's message box will be hidden until you press tab to activate one.
*/
wxListItemAttr* MoneyListCtrl::OnGetItemAttr(long item) const
{
    if (item < 0 || item >= static_cast<int>(m_money.size())) return 0;

    const Model_Checking::Data& tran = m_money[item];
    bool in_the_future = (tran.TRANSDATE > m_today);

    // apply alternating background pattern
    int user_colour_id = tran.FOLLOWUPID;
    if (user_colour_id < 0 ) user_colour_id = 0;
    else if (user_colour_id > 7) user_colour_id = 0;

    if (user_colour_id == 0) {
        if (in_the_future){
            return (item % 2 ? m_attr3.get() : m_attr4.get());
        }
        return (item % 2 ? m_attr1.get() : m_attr2.get());
    }

    return (item % 2 ? m_attr1.get() : m_attr2.get());
}
//----------------------------------------------------------------------------
// If any of these keys are encountered, the search for the event handler
// should continue as these keys may be processed by the operating system.
void MoneyListCtrl::OnChar(wxKeyEvent& event)
{

    if (wxGetKeyState(WXK_ALT) ||
        wxGetKeyState(WXK_COMMAND) ||
        wxGetKeyState(WXK_UP) ||
        wxGetKeyState(WXK_DOWN) ||
        wxGetKeyState(WXK_LEFT) ||
        wxGetKeyState(WXK_RIGHT) ||
        wxGetKeyState(WXK_HOME) ||
        wxGetKeyState(WXK_END) ||
        wxGetKeyState(WXK_PAGEUP) ||
        wxGetKeyState(WXK_PAGEDOWN) ||
        wxGetKeyState(WXK_NUMPAD_UP) ||
        wxGetKeyState(WXK_NUMPAD_DOWN) ||
        wxGetKeyState(WXK_NUMPAD_LEFT) ||
        wxGetKeyState(WXK_NUMPAD_RIGHT) ||
        wxGetKeyState(WXK_NUMPAD_PAGEDOWN) ||
        wxGetKeyState(WXK_NUMPAD_PAGEUP) ||
        wxGetKeyState(WXK_NUMPAD_HOME) ||
        wxGetKeyState(WXK_NUMPAD_END) ||
        wxGetKeyState(WXK_DELETE) ||
        wxGetKeyState(WXK_NUMPAD_DELETE) ||
        wxGetKeyState(WXK_TAB)||
        wxGetKeyState(WXK_RETURN)||
        wxGetKeyState(WXK_NUMPAD_ENTER)||
        wxGetKeyState(WXK_SPACE)||
        wxGetKeyState(WXK_NUMPAD_SPACE)
        )
    {
        event.Skip();
    }
}
//----------------------------------------------------------------------------

void MoneyListCtrl::OnCopy()
{
    if (m_selected_row < 0) return;

    if (GetSelectedItemCount() > 1)
        m_selectedForCopy = -1;
    else
        m_selectedForCopy = m_money[m_selected_row].TRANSID;

    if (wxTheClipboard->Open())
    {
        const wxString seperator = "\t";
        wxString data = "";
        if (GetSelectedItemCount() > 1)
        {
            for (int row = 0; row < GetItemCount(); row++)
            {
                if (GetItemState(row, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
                {
                    for (int column = 0; column < static_cast<int>(m_columns.size()); column++)
                    {
                        if (GetColumnWidth(column) > 0)
                            data += OnGetItemText(row, column) + seperator;
                    }
                    data += "\n";
                }
            }
        }
        else
        {
            for (int column = 0; column < static_cast<int>(m_columns.size()); column++)
            {
                if (GetColumnWidth(column) > 0)
                    data += OnGetItemText(m_selected_row, column) + seperator;
            }
            data += "\n";
        }
        wxTheClipboard->SetData(new wxTextDataObject(data));
        wxTheClipboard->Close();
    }
}

void MoneyListCtrl::OnDuplicateTransaction()
{
    if ((m_selected_row < 0) || (GetSelectedItemCount() > 1)) return;

    int transaction_id = m_money[m_selected_row].TRANSID;
    mmTransDialog dlg(this, m_sp->get_account_id(), transaction_id, 0 /*TODO*/, true);
    if (dlg.ShowModal() == wxID_OK)
    {
        m_selected_row = dlg.GetTransactionID();
        doRefreshItems(m_selected_row);
    }
    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;
}

void MoneyListCtrl::OnPaste()
{
    if (m_selectedForCopy < 0) return;
    Model_Checking::Data* tran = Model_Checking::instance().get(m_selectedForCopy);
    if (tran)
    {
        int transactionID = OnPaste(tran);
        doRefreshItems(transactionID);
    }
}

int MoneyListCtrl::OnPaste(Model_Checking::Data* tran)
{
    bool useOriginalDate = Model_Setting::instance().GetBoolSetting(INIDB_USE_ORG_DATE_COPYPASTE, false);

    Model_Checking::Data* copy = Model_Checking::instance().clone(tran); //TODO: this function can't clone split transactions
    if (!useOriginalDate) copy->TRANSDATE = wxDateTime::Now().FormatISODate();
    if (Model_Checking::type(copy->TRANSCODE) != Model_Checking::TRANSFER) copy->ACCOUNTID = m_sp->get_account_id();
    int transactionID = Model_Checking::instance().save(copy);

    Model_Splittransaction::Cache copy_split;
    for (const auto& split_item : Model_Checking::splittransaction(tran))
    {
        Model_Splittransaction::Data *copy_split_item = Model_Splittransaction::instance().clone(&split_item);
        copy_split_item->TRANSID = transactionID;
        copy_split.push_back(copy_split_item);
    }
    Model_Splittransaction::instance().save(copy_split);

    return transactionID;
}

//----------------------------------------------------------------------------

void MoneyListCtrl::OnListKeyDown(wxListEvent& event)
{
    if (wxGetKeyState(WXK_COMMAND) || wxGetKeyState(WXK_ALT) || wxGetKeyState(WXK_CONTROL)
        || m_selected_row == -1) {
        event.Skip();
        return;
    }

    //Read status of the selected transaction
    wxString status = m_money[m_selected_row].STATUS;

    if (wxGetKeyState(wxKeyCode('R')) && status != "R") {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKRECONCILED);
        AddPendingEvent(evt);
    }
    else if (wxGetKeyState(wxKeyCode('U')) && status != "") {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKUNRECONCILED);
        AddPendingEvent(evt);
    }
    else if (wxGetKeyState(wxKeyCode('F')) && status != "F") {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP);
        AddPendingEvent(evt);
    }
    else if (wxGetKeyState(wxKeyCode('V')) && status != "V") {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKVOID);
        AddPendingEvent(evt);
    }
    else if ((wxGetKeyState(WXK_DELETE) || wxGetKeyState(WXK_NUMPAD_DELETE)) && status != "V")
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_MARKVOID);
        AddPendingEvent(evt);
    }
    else if (wxGetKeyState(WXK_DELETE) || wxGetKeyState(WXK_NUMPAD_DELETE))
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE2);
        AddPendingEvent(evt);
    }
    else {
        event.Skip();
        return;
    }

}
//----------------------------------------------------------------------------

void MoneyListCtrl::OnDeleteTransaction(wxCommandEvent& /*event*/)
{
    //check if a transaction is selected
    if (GetSelectedItemCount() < 1) return;

    m_topItemIndex = GetTopItem() + GetCountPerPage() - 1;

    Model_Checking::Data& checking_entry = m_money[m_selected_row];
    if (TransactionLocked(checking_entry.TRANSDATE))
    {
        return;
    }

    //ask if they really want to delete
    wxMessageDialog msgDlg(this
        , _("Do you really want to delete the selected transaction?")
        , _("Confirm Transaction Deletion")
        , wxYES_NO | wxYES_DEFAULT | wxICON_ERROR);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        long x = 0;
        for (const auto& i : m_money)
        {
            long transID = i.TRANSID;
            if (GetItemState(x, wxLIST_STATE_SELECTED) == wxLIST_STATE_SELECTED)
            {
                SetItemState(x, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

                // remove also removes any split transactions
                Model_Checking::instance().remove(transID);
                mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION), transID);
                if (x <= m_topItemIndex) m_topItemIndex--;
                if (!m_money.empty() && m_selected_row > 0) m_selected_row--;
                if (m_selectedForCopy == transID) m_selectedForCopy = -1;

                m_money.erase(m_money.begin() + x);
                break;
            }
            x++;
        }

        doRefreshItems();
    }
}
//----------------------------------------------------------------------------
bool MoneyListCtrl::TransactionLocked(const wxString& transdate)
{
    Model_Account::Data* acc = Model_Account::instance().get(m_sp->get_account_id());
    if (Model_Account::BoolOf(acc->STATEMENTLOCKED))
    {
        wxDateTime transaction_date;
        if (transaction_date.ParseDate(transdate))
        {
            if (transaction_date <= Model_Account::DateOf(acc->STATEMENTDATE))
            {
                wxMessageBox(wxString::Format(_(
                    "Locked transaction to date: %s\n\n"
                    "Reconciled transactions.")
                    , mmGetDateForDisplay(acc->STATEMENTDATE))
                    , _("MMEX Transaction Check"), wxOK | wxICON_WARNING);
                return true;
            }
        }
    }
    return false;
}

void MoneyListCtrl::OnEditTransaction(wxCommandEvent& event)
{
        if (m_selected_row < 0) return;

        wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, event.GetId());
        AddPendingEvent(evt);

}

void MoneyListCtrl::setColumnImage(EColumn col, int image)
{
    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(image);

    SetColumn(col, item);
}

void MoneyListCtrl::OnMarkTransaction(const wxString& status)
{

    wxString org_status = "";
    int trans_id = m_money[m_selected_row].TRANSID;
    Model_Checking::Data *trx = Model_Checking::instance().get(trans_id);
    if (trx)
    {
        org_status = trx->STATUS;
        if (org_status != status) {
            m_money[m_selected_row].STATUS = status;
            trx->STATUS = status;
            Model_Checking::instance().save(trx);
            doRefreshItems(trans_id);
        }
    }
}

void MoneyListCtrl::OnMenuHandler(wxCommandEvent& event)
{
    wxString msg = "Nothing done";
    int event_id = event.GetId();

    if (MENU_TREEPOPUP_EDIT2 == event_id) {
        int trans_id = m_money[m_selected_row].TRANSID;
        mmTransDialog dlg(this, m_sp->get_account_id(), trans_id, 0 /*TODO*/, false, Model_Checking::DEPOSIT);
        if (dlg.ShowModal() == wxID_OK) {
            doRefreshItems(dlg.GetTransactionID());
        }
        msg = "Edit transaction";
    }
    else if (MENU_TREEPOPUP_NEW_DEPOSIT == event_id) {
        mmTransDialog dlg(this, m_sp->get_account_id(), -1, 0 /*TODO*/, false, Model_Checking::DEPOSIT);
        if (dlg.ShowModal() == wxID_OK) {
            initVirtualListControl(dlg.GetTransactionID());
        }
        msg = "New deposit transaction";
    }
    else if (MENU_TREEPOPUP_NEW_WITHDRAWAL == event_id)
    {
        mmTransDialog dlg(this, m_sp->get_account_id(), -1, 0 /*TODO*/, false, Model_Checking::WITHDRAWAL);
        if (dlg.ShowModal() == wxID_OK) {
            initVirtualListControl(dlg.GetTransactionID());
        }
        msg = "New withdrawal transaction";
    }
    else if (MENU_TREEPOPUP_NEW_TRANSFER == event_id)
    {
        mmTransDialog dlg(this, m_sp->get_account_id(), -1, 0 /*TODO*/, false, Model_Checking::TRANSFER);
        if (dlg.ShowModal() == wxID_OK) {
            initVirtualListControl(dlg.GetTransactionID());
        }
        msg = "New transfer transaction";
    }
    else if (MENU_ON_DUPLICATE_TRANSACTION == event_id) {

        try {
            int trx_id = m_money.at(m_selected_row).TRANSID;
            Model_Checking::Data* trx = Model_Checking::instance().get(trx_id);
            if (trx) {
                mmTransDialog dlg(this, m_sp->get_account_id(), trx_id, 0.0, true);
                if (dlg.ShowModal() == wxID_OK) {
                    initVirtualListControl(dlg.GetTransactionID());
                }
                msg = "Duplicate transaction";
            }
        }
        catch (const std::out_of_range& oor) {
            msg = "Out of Range error: " + wxString::FromUTF8(oor.what());
        }
    }
    else if (MENU_TREEPOPUP_MARKRECONCILED == event_id) {
        OnMarkTransaction("R");
    }
    else if (MENU_ON_COPY_TRANSACTION == event_id) {
        OnCopy();
        msg = wxString::Format("Selected for copy transaction ID: %i", m_selectedForCopy);
    }
    else if (MENU_ON_PASTE_TRANSACTION == event_id) {
        OnPaste();
        msg = wxString::Format("Past transaction ID: %i", m_selectedForCopy);
    }
    else if (MENU_TREEPOPUP_MARKUNRECONCILED == event_id) {
        OnMarkTransaction("");
    }
    else if (MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP == event_id) {
        OnMarkTransaction("F");
    }
    else if (MENU_TREEPOPUP_MARKVOID == event_id) {
        OnMarkTransaction("V");
    }

    wxLogDebug(msg +"\nID: %i", event_id);
}

//----------------------------------------------------------------------------

void MoneyListCtrl::doRefreshItems(int trans_id, bool filter)
{
    m_today = wxDateTime::Today().FormatISODate();
    this->SetEvtHandlerEnabled(false);
    Hide();

    // decide whether top or down icon needs to be shown
    setColumnImage(g_sortcol, g_asc ? ICON_ASC : ICON_DESC);

    SetItemCount(m_money.size());
    Show();
    sortTable();
    //markSelectedTransaction(trans_id);

    long i = static_cast<long>(m_money.size());
    if (m_topItemIndex >= i)
        m_topItemIndex = g_asc ? i - 1 : 0;
    if (m_selected_row > i - 1) m_selected_row = -1;
    if (m_topItemIndex < m_selected_row) m_topItemIndex = m_selected_row;


    if (m_selected_row >= 0 && !m_money.empty())
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        if (m_topItemIndex < 0 || (m_topItemIndex - m_selected_row) > GetCountPerPage())
            m_topItemIndex = m_selected_row;
        EnsureVisible(m_topItemIndex);
    }

    //m_cp->setAccountSummary();
    //m_cp->updateExtraTransactionData(m_selected_row);
    this->SetEvtHandlerEnabled(true);
    Refresh();
    Update();
    SetFocus();
}

//----------------------------------------------------------------------------
void MoneyListCtrl::OnListItemActivated(wxListEvent& /*event*/)
{
    m_sp->OnListItemActivated(m_selected_row);
}

int MoneyListCtrl::initVirtualListControl(int id, int col, bool asc)
{

    int account_id = m_sp->get_account_id();

    const auto splits = Model_Splittransaction::instance().get_all();
    Model_Checking::Data_Set trans = Model_Checking::instance().find_or(Model_Checking::ACCOUNTID(account_id)
        , Model_Checking::TOACCOUNTID(account_id));

    m_money.clear();
    for (const auto& tran : trans)
    {
        Model_Checking::Full_Data full_tran(tran, splits);
        full_tran.PAYEENAME = full_tran.real_payee_name(account_id);
        double transaction_amount = Model_Checking::amount(tran, account_id);
        full_tran.AMOUNT = transaction_amount;

        m_money.push_back(full_tran);
    }

    sortTable();

    SetItemCount(m_money.size());
    if (!m_money.empty())
        EnsureVisible(m_money.size() - 1);

    return id;
}

const wxString MoneyListCtrl::getMoneyInfo(int selectedIndex) const
{
    if (m_money.size() < 1 || selectedIndex < 0) return wxEmptyString;
    auto money = m_money[selectedIndex];
    Model_Checking::Data* t = Model_Checking::instance().get(money.TRANSID);

    wxString additionInfo = t ? t->TRANSDATE : "TBD";
    return additionInfo;
}

int MoneyListCtrl::OnGetItemColumnImage(long item, long column) const
{
    if (m_money.empty())
        return ICON_NONE;

    int res = -1;
    if (column == COL_IMGSTATUS)
    {
        res = ICON_NONE;
        wxString status = OnGetItemText(item, COL_STATUS);

        if (status == "F")
            res = ICON_FOLLOWUP;
        else if (status == "R")
            res = ICON_RECONCILED;
        else if (status == "V")
            res = ICON_VOID;
        else if (status == "D")
            res = ICON_DUPLICATE;
    }

    return res;
}

void MoneyListCtrl::sortTable()
{
    switch (g_sortcol)
    {
    case COL_PAYEE_STR:
        std::stable_sort(this->m_money.begin(), this->m_money.end(), SorterByPAYEENAME());
        break;
    case COL_STATUS:
        std::stable_sort(this->m_money.begin(), this->m_money.end(), SorterBySTATUS());
        break;
    case COL_NOTES:
        std::stable_sort(this->m_money.begin(), this->m_money.end(), SorterByNOTES());
        break;
    case COL_DATE:
        std::stable_sort(this->m_money.begin(), this->m_money.end(), SorterByTRANSDATE());
        break;
    default:
        break;
    }

    if (!g_asc)
        std::reverse(this->m_money.begin(), this->m_money.end());
}

void MoneyListCtrl::createColumns(mmListCtrl &lst)
{
    for (const auto& entry : m_columns)
    {
        int count = lst.GetColumnCount();
        lst.InsertColumn(count
            , entry.HEADER
            , entry.FORMAT
            , Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, count), entry.WIDTH));
    }
}

