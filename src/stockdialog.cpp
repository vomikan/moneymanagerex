/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2016, 2020 Nikolay

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

#include "attachmentdialog.h"
#include "images_list.h"
#include "constants.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "paths.h"
#include "stock_item.h"
#include "stockdialog.h"
#include "transdialog.h"
#include "util.h"
#include "validators.h"
#include "mmOnline.h"

#include "model/allmodel.h"
#include "accountdialog.h"
#include "mmframe.h"
#include "stock_settings.h"

#include <wx/numdlg.h>
#include <wx/textdlg.h>
#include <wx/valnum.h>

using namespace rapidjson;
    
IMPLEMENT_DYNAMIC_CLASS(mmStockDialog, wxDialog)

wxBEGIN_EVENT_TABLE(mmStockDialog, wxDialog)
    EVT_CLOSE(mmStockDialog::OnQuit)
    EVT_BUTTON(wxID_OK, mmStockDialog::OnSave)
    EVT_BUTTON(wxID_INDEX, mmStockDialog::OnStockWebButton)
    EVT_BUTTON(wxID_SETUP, mmStockDialog::OnStockSetup)
    EVT_BUTTON(wxID_FILE, mmStockDialog::OnOpenAttachment)
    EVT_BUTTON(wxID_ADD, mmStockDialog::OnNewEntry)
    EVT_BUTTON(wxID_REMOVE, mmStockDialog::OnNewEntry)
    EVT_BUTTON(wxID_NEW, mmStockDialog::OnNewEntry)
    EVT_MENU(wxID_ANY, mmStockDialog::OnStockItemMenu)
    EVT_CHILD_FOCUS(mmStockDialog::OnFocusChange)
    EVT_LIST_ITEM_RIGHT_CLICK(wxID_ANY, mmStockDialog::OnListRightClick)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, mmStockDialog::OnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, mmStockDialog::OnListItemActivated)
wxEND_EVENT_TABLE()

mmStockDialog::mmStockDialog( )
{
}

mmStockDialog::mmStockDialog(wxWindow* parent
    , mmGUIFrame* gui_frame
    , const wxString& symbol
    , int accountID
    , const wxString& name
    )
    : m_gui_frame(gui_frame)
    , m_symbol(symbol)
    , m_edit(!symbol.empty() ? true: false)
    , m_account_id(accountID)
    , m_stock_symbol_ctrl(nullptr)
    , m_info_txt(nullptr)
    , m_bAttachments(nullptr)
    , m_stock_event_listbox(nullptr)
{
    long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxCLOSE_BOX;
    Create(parent, wxID_ANY, "", wxDefaultPosition, wxSize(600, 300), style, name);

    if (!m_edit) {
        wxCommandEvent* evt = new wxCommandEvent(wxEVT_BUTTON, wxID_SETUP);
        AddPendingEvent(*evt);
        delete evt;
    }
}

bool mmStockDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    SetIcon(mmex::getProgramIcon());

    if (m_edit) {
        DataToControls();
    }

    this->SetTitle(m_edit ? _("Edit Stock Investment") : _("New Stock Investment"));
    m_stock_symbol_ctrl->Enable(!m_edit);

    Centre();

    return TRUE;
}

void mmStockDialog::DataToControls()
{
    m_stock_symbol_ctrl->SetValue(m_symbol);

    Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    if (account) currency = Model_Account::currency(account);
    int currency_precision = Model_Currency::precision(currency);
    m_precision = currency_precision;

    Model_Ticker::Data* t = Model_Ticker::instance().get(m_symbol);
    if (t) m_precision = t->PRECISION;

    if (currency_precision < m_precision)
        currency_precision = m_precision;

    ShowStockHistory();
}

void mmStockDialog::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    this->SetSizer(mainBoxSizer);

    wxBoxSizer* leftBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(leftBoxSizer, g_flagsH);

    wxStaticBox* itemStaticBoxSizer4Static = new wxStaticBox(this, wxID_ANY, _("Stock Investment Details"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(itemStaticBoxSizer4Static, wxVERTICAL);
    leftBoxSizer->Add(itemStaticBoxSizer4, g_flagsExpand);

    wxPanel* itemPanel5 = new wxPanel(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel5, g_flagsV);

    wxFlexGridSizer* itemFlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    itemPanel5->SetSizer(itemFlexGridSizer6);

    //Symbol
    wxStaticText* symbol = new wxStaticText(itemPanel5, wxID_STATIC, _("Stock Symbol"));
    itemFlexGridSizer6->Add(symbol, g_flagsH);
    symbol->SetFont(this->GetFont().Bold());

    m_stock_symbol_ctrl = new mmTextCtrl(itemPanel5, ID_TEXTCTRL_STOCK_SYMBOL
        , "", wxDefaultPosition, wxSize(150, -1), 0);
    itemFlexGridSizer6->Add(m_stock_symbol_ctrl, g_flagsH);
    m_stock_symbol_ctrl->SetToolTip(_("Enter the stock symbol. (Optional) Include exchange. eg: IBM.BE"));

    //
    itemFlexGridSizer6->Add(new wxStaticText(itemPanel5, wxID_STATIC, _("Info")), g_flagsH);
    wxBoxSizer* iconsSizer = new wxBoxSizer(wxHORIZONTAL);
    itemFlexGridSizer6->Add(iconsSizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    m_bAttachments = new wxBitmapButton(itemPanel5, wxID_FILE, mmBitmap(png::CLIP));
    m_bAttachments->SetToolTip(_("Organize attachments of this stock"));

    wxBitmapButton* itemButton31 = new wxBitmapButton(itemPanel5, wxID_INDEX, mmBitmap(png::WEB));
    itemButton31->SetToolTip(_("Display the web page for the specified Stock symbol"));
    wxBitmapButton* setup_button = new wxBitmapButton(itemPanel5, wxID_SETUP, mmBitmap(png::RUN));
    setup_button->SetToolTip(_("Item setup"));

    iconsSizer->Add(m_bAttachments, g_flagsH);
    iconsSizer->Add(itemButton31, g_flagsH);
    iconsSizer->Add(setup_button, g_flagsH);

    m_info_txt = new wxStaticText(this, wxID_STATIC, "");
    itemStaticBoxSizer4->Add(m_info_txt, g_flagsExpand);
    m_info_txt->SetMinSize(wxSize(220, 90));
    itemStaticBoxSizer4->AddSpacer(1);

    leftBoxSizer->AddSpacer(20);

    //History Panel
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    wxStaticBox* historyStaticBox = new wxStaticBox(this, wxID_ANY, _("Stock History"));
    wxStaticBoxSizer* historyStaticBoxSizer = new wxStaticBoxSizer(historyStaticBox, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBoxSizer, g_flagsExpand);

    m_stock_event_listbox = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize
        , wxLC_REPORT | wxLC_SINGLE_SEL);

    m_stock_event_listbox->SetMinSize(wxSize(500, 150));
    historyStaticBoxSizer->Add(m_stock_event_listbox, g_flagsExpand);

    // Add first column
    wxListItem col0;
    col0.SetId(0);
    col0.SetText(_("Event"));
    col0.SetWidth(90);
    m_stock_event_listbox->InsertColumn(0, col0);

    // Add 2nd column
    wxListItem col1;
    col1.SetId(1);
    col1.SetText( _("Date") );
    col1.SetWidth(90);
    m_stock_event_listbox->InsertColumn(1, col1);

    // Add 3rd column
    wxListItem col2;
    col2.SetId(2);
    col2.SetText(_("Number")); //Purchase price
    col2.SetWidth(80);
    m_stock_event_listbox->InsertColumn(2, col2);

    // Add 4rd column
    wxListItem col3;
    col2.SetId(3);
    col3.SetText(_("Price")); //Purchase price
    col3.SetWidth(80);
    m_stock_event_listbox->InsertColumn(3, col3);

    // Add 5th column
    wxListItem col4;
    col4.SetId(4);
    col4.SetText(_("Value")); //Purchase price
    col4.SetWidth(80);
    m_stock_event_listbox->InsertColumn(4, col4);

    // Add 6th column
    wxListItem col5;
    col5.SetId(5);
    col5.SetText(_("Notes"));
    col5.SetWidth(160);
    m_stock_event_listbox->InsertColumn(5, col5);

    //History Buttons
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    historyStaticBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Centre());
    wxBoxSizer* buttons_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(buttons_sizer);


    wxStdDialogButtonSizer*  std_buttons_sizer = new wxStdDialogButtonSizer;

    wxButton* buttonAdd = new wxButton(buttons_panel, wxID_ADD, _("Buy"));
    buttonAdd->SetToolTip(_("Add new buy record"));

    wxButton* buttonEdit = new wxButton(buttons_panel, wxID_REMOVE, _("Sell"));
    buttonEdit->SetToolTip(_("Add new sell record"));

    wxButton* buttonDel = new wxButton(buttons_panel, wxID_NEW, _("Dividend"));
    buttonDel->SetToolTip(_("Add new dividend record"));

    std_buttons_sizer->Add(buttonAdd, g_flagsH);
    std_buttons_sizer->Add(buttonEdit, g_flagsH);
    std_buttons_sizer->Add(buttonDel, g_flagsH);
    buttons_sizer->Add(std_buttons_sizer);

    //OK buttons
    wxStdDialogButtonSizer*  buttonsOK_CANCEL_sizer = new wxStdDialogButtonSizer;
    leftBoxSizer->Add(buttonsOK_CANCEL_sizer, wxSizerFlags(g_flagsV).Centre());

    wxButton* itemButtonOK = new wxButton(this, wxID_OK, _("&OK "));

    if (!m_edit)
        itemButtonOK->SetFocus();
    buttonsOK_CANCEL_sizer->Add(itemButtonOK, g_flagsH);
}

void mmStockDialog::OnStockWebButton(wxCommandEvent& /*event*/)
{
    const wxString stockSymbol = m_stock_symbol_ctrl->GetValue().Trim();

    wxSharedPtr<mmWebPage> e;
    e = new mmWebPage(m_symbol);
}

void mmStockDialog::OnSave(wxCommandEvent& /*event*/)
{
    EndModal(wxID_OK);
}

void mmStockDialog::OnListItemSelected(wxListEvent& event)
{
    long selectedIndex = event.GetIndex();
    long histId = m_stock_event_listbox->GetItemData(selectedIndex);
    Model_Stock::Data* Data = Model_Stock::instance().get(histId);

    if (Data)
    {
        m_stock_id = Data->STOCKID;
    }

    /*bool en = selectedIndex > -1;
    wxButton* bE = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    if (bE) bE->Enable(en);
    wxButton* bD = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    if (bD) bD->Enable(en);
    m_bAttachments->Enable(en);
    */
}

void mmStockDialog::OnStockSetup(wxCommandEvent& /*event*/)
{
    wxString symbol = m_stock_symbol_ctrl->GetValue();

    mmStockSetup dlg(this, symbol, m_account_id);
    int d = dlg.ShowModal();
    if (!m_edit && d != static_cast<int>(wxID_OK))
    {
        wxCloseEvent evt;
        OnQuit(evt);
    }
    else
    {
        symbol = dlg.getSymbol();
        wxLogDebug("%s", symbol);
        if (!m_edit) {
            m_stock_symbol_ctrl->ChangeValue(symbol);
            m_symbol = symbol;
        }
        DataToControls();
        ShowStockHistory();
    }
}

void mmStockDialog::OnStockEventEditButton(wxCommandEvent& event)
{
    long sel_item = m_stock_event_listbox->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel_item >= 0)
    {
        int id = m_list.at(sel_item).id;
        //int id = m_stock_event_listbox->GetItemData(sel_item);
        wxString type = m_list.at(sel_item).type;

        if (type == "Dividend")
        {
            mmTransDialog dlg(this, m_account_id, id, 0, false, Model_Checking::DEPOSIT);
            dlg.ShowModal();
        }
        else if (type == "Commission")
        {
            mmTransDialog dlg(this, m_account_id, id, 0, false, Model_Checking::WITHDRAWAL);
            dlg.ShowModal();
        }
        else
        {
            mmStockItem dlg(this, m_account_id, id, m_symbol);
            dlg.ShowModal();
        }

        ShowStockHistory();
    }
}

void mmStockDialog::OnStockEventDeleteButton(wxCommandEvent& /*event*/)
{

    long sel_item = m_stock_event_listbox->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel_item >= 0)
    {

        wxString deletingItem = _("Are you sure you want to delete the item?");
        wxMessageDialog msgDlg(this, deletingItem, _("Confirm Deletion"),
            wxYES_NO | wxNO_DEFAULT | wxICON_EXCLAMATION);
        if (msgDlg.ShowModal() == wxID_YES)
        {
            Model_Stock::instance().Savepoint();

            wxString type = m_list.at(sel_item).type;
            int id = m_list.at(sel_item).id;
            if (type == "Dividend" || type == "Commission")
            {
                Model_Checking::instance().remove(id);
                mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION), id);
            }
            else
            {
                Model_Stock::instance().remove(id);
                mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::STOCK), id);
            }

            Model_Stock::instance().ReleaseSavepoint();
            m_stock_id = -1;
            ShowStockHistory();
        }
    }
}

void mmStockDialog::ShowStockHistory()
{
    m_stock_event_listbox->DeleteAllItems();
    if (m_symbol.empty())
        return;

    m_list.clear();
    double numTotal = 0.0, commTotal = 0.0;

    Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    Model_Stock::Data_Set histData = Model_Stock::instance().find(Model_Stock::SYMBOL(m_symbol));

    for (const auto& entry: histData)
    {
        Data i;
        i.id = entry.STOCKID;
        i.type = entry.NUMSHARES > 0 ? _("Buy") : _("Sale");
        i.DATE = Model_Stock::PURCHASEDATE(entry).FormatISODate();
        i.number = entry.NUMSHARES;
        i.commission = entry.COMMISSION;
        i.price = entry.PURCHASEPRICE;
        i.notes = entry.NOTES;
        m_list.push_back(i);

        numTotal += i.number;
        commTotal += i.commission;
    }


    //
    Model_Payee::Data* payee = Model_Payee::instance().get(m_symbol);
    int payee_id = payee ? payee->PAYEEID : -1;

    Model_Checking::Data_Set dividends = Model_Checking::instance().find(Model_Checking::ACCOUNTID(m_account_id)
        , Model_Checking::TRANSCODE(Model_Checking::DEPOSIT)
        , Model_Checking::PAYEEID(payee_id));

    const auto all_split = Model_Splittransaction::instance().get_all();

    for (const auto& entry : dividends)
    {
        Data i;
        i.id = entry.TRANSID;
        i.type = wxTRANSLATE("Dividend");
        i.DATE = entry.TRANSDATE;
        i.number = 1;
        i.notes = entry.NOTES;

        double price = 0.0, commission = 0.0;
        Model_Splittransaction::Data_Set splits;
        if (all_split.count(entry.id())) splits = all_split.at(entry.id());
        if (splits.empty())
        {
            price = i.type == "Dividend" ? entry.TRANSAMOUNT : -entry.TRANSAMOUNT;
        }
        else
        {
            for (const auto& item : splits)
            {
                if (item.SPLITTRANSAMOUNT > 0.0) {
                    price += item.SPLITTRANSAMOUNT;
                }
                else
                {
                    commission += -item.SPLITTRANSAMOUNT;
                }
            }
        }
        i.commission = commission;
        i.price = price;

        m_list.push_back(i);
    }

    std::stable_sort(m_list.begin(), m_list.end(), SorterByDATE());
    std::reverse(m_list.begin(), m_list.end());

    for (size_t idx = 0; idx < m_list.size(); idx++)
    {
        wxListItem item;
        item.SetId(static_cast<long>(idx));

        auto &i = m_list.at(idx);
        m_stock_event_listbox->InsertItem(item);

        m_stock_event_listbox->SetItem(static_cast<long>(idx), 0, i.type);

        m_stock_event_listbox->SetItem(static_cast<long>(idx), 1, mmGetDateForDisplay(i.DATE));
        double value = i.price * i.number + (i.type == "Dividend" ? -i.commission : i.commission);
        if (i.type != "Dividend") value = -value;
        m_stock_event_listbox->SetItem(static_cast<long>(idx), 2, Model_Account::toString(i.number, account, floor(i.number) ? 0 : m_precision));
        m_stock_event_listbox->SetItem(static_cast<long>(idx), 3, Model_Account::toString(i.price, account, m_precision));
        m_stock_event_listbox->SetItem(static_cast<long>(idx), 4, Model_Account::toString(value, account, m_precision));
        m_stock_event_listbox->SetItem(static_cast<long>(idx), 5, i.notes);
    }

    size_t rows = histData.size() - 1;
    m_stock_event_listbox->RefreshItems(0, rows);

    const wxString commStr = Model_Account::toString(commTotal, account, m_precision);
    const wxString numberStr = Model_Account::toString(numTotal, account, floor(numTotal) ? 0 : m_precision);
    wxString infoStr = "Name TBD";
    infoStr += "\n" + wxString::Format(_("Number of items: %s"), numberStr);
    infoStr += "\n" + wxString::Format(_("Commission paid: %s"), commStr);
    infoStr += "\n" + wxString("Value:");
    infoStr += "\n" + wxString("Everage Price:");
    infoStr += "\n" + wxString("Gain loss TBD:");

    m_info_txt->SetLabelText(infoStr);

}

void mmStockDialog::OnFocusChange(wxChildFocusEvent& event)
{
    m_stock_symbol_ctrl->SetValue(m_stock_symbol_ctrl->GetValue().Upper());
    event.Skip();
}

void mmStockDialog::OnQuit(wxCloseEvent& /*event*/)
{
    EndModal(wxID_CANCEL);
}

int mmStockDialog::get_stock_id() const
{
    return m_stock_id;
}

void mmStockDialog::OnListItemActivated(wxListEvent& event)
{
    OnStockEventEditButton(event);
}

void mmStockDialog::OnOpenAttachment(wxCommandEvent& event)
{
    long sel_item = m_stock_event_listbox->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (sel_item >= 0)
    {
        wxLogDebug("TBD");
    }
}

void mmStockDialog::OnListRightClick(wxListEvent& event)
{
    wxMenu menu;
    menu.Append(wxID_EDIT, _("Edit"));
    menu.AppendSeparator();
    menu.Append(wxID_DELETE, _("Delete"));

    PopupMenu(&menu);

    this->SetFocus();
}

void mmStockDialog::OnStockItemMenu(wxCommandEvent& event)
{
    int id = event.GetId();

    switch (id)
    {
    case wxID_EDIT:
        OnStockEventEditButton(event);
        break;
    case wxID_DELETE:
        OnStockEventDeleteButton(event);
        break;
    default:
        break;
    }
}

void mmStockDialog::OnNewEntry(wxCommandEvent & event)
{
    int id = event.GetId();

    switch (id)
    {
    case wxID_ADD:
        OnBuy();
        break;
    case wxID_REMOVE:
        OnSell();
        break;
    case wxID_NEW:
        OnDividend();
        break;
    default:
        break;
    }

}

void mmStockDialog::OnBuy()
{
    mmStockItem dlg(this, m_account_id, -1, m_symbol, 0);
    dlg.ShowModal();
    m_stock_id = dlg.get_id();

    ShowStockHistory();
}

void mmStockDialog::OnSell()
{
    mmStockItem dlg(this, m_account_id, -1, m_symbol, 1);
    dlg.ShowModal();
    m_stock_id = dlg.get_id();

    ShowStockHistory();

}

void mmStockDialog::OnDividend()
{

    mmTransDialog dlg(this, m_account_id, 0, 0, false, Model_Checking::DEPOSIT);
    if (dlg.ShowModal() == wxID_OK)
    {

    }
    ShowStockHistory();
}
