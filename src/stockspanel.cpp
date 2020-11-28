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
#include "transdialog.h"

#include "model/allmodel.h"

#include <wx/busyinfo.h>
#include <algorithm>

enum
{
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

mmStocksPanel::mmStocksPanel(
      mmGUIFrame* frame
    , wxWindow *parent
    , int accountID
    , wxWindowID winid)
    : m_account_id(accountID)
    , m_frame(frame)
    , m_currency()
    , m_view_mode(0)
{
    Create(parent, winid);
}

bool mmStocksPanel::Create(wxWindow *parent
    , wxWindowID winid, const wxPoint& pos
    , const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
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

void mmStocksPanel::CreateControls()
{

    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    /* ---------------------- */
    wxPanel* headerPanel = new wxPanel(this, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
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
    wxSplitterWindow* splitter_window = new wxSplitterWindow(this
        , wxID_ANY, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    m_notebook = new wxNotebook(splitter_window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    wxPanel* stocks_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(stocks_tab, _("Stocks"));
    wxBoxSizer *stocks_sizer = new wxBoxSizer(wxVERTICAL);
    stocks_tab->SetSizer(stocks_sizer);

    wxPanel* money_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(money_tab, _("Money"));

    m_stock_list = new StocksListCtrl(this, stocks_tab, wxID_ANY);
    stocks_sizer->Add(m_stock_list, g_flagsExpand);

    wxBoxSizer *money_sizer = new wxBoxSizer(wxVERTICAL);
    money_tab->SetSizer(money_sizer);

    m_listCtrlMoney = new MoneyListCtrl(this, money_tab, wxID_ANY);

    money_sizer->Add(m_listCtrlMoney, g_flagsExpand);

    //
    wxPanel* bottom_panel = new wxPanel(splitter_window, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    splitter_window->SplitHorizontally(m_notebook, bottom_panel);
    splitter_window->SetMinimumPaneSize(100);
    splitter_window->SetSashGravity(1.0);
    itemBoxSizer9->Add(splitter_window, g_flagsExpandBorder1);

    wxBoxSizer* BoxSizerVBottom = new wxBoxSizer(wxVERTICAL);
    bottom_panel->SetSizer(BoxSizerVBottom);

    wxBoxSizer* BoxSizerHBottom = new wxBoxSizer(wxHORIZONTAL);
    BoxSizerVBottom->Add(BoxSizerHBottom, g_flagsBorder1V);

    wxButton* itemButton6 = new wxButton(bottom_panel, wxID_NEW, _("&New "));
    itemButton6->SetToolTip(_("New Stock Investment"));
    BoxSizerHBottom->Add(itemButton6, 0, wxRIGHT, 5);

    wxButton* itemButton81 = new wxButton(bottom_panel, wxID_EDIT, _("&Edit "));
    itemButton81->SetToolTip(_("Edit Stock Investment"));
    BoxSizerHBottom->Add(itemButton81, 0, wxRIGHT, 5);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton(bottom_panel, wxID_DELETE, _("&Delete "));
    itemButton7->SetToolTip(_("Delete Stock Investment"));
    BoxSizerHBottom->Add(itemButton7, 0, wxRIGHT, 5);
    itemButton7->Enable(false);

    refresh_button_ = new wxBitmapButton(bottom_panel
        , wxID_REFRESH, mmBitmap(png::LED_OFF), wxDefaultPosition, wxSize(30, itemButton7->GetSize().GetY()));
    refresh_button_->SetLabelText(_("Refresh"));
    refresh_button_->SetToolTip(_("Refresh Stock Prices online"));
    BoxSizerHBottom->Add(refresh_button_, 0, wxRIGHT, 5);

    //Infobar
    stock_details_ = new wxStaticText(bottom_panel, wxID_STATIC, ""
        , wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    BoxSizerVBottom->Add(stock_details_, g_flagsExpandBorder1);

    updateExtraStocksData(-1);

}

void mmStocksPanel::doListItemSelected(int selectedIndex)
{
    updateExtraStocksData(selectedIndex);
    enableEditDeleteButtons(selectedIndex >= 0);
}

void mmStocksPanel::OnNotebookPageChanged(wxBookCtrlEvent & event)
{
    m_view_mode = event.GetSelection();
    wxLogDebug("%i Mode", m_view_mode);
}

void mmStocksPanel::doListItemActivated(int selectedIndex)
{
    call_dialog(selectedIndex);
    updateExtraStocksData(selectedIndex);
}

void mmStocksPanel::updateHeader()
{
    const Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    double account_balance = 0.0, gain_loss = 0.0;

    //Get Stock Investment Account Balance as Init Amount + sum (Value) - sum (Purchase Price)
    header_text_->SetLabelText(getPanelTitle(*account));

    //Get Init Value of the account
    account_balance = account->INITIALBAL;

    // - Income (Dividends), Withdrawal (TAX), Trasfers to other accounts 
    Model_Checking::Data_Set trans_list = Model_Checking::instance().find(Model_Checking::ACCOUNTID(m_account_id));
    for (const auto& i : trans_list)
    {
        account_balance += i.TRANSCODE == Model_Checking::all_type()[Model_Checking::DEPOSIT]
            ? i.TRANSAMOUNT : -i.TRANSAMOUNT;
    }

    auto today = wxDateTime::Today();
    // + Transfered from other accounts
    for (const auto& acc : Model_Account::instance().all())
    {
        Model_Currency::Data* currency = Model_Account::currency(acc);
        double rate = Model_CurrencyHistory::getDayRate(currency->CURRENCYID, today);
        trans_list = Model_Checking::instance().find(Model_Checking::TOACCOUNTID(m_account_id)
            , Model_Checking::ACCOUNTID(acc.ACCOUNTID));
        for (const auto& i : trans_list)
        {
            account_balance += i.TOTRANSAMOUNT * rate;
        }
    }

    Model_Stock::Data_Set investment = Model_Stock::instance().find(Model_Stock::HELDAT(m_account_id));
    for (const auto& i : investment)
    {
        Model_Ticker::Data* ticker = Model_Ticker::instance().get(i.TICKERID);
        Model_Currency::Data* currency = Model_Currency::instance().get(ticker->CURRENCYID);
        double rate = Model_CurrencyHistory::getDayRate(currency->CURRENCYID, today);
        wxSharedPtr<Model_StockStat> s;
        double current_price = Model_StockHistory::getLastRate(ticker->TICKERID);
        s = new Model_StockStat(ticker->TICKERID, m_account_id, current_price);
        double gl = s->get_gain_loss() * rate;
        gain_loss += gl;
        account_balance += gl;
    }

    const wxString accBal = wxString::Format(_("Total: %s"), Model_Account::instance().toCurrency(account_balance, account));
    wxString gail_loss_str = Model_Account::instance().toCurrency(gain_loss, account);
    gail_loss_str = wxString::Format((gain_loss < 0 ? _("Loss: %s") : _("Gain: %s")), gail_loss_str);
    header_total_->SetLabelText(wxString::Format("%s %s", accBal, gail_loss_str));
    this->Layout();
}


const wxString mmStocksPanel::getPanelTitle(const Model_Account::Data& account) const
{
    Model_Currency::Data* currency = Model_Currency::instance().get(account.CURRENCYID);
    return wxString::Format(_("Stock Portfolio: %s (%s)"), account.ACCOUNTNAME, currency->CURRENCY_SYMBOL);
}

wxString mmStocksPanel::BuildPage() const
{
    const Model_Account::Data* account = Model_Account::instance().get(m_account_id);
    return m_stock_list->BuildPage(account ? getPanelTitle(*account) : "");
}

void mmStocksPanel::OnDeleteStocks(wxCommandEvent& event)
{
    if (m_view_mode == 0)
        m_stock_list->OnDeleteStocks(event);
    else
        m_listCtrlMoney->OnDeleteTransaction(event);
}

void mmStocksPanel::OnNewStocks(wxCommandEvent& event)
{

    if (m_view_mode == 0)
    {
        m_stock_list->OnNewStocks(event);
        dataToControls();
    }
    else
    {
        m_listCtrlMoney->OnNewTransaction(event);
        dataToControls(m_listCtrlMoney->getSelectedID());
    }
}

void mmStocksPanel::OnEditRecord(wxCommandEvent& event)
{
    if (m_view_mode == 0)
        m_stock_list->OnEditStocks(event);
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

    if (m_stock_list->m_stocks.empty())
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

    if (o->get_error_code() != 0)
    {
        msg = o->get_error_str();
        return false;
    }

    // Now refresh the display
    int selected_id = -1;
    if (m_stock_list->get_selectedIndex() > -1)
        selected_id = m_stock_list->m_stocks[m_stock_list->get_selectedIndex()].STOCKID;
    m_stock_list->doRefreshItems(selected_id);

    return true;
}

void mmStocksPanel::updateExtraStocksData(int selectedIndex)
{
    if (selectedIndex >= 0)
    {
        const wxString additionInfo =
            m_view_mode == 0
            ? m_stock_list->getStockInfo(selectedIndex)
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
        Model_Stock::Data* stock = &m_stock_list->m_stocks[selectedIndex];
        mmStockDialog dlg(this, m_frame, stock->TICKERID, m_account_id);
        dlg.ShowModal();
        id = dlg.get_ticker_id();
    }
    else
    {
        Model_Checking::Data* money = &m_listCtrlMoney->m_money[selectedIndex];

        if (money)
        {
            wxLogDebug("%s %s %.2f %s"
                , money->TRANSCODE
                , money->TRANSDATE
                , money->TRANSAMOUNT
                , money->NOTES);
        }

        mmTransDialog dlg(this, m_account_id, money->TRANSID);
        if (dlg.ShowModal() == wxID_OK)
        {

        }
    }

    dataToControls();
}

void mmStocksPanel::RefreshList(int transID)
{
    dataToControls(transID);
}

void mmStocksPanel::dataToControls(int transID)
{
    m_stock_list->initVirtualListControl(transID);
    m_listCtrlMoney->initVirtualListControl(transID);
}

void mmStocksPanel::DisplayAccountDetails(int accountID)
{

    m_account_id = accountID;
    Model_Account::Data* account = Model_Account::instance().get(accountID);
    if (account)
        m_currency = Model_Account::currency(account);

    m_view_mode = 0;
    m_notebook->SetSelection(m_view_mode);
    RefreshList();

}
