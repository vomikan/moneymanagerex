/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2010-2020 Nikolay

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
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmTips.h"
#include "stockdialog.h"
#include "util.h"
#include "mmOnline.h"

#include "model/allmodel.h"

#include <wx/busyinfo.h>
#include <algorithm>

enum {
    IDC_PANEL_STOCKS_LISTCTRL = wxID_HIGHEST + 1900,
    MENU_TREEPOPUP_EDIT,
    MENU_TREEPOPUP_DELETE,
    MENU_TREEPOPUP_NEW,
    MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
    MENU_HEADER_HIDE,
    MENU_HEADER_SORT,
    MENU_HEADER_RESET,
};

enum class ico { GAIN, LOSS, ZERO, ARROW_UP, ARROW_DOWN };

BEGIN_EVENT_TABLE(mmStocksPanel, wxPanel)
EVT_BUTTON(wxID_NEW, mmStocksPanel::OnNewStocks)
EVT_BUTTON(wxID_EDIT, mmStocksPanel::OnEditRecord)
EVT_BUTTON(wxID_ADD, mmStocksPanel::OnEditRecord)
EVT_BUTTON(wxID_VIEW_DETAILS, mmStocksPanel::OnEditRecord)
EVT_BUTTON(wxID_DELETE, mmStocksPanel::OnDeleteStocks)
EVT_BUTTON(wxID_REFRESH, mmStocksPanel::OnRefreshQuotes)
EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, mmStocksPanel::OnNotebookPageChanged)
END_EVENT_TABLE()

mmStocksPanel::mmStocksPanel(int accountID
    , mmGUIFrame* frame
    , wxWindow *parent
    , wxWindowID winid, const wxPoint& pos, const wxSize& size, long style
    , const wxString& name)
    : m_account_id(accountID)
    , m_frame(frame)
    , m_currency()
    , m_view_mode(0)
{
    Create(parent, winid, pos, size, style, name);
}

bool mmStocksPanel::Create(wxWindow *parent
    , wxWindowID winid, const wxPoint& pos
    , const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    this->windowsFreezeThaw();

    Model_Account::Data *account = Model_Account::instance().get(m_account_id);
    if (account)
        m_currency = Model_Account::currency(account);
    else
        m_currency = Model_Currency::GetBaseCurrency();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    dataToControls();

    this->windowsFreezeThaw();
    Model_Usage::instance().pageview(this);
    return TRUE;
}

mmStocksPanel::~mmStocksPanel()
{
}

void mmStocksPanel::dataToControls()
{
    listCtrlAccount_->initVirtualListControl();

    m_listCtrlMoney->initVirtualListControl();

}

void mmStocksPanel::CreateControls()
{

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY
        , wxDefaultPosition , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, 0, wxALIGN_LEFT);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    header_text_ = new wxStaticText(headerPanel, wxID_STATIC, "");
    header_text_->SetFont(this->GetFont().Larger().Bold());

    header_total_ = new wxStaticText(headerPanel, wxID_STATIC, "");

    wxBoxSizer* itemBoxSizerHHeader = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerHHeader->Add(header_text_, 1, wxALIGN_CENTER_VERTICAL | wxALL, 1);

    itemBoxSizerVHeader->Add(itemBoxSizerHHeader, 1, wxEXPAND, 1);
    itemBoxSizerVHeader->Add(header_total_, 1, wxALL, 1);

    /* ---------------------- */
    wxSplitterWindow* itemSplitterWindow10 = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    wxNotebook* m_notebook = new wxNotebook(itemSplitterWindow10, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    wxPanel* notes_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(notes_tab, _("Stocks"));
    wxBoxSizer *notes_sizer = new wxBoxSizer(wxVERTICAL);
    notes_tab->SetSizer(notes_sizer);
    
    wxPanel* others_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(others_tab, _("Money"));

    listCtrlAccount_ = new StocksListCtrl(this, notes_tab, wxID_ANY);
    notes_sizer->Add(listCtrlAccount_, g_flagsExpand);

    wxBoxSizer *others_sizer = new wxBoxSizer(wxVERTICAL);
    others_tab->SetSizer(others_sizer);

    m_listCtrlMoney = new MoneyListCtrl(this, others_tab, wxID_ANY);
    m_listCtrlMoney->SetMinSize(wxSize(500, 150));

    others_sizer->Add(m_listCtrlMoney, g_flagsExpand);

    //
    wxPanel* BottomPanel = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    itemSplitterWindow10->SplitHorizontally(m_notebook, BottomPanel);
    itemSplitterWindow10->SetMinimumPaneSize(100);
    itemSplitterWindow10->SetSashGravity(1.0);
    itemBoxSizer9->Add(itemSplitterWindow10, g_flagsExpandBorder1);

    wxBoxSizer* BoxSizerVBottom = new wxBoxSizer(wxVERTICAL);
    BottomPanel->SetSizer(BoxSizerVBottom);

    wxBoxSizer* BoxSizerHBottom = new wxBoxSizer(wxHORIZONTAL);
    BoxSizerVBottom->Add(BoxSizerHBottom, g_flagsBorder1V);

    wxButton* itemButton6 = new wxButton(BottomPanel, wxID_NEW, _("&New "));
    itemButton6->SetToolTip(_("New Stock Investment"));
    BoxSizerHBottom->Add(itemButton6, 0, wxRIGHT, 5);

    wxButton* itemButton81 = new wxButton(BottomPanel, wxID_EDIT, _("&Edit "));
    itemButton81->SetToolTip(_("Edit Stock Investment"));
    BoxSizerHBottom->Add(itemButton81, 0, wxRIGHT, 5);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton(BottomPanel, wxID_DELETE, _("&Delete "));
    itemButton7->SetToolTip(_("Delete Stock Investment"));
    BoxSizerHBottom->Add(itemButton7, 0, wxRIGHT, 5);
    itemButton7->Enable(false);

    refresh_button_ = new wxBitmapButton(BottomPanel
        , wxID_REFRESH, mmBitmap (png::LED_OFF), wxDefaultPosition, wxSize(30, itemButton7->GetSize().GetY()));
    refresh_button_->SetLabelText(_("Refresh"));
    refresh_button_->SetToolTip(_("Refresh Stock Prices online"));
    BoxSizerHBottom->Add(refresh_button_, 0, wxRIGHT, 5);

    //Infobar
    stock_details_ = new wxStaticText(BottomPanel, wxID_STATIC, ""
        , wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    BoxSizerVBottom->Add(stock_details_, g_flagsExpandBorder1);

    updateExtraStocksData(-1);

}

void mmStocksPanel::OnListItemSelected(int selectedIndex)
{
    updateExtraStocksData(selectedIndex);
    enableEditDeleteButtons(selectedIndex >= 0);
}

void mmStocksPanel::OnNotebookPageChanged(wxBookCtrlEvent & event)
{
    m_view_mode = event.GetSelection();
    wxLogDebug("%i Mode", m_view_mode);
}

void mmStocksPanel::OnListItemActivated(int selectedIndex)
{
    call_dialog(selectedIndex);
    updateExtraStocksData(selectedIndex);
}

void mmStocksPanel::updateHeader()
{
    const Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    double initVal = 0.0;

    //We need money amount and Total amount

    //Get Stock Investment Account Balance as Init Amount + sum (Value) - sum (Purchase Price)
    std::pair<double, double> investment_balance;
    if (account)
    {
        header_text_->SetLabelText(GetPanelTitle(*account));
        //Get Init Value of the account
        initVal = account->INITIALBAL;
        investment_balance = Model_Account::investment_balance(account);
    }

    // - Income (Dividends), Withdrawal (TAX), Trasfers to other accounts 
    Model_Checking::Data_Set trans_list = Model_Checking::instance().find(Model_Checking::ACCOUNTID(m_account_id));
    for (const auto& i : trans_list)
    {
        initVal += i.TRANSCODE == Model_Checking::all_type()[Model_Checking::DEPOSIT]
            ? i.TRANSAMOUNT : -i.TRANSAMOUNT;
    }
    // + Transfered from other accounts
    trans_list = Model_Checking::instance().find(Model_Checking::TOACCOUNTID(m_account_id));
    for (const auto& i : trans_list)
    {
        initVal += i.TOTRANSAMOUNT;
    }

    Model_Stock::Data_Set investment = Model_Stock::instance().find(Model_Stock::HELDAT(m_account_id));
    for (const auto& i : investment)
    {
        initVal -= i.PURCHASEPRICE * i.NUMSHARES + i.COMMISSION;
    }

    wxString accBal = Model_Account::instance().toCurrency(initVal, account);
    header_total_->SetLabelText(accBal);
    this->Layout();
}


wxString mmStocksPanel::GetPanelTitle(const Model_Account::Data& account) const
{
    return wxString::Format(_("Stock Portfolio: %s"), account.ACCOUNTNAME);
}

wxString mmStocksPanel::BuildPage() const
{ 
    const Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    return listCtrlAccount_->BuildPage(account ? GetPanelTitle(*account) : "");
}

void mmStocksPanel::OnDeleteStocks(wxCommandEvent& event)
{
    listCtrlAccount_->OnDeleteStocks(event);
}

void mmStocksPanel::OnNewStocks(wxCommandEvent& event)
{
    listCtrlAccount_->OnNewStocks(event);
}

void mmStocksPanel::OnEditRecord(wxCommandEvent& event)
{
    if (m_view_mode == 0)
        listCtrlAccount_->OnEditStocks(event);
    else
        m_listCtrlMoney->OnEditTransaction(event);
}

void mmStocksPanel::OnRefreshQuotes(wxCommandEvent& WXUNUSED(event))
{
    wxString sError = "";
    bool ok = onlineQuoteRefresh(sError);
    if (ok)
    {
        const wxString header = _("Stock prices successfully updated");
        stock_details_->SetLabelText(header);
        wxMessageDialog msgDlg(this, sError, header);
        msgDlg.ShowModal();
    }
    else
    {
        refresh_button_->SetBitmapLabel(mmBitmap(png::LED_RED));
        stock_details_->SetLabelText(sError);
        mmErrorDialogs::MessageError(this, sError, _("Error"));
    }
}

/*** Trigger a quote download ***/
bool mmStocksPanel::onlineQuoteRefresh(wxString& msg)
{
    wxString base_currency_symbol;
    if (!Model_Currency::GetBaseCurrencySymbol(base_currency_symbol))
    {
        msg = _("Could not find base currency symbol!");
        return false;
    }

    if (listCtrlAccount_->m_stocks.empty())
    {
        msg = _("Nothing to update");
        return false;
    }

    refresh_button_->SetBitmapLabel(mmBitmap(png::LED_YELLOW));
    stock_details_->SetLabelText(_("Connecting..."));

    wxBusyInfo info
#if (wxMAJOR_VERSION == 3 && wxMINOR_VERSION >= 1)
        (
            wxBusyInfoFlags()
            .Parent(this)
            .Title(_("Downloading stock prices"))
            .Text(_("Please wait..."))
            .Foreground(*wxWHITE)
            .Background(wxColour(104, 179, 51))
            .Transparency(4 * wxALPHA_OPAQUE / 5)
            );
#else
        (_("Downloading stock prices"), this);
#endif

    wxSharedPtr<mmOnline> o;
    o = new mmOnline();

    if (o->get_error_code() != 0) {
        msg = o->get_error_str();
        return false;
    }

    // Now refresh the display
    int selected_id = -1;
    if (listCtrlAccount_->get_selectedIndex() > -1)
        selected_id = listCtrlAccount_->m_stocks[listCtrlAccount_->get_selectedIndex()].STOCKID;
    listCtrlAccount_->doRefreshItems(selected_id);

    return true;
}

void mmStocksPanel::updateExtraStocksData(int selectedIndex)
{
    if (selectedIndex >= 0)
    {
        const wxString additionInfo =
            m_view_mode == 0
            ? listCtrlAccount_->getStockInfo(selectedIndex)
            : m_listCtrlMoney->getMoneyInfo(selectedIndex);
        stock_details_->SetLabelText(additionInfo);
    }
}


void mmStocksPanel::enableEditDeleteButtons(bool en)
{
    wxButton* bN = static_cast<wxButton*>(FindWindow(wxID_NEW));
    wxButton* bE = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    wxButton* bA = static_cast<wxButton*>(FindWindow(wxID_ADD));
    wxButton* bV = static_cast<wxButton*>(FindWindow(wxID_VIEW_DETAILS));
    wxButton* bD = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    wxButton* bM = static_cast<wxButton*>(FindWindow(wxID_MOVE_FRAME));
    if (bN) bN->Enable(!en);
    if (bE) bE->Enable(en);
    if (bA) bA->Enable(en);
    if (bV) bV->Enable(en);
    if (bD) bD->Enable(en);
    if (bM) bM->Enable(en);
    if (!en)
    {
        stock_details_->SetLabelText(_(STOCKTIPS[rand() % (sizeof(STOCKTIPS) / sizeof(wxString))]));
    }
}

void mmStocksPanel::call_dialog(int selectedIndex)
{

    int id = -1;

    if (m_view_mode == 0)
    {
        Model_Stock::Data* stock = &listCtrlAccount_->m_stocks[selectedIndex];
        mmStockDialog dlg(this, m_frame, stock->TICKERID, m_account_id);
        dlg.ShowModal();
        id = dlg.get_ticker_id();
    }
    else
    {
        Model_Checking::Data* money = &m_listCtrlMoney->m_money[selectedIndex];

        if (money) {
            wxLogDebug("%s %s %.2f %s"
            , money->TRANSCODE
            , money->TRANSDATE
            , money->TRANSAMOUNT
            , money->NOTES);
        }
        //mmMoneyDialog dlg(this, m_frame, money->TRANSID, m_account_id);
        //id = dlg.get_ticker_id();
    }


    listCtrlAccount_->doRefreshItems();

}
