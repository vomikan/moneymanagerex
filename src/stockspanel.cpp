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
#include "constants.h"
#include "images_list.h"
#include "mmSimpleDialogs.h"
#include "mmTips.h"
#include "stockdialog.h"
#include "util.h"
#include"mmOnline.h"
#include <algorithm>

#include "model/allmodel.h"

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

/*******************************************************/

wxBEGIN_EVENT_TABLE(StocksListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY, StocksListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, StocksListCtrl::OnListItemSelected)
    EVT_LIST_KEY_DOWN(wxID_ANY, StocksListCtrl::OnListKeyDown)
    EVT_MENU(MENU_TREEPOPUP_NEW, StocksListCtrl::OnNewStocks)
    EVT_MENU(MENU_TREEPOPUP_EDIT, StocksListCtrl::OnEditStocks)
    EVT_MENU(MENU_TREEPOPUP_DELETE, StocksListCtrl::OnDeleteStocks)
    EVT_MENU(wxID_INDEX, StocksListCtrl::OnStockWebPage)
    EVT_RIGHT_DOWN(StocksListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(StocksListCtrl::OnListLeftClick)
wxEND_EVENT_TABLE()

StocksListCtrl::~StocksListCtrl()
{
    if (m_imageList) delete m_imageList;
}

StocksListCtrl::StocksListCtrl(mmStocksPanel* sp, wxWindow *parent, wxWindowID winid)
    : mmListCtrl(parent, winid)
    , m_stock_panel(sp)
    , m_imageList(0)
{
    int x = Option::instance().getIconSize();
    m_imageList = new wxImageList(x, x);
    m_imageList->Add(mmBitmap(png::PROFIT));
    m_imageList->Add(mmBitmap(png::LOSS));
    m_imageList->Add(mmBitmap(png::EMPTY));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));

    SetImageList(m_imageList, wxIMAGE_LIST_SMALL);

    // load the global variables
    m_selected_col = Model_Setting::instance().GetIntSetting("STOCKS_SORT_COL", col_sort());
    m_asc = Model_Setting::instance().GetBoolSetting("STOCKS_ASC", true);

    m_columns.push_back(PANEL_COLUMN(" ", 25, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Name"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Current Price"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Init. Date"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Number"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Everage Price"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Current Value"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Gain/Loss"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Sector"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));


    m_col_width = "STOCKS_COL%d_WIDTH";
    m_default_sort_column = col_sort();

    for (const auto& entry : m_columns)
    {
        int count = GetColumnCount();
        InsertColumn(count
            , entry.HEADER
            , entry.FORMAT
            , Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, count), entry.WIDTH));
    }

    initVirtualListControl(-1, m_selected_col, m_asc);
    if (!m_stocks.empty())
        EnsureVisible(m_stocks.size() - 1);

}

void StocksListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    if (m_selected_row > -1)
        SetItemState(m_selected_row, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0)
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    m_stock_panel->OnListItemSelected(m_selected_row);

    bool hide_menu_item = (m_selected_row < 0);

    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _("&New Stock Investment"));
    menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Stock Investment"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Stock Investment"));
    menu.AppendSeparator();
    menu.Append(wxID_INDEX, _("Stock &Web Page"));

    menu.Enable(MENU_TREEPOPUP_NEW, hide_menu_item);
    menu.Enable(MENU_TREEPOPUP_EDIT, !hide_menu_item);
    menu.Enable(MENU_TREEPOPUP_DELETE, !hide_menu_item);
    menu.Enable(wxID_INDEX, !hide_menu_item);

    PopupMenu(&menu, event.GetPosition());

    this->SetFocus();
}

wxString StocksListCtrl::OnGetItemText(long item, long column) const
{
    int ID = m_stocks[item].STOCKID;
    Model_Ticker::Data* i = Model_Ticker::instance().get(ID);

    wxSharedPtr<Model_StockStat> s;
    double current_price = Model_StockHistory::getLastRate(i->UNIQUENAME);
    s = new Model_StockStat(i->UNIQUENAME, m_stock_panel->get_account_id(), current_price);

    Model_Currency::Data_Set c = Model_Currency::instance()
        .find(Model_Currency::CURRENCY_SYMBOL(i->CURRENCY_SYMBOL));
    Model_Currency::Data* currency = Model_Currency::instance().get(c.begin()->CURRENCYID);

    Model_Account::Data* a = Model_Account::instance().get(m_stock_panel->get_account_id());
    Model_Currency::Data* acc_currency = Model_Currency::instance().get(a->CURRENCYID);

    bool default_curr = (acc_currency == currency);

    switch (column)
    {
    case COL_SYMBOL:
        return m_stocks[item].SYMBOL;
    case COL_CURRPRICE:
        return default_curr ?
            Model_Currency::toString(current_price, currency, i->PRECISION)
            : Model_Currency::toCurrency(current_price, currency, i->PRECISION);
    case COL_DATE:
        return mmGetDateForDisplay(m_stocks[item].PURCHASEDATE);
    case COL_NUMBER:
        return Model_Currency::toString(m_stocks[item].NUMSHARES, currency
            , (m_stocks[item].NUMSHARES == floor(m_stocks[item].NUMSHARES) ? 0 : i->PRECISION));
    case COL_PRICE:
        return default_curr ?
            Model_Currency::toString(s->get_everage_price(), currency, i->PRECISION)
            : Model_Currency::toCurrency(s->get_everage_price(), currency, i->PRECISION);
    case COL_CURRVALUE:
        return default_curr ?
            Model_Currency::toString(s->get_count() * current_price, currency, i->PRECISION)
            : Model_Currency::toCurrency(s->get_count() * current_price, currency, i->PRECISION);
    case COL_GAIN_LOSS:
        return default_curr ?
            Model_Currency::toString(s->get_gain_loss(), currency, i->PRECISION)
            : Model_Currency::toCurrency(s->get_gain_loss(), currency, i->PRECISION);
    case COL_SECTOR:
        return i->SECTOR;

    default:
        break;
    }

    return wxEmptyString;
}

void StocksListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    m_stock_panel->OnListItemSelected(m_selected_row);
}

void mmStocksPanel::OnListItemSelected(int selectedIndex)
{
    updateExtraStocksData(selectedIndex);
    enableEditDeleteButtons(selectedIndex >= 0);
}

void StocksListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selected_row = -1;
        m_stock_panel->OnListItemSelected(m_selected_row);
    }
    event.Skip();
}

int StocksListCtrl::OnGetItemImage(long item) const
{
    wxSharedPtr<Model_StockStat> s;
    double current_price = Model_StockHistory::getLastRate(m_stocks[item].SYMBOL);
    s = new Model_StockStat(m_stocks[item].SYMBOL, m_stock_panel->get_account_id(), current_price);

    /* Returns the icon to be shown for each entry */
    double val = s->get_gain_loss();

    if (val > 0.0) return static_cast<int>(ico::GAIN);
    else if (val < 0.0) return static_cast<int>(ico::LOSS);
    else return static_cast<int>(ico::ZERO);
}

void StocksListCtrl::OnListKeyDown(wxListEvent& event)
{
    switch (event.GetKeyCode())
    {
        case WXK_DELETE:
            {
                wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED,  MENU_TREEPOPUP_DELETE);
                OnDeleteStocks(evt);
            }
            break;

        default:
            event.Skip();
            break;
    }
}

void StocksListCtrl::OnNewStocks(wxCommandEvent& /*event*/)
{
    mmStockDialog dlg(this, m_stock_panel->m_frame, "", m_stock_panel->get_account_id());
    dlg.ShowModal();
    int id = dlg.get_stock_id();
    doRefreshItems(id);

}

void StocksListCtrl::OnDeleteStocks(wxCommandEvent& /*event*/)
{
    if (m_selected_row == -1) return;

    wxMessageDialog msgDlg(this, _("Do you really want to delete the stock investment?")
        , _("Confirm Stock Investment Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);
    if (msgDlg.ShowModal() == wxID_YES)
    {
        Model_Stock::Data_Set s = Model_Stock::instance().find(Model_Stock::SYMBOL(m_stocks[m_selected_row].SYMBOL));
        Model_Stock::instance().Savepoint();
        for (const auto& i : s)
        {
            Model_Stock::instance().remove(i.STOCKID);
            // TODO: uttachments
        }
        Model_Stock::instance().ReleaseSavepoint();
        DeleteItem(m_selected_row);
        doRefreshItems(-1);
        m_stock_panel->m_frame->RefreshNavigationTree();
    }
}

void StocksListCtrl::OnEditStocks(wxCommandEvent& event)
{
    if (m_selected_row < 0) return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, event.GetId());
    AddPendingEvent(evt);
}

void StocksListCtrl::OnStockWebPage(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;
    const wxString stockSymbol = m_stocks[m_selected_row].SYMBOL;

    wxSharedPtr<mmWebPage> e;
    e = new mmWebPage(stockSymbol);
}

void StocksListCtrl::OnListItemActivated(wxListEvent& event)
{
        m_stock_panel->OnListItemActivated(m_selected_row);
}

void mmStocksPanel::OnListItemActivated(int selectedIndex)
{
    call_dialog(selectedIndex);
    updateExtraStocksData(selectedIndex);
}

void StocksListCtrl::OnColClick(wxListEvent& event)
{
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT)
        ColumnNr = event.GetColumn();
    else
        ColumnNr = m_ColumnHeaderNbr;
    if (0 >= ColumnNr || ColumnNr >= getColumnsNumber()) return;

    if (m_selected_col == ColumnNr && event.GetId() != MENU_HEADER_SORT) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = ColumnNr;

    Model_Setting::instance().Set("STOCKS_ASC", m_asc);
    Model_Setting::instance().Set("STOCKS_SORT_COL", m_selected_col);

    int trx_id = -1;
    if (m_selected_row>=0) trx_id = m_stocks[m_selected_row].STOCKID;
    doRefreshItems(trx_id);
    m_stock_panel->OnListItemSelected(-1);
}

void StocksListCtrl::doRefreshItems(int trx_id)
{
    int selectedIndex = initVirtualListControl(trx_id, m_selected_col, m_asc);
    long cnt = static_cast<long>(m_stocks.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = m_asc ? cnt - 1 : 0;

    if (cnt>0)
    {
        RefreshItems(0, cnt > 0 ? cnt - 1 : 0);
    }
    else
        selectedIndex = -1;

    if (selectedIndex >= 0 && cnt > 0)
    {
        SetItemState(selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selectedIndex);
    }
}

/*******************************************************/
BEGIN_EVENT_TABLE(mmStocksPanel, wxPanel)
    EVT_BUTTON(wxID_NEW,          mmStocksPanel::OnNewStocks)
    EVT_BUTTON(wxID_EDIT,         mmStocksPanel::OnEditStocks)
    EVT_BUTTON(wxID_ADD,          mmStocksPanel::OnEditStocks)
    EVT_BUTTON(wxID_VIEW_DETAILS, mmStocksPanel::OnEditStocks)
    EVT_BUTTON(wxID_DELETE,       mmStocksPanel::OnDeleteStocks)
    EVT_BUTTON(wxID_REFRESH,      mmStocksPanel::OnRefreshQuotes)
END_EVENT_TABLE()
/*******************************************************/
mmStocksPanel::mmStocksPanel(int accountID
    , mmGUIFrame* frame
    , wxWindow *parent
    , wxWindowID winid, const wxPoint& pos, const wxSize& size, long style
    , const wxString& name)
    : m_account_id(accountID)
    , m_frame(frame)
    , m_currency()
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

    listCtrlAccount_ = new StocksListCtrl(this, itemSplitterWindow10, wxID_ANY);

    wxPanel* BottomPanel = new wxPanel(itemSplitterWindow10, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    itemSplitterWindow10->SplitHorizontally(listCtrlAccount_, BottomPanel);
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

void StocksListCtrl::sortTable()
{
    std::sort(m_stocks.begin(), m_stocks.end());
    switch (m_selected_col)
    {
    case StocksListCtrl::COL_DATE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByPURCHASEDATE());
        break;
    case StocksListCtrl::COL_SYMBOL:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySYMBOL());
        break;
    case StocksListCtrl::COL_NUMBER:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByNUMSHARES());
        break;
    case StocksListCtrl::COL_PRICE:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
        {
            double valueX = x.PURCHASEPRICE;
            double valueY = y.PURCHASEPRICE;
            return valueX < valueY;
        });
        break;
    case StocksListCtrl::COL_GAIN_LOSS:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
        {
            double valueX = 0; // GetGainLoss(x);
            double valueY = 0; // GetGainLoss(y);
            return valueX < valueY;
        });
        break;
    case StocksListCtrl::COL_CURRPRICE:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
        {
            double valueX = 0.0; //TODO: // x.CURRENTPRICE;
            double valueY = 0.0; //TODO: //  y.CURRENTPRICE;
            return valueX < valueY;
        });
        break;
    case StocksListCtrl::COL_CURRVALUE:
        std::stable_sort(m_stocks.begin(), m_stocks.end()
            , [](const Model_Stock::Data& x, const Model_Stock::Data& y)
        {
            double valueX = 0.0; //TODO: //  x.CURRENTPRICE * x.NUMSHARES - x.COMMISSION;
            double valueY = 0.0; //TODO: //  y.CURRENTPRICE * y.NUMSHARES - y.COMMISSION;
            return valueX < valueY;
        });
        break;

    default:
        break;
    }
    if (!m_asc) std::reverse(m_stocks.begin(), m_stocks.end());
}

int StocksListCtrl::initVirtualListControl(int id, int col, bool asc)
{
    /* Clear all the records */
    DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(asc ? static_cast<int>(ico::ARROW_DOWN) : static_cast<int>(ico::ARROW_UP));
    SetColumn(col, item);

    std::map<wxString, Data> list;

    //TODO: Current price
    //Model_StockHistory::Data_Set histData = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(i.first));
    //std::sort(histData.begin(), histData.end(), SorterByDATE());

    Model_Stock::Data_Set stocks = Model_Stock::instance().find(Model_Stock::HELDAT(m_stock_panel->get_account_id()));
    for (const auto& entry : stocks)
    {
        Model_Ticker::Data* e = Model_Ticker::instance().get(entry.SYMBOL);
        const wxString name = e ? e->UNIQUENAME : entry.SYMBOL;
        Data i;
        if (list.find(name) != list.end())
        {
            i = list.at(name);
            i.date = i.date > entry.PURCHASEDATE ? entry.PURCHASEDATE : i.date;
            i.number += entry.NUMSHARES;
            i.commission += entry.COMMISSION;
            i.gain_loss += 0.0; //TODO: //  (entry.NUMSHARES * entry.CURRENTPRICE - entry.VALUE - entry.COMMISSION);
            i.current_value += 0.0; //TODO: //  entry.NUMSHARES * entry.CURRENTPRICE;
            i.value += 0.0; //TODO: //  entry.VALUE;
            i.purchase_price += entry.PURCHASEPRICE * fabs(entry.NUMSHARES);
        }
        else
        {
            i.date = entry.PURCHASEDATE;
            i.number = entry.NUMSHARES;
            i.commission = entry.COMMISSION;
            i.gain_loss = 0.0; //TODO: //  (entry.NUMSHARES * entry.CURRENTPRICE - entry.VALUE - entry.COMMISSION);
            i.current_value = 0.0; //TODO: //  entry.NUMSHARES * entry.CURRENTPRICE;
            i.value = 0.0; //TODO: //  entry.VALUE;
            i.purchase_price = entry.PURCHASEPRICE * fabs(entry.NUMSHARES);
        }
        i.ID = e->TICKERID;
        i.sector = e->SECTOR;
        list[name] = i;
    }

    m_stocks.clear();
    for (const auto& i : list)
    {
        Model_Stock::Data* entry = Model_Stock::instance().create();
        entry->PURCHASEDATE = i.second.date;
        entry->NUMSHARES = i.second.number;
        entry->PURCHASEPRICE = i.second.purchase_price;
        entry->HELDAT = m_stock_panel->get_account_id();
        entry->SYMBOL = i.first;
        entry->COMMISSION = i.second.commission;
        entry->STOCKID = i.second.ID;

        m_stocks.push_back(*entry);
    }

    sortTable();

    int cnt = 0, selected_item = -1;
    for (const auto& stock: m_stocks)
    {
        if (id == stock.STOCKID)
        {
            selected_item = cnt;
            break;
        }
        ++cnt;
    }

    SetItemCount(m_stocks.size());

    m_stock_panel->updateHeader();

    return selected_item;
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

void mmStocksPanel::OnEditStocks(wxCommandEvent& event)
{
    listCtrlAccount_->OnEditStocks(event);
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

    wxSharedPtr<mmOnline> o;
    o = new mmOnline();

    if (o->get_error()) {
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
    enableEditDeleteButtons(selectedIndex >= 0);
    if (selectedIndex >= 0)
    {
        const wxString additionInfo = listCtrlAccount_->getStockInfo(selectedIndex);
        stock_details_->SetLabelText(additionInfo);
    }
}

wxString StocksListCtrl::getStockInfo(int selectedIndex) const
{
    wxString additionInfo = "TBD";
    return additionInfo;
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
    Model_Stock::Data* stock = &listCtrlAccount_->m_stocks[selectedIndex];
    mmStockDialog dlg(this, m_frame, stock->SYMBOL, m_account_id);
    dlg.ShowModal();
    int id = dlg.get_stock_id();

    listCtrlAccount_->doRefreshItems(id);

}
