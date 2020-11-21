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
#include "stocks_list.h"
#include "images_list.h"
#include "stockdialog.h"
#include "util.h"
#include "mmOnline.h"

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
    , m_sortCol(COL_DEF_SORT)
    , g_sortcol(COL_DEF_SORT)
    , m_prevSortCol(COL_DEF_SORT)
{
    int x = Option::instance().getIconSize();
    m_imageList = new wxImageList(x, x);
    m_imageList->Add(mmBitmap(png::PROFIT));
    m_imageList->Add(mmBitmap(png::LOSS));
    m_imageList->Add(mmBitmap(png::EMPTY));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));

    SetImageList(m_imageList, wxIMAGE_LIST_SMALL);


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

    for (const auto& entry : m_columns)
    {
        int count = GetColumnCount();
        InsertColumn(count
            , entry.HEADER
            , entry.FORMAT
            , Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, count), entry.WIDTH));
    }

    m_default_sort_column = col_sort();
    // load the global variables
    m_selected_col = Model_Setting::instance().GetIntSetting("STOCKS_SORT_COL", col_sort());
    m_asc = Model_Setting::instance().GetBoolSetting("STOCKS_ASC", true);

    setSortColumn(g_sortcol);
    setSortOrder(g_asc);
    setColumnImage(getSortColumn()
        , getSortOrder() ? ICON_ASC : ICON_DESC); // asc\desc sort mark (arrow)

    SetFocus();
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
    int ID = m_stocks[item].TICKERID;
    Model_Ticker::Data* i = Model_Ticker::instance().get(ID);

    Model_Account::Data* a = Model_Account::instance().get(m_stock_panel->get_account_id());
    Model_Currency::Data* acc_currency = Model_Currency::instance().get(a->CURRENCYID);

    if (i)
    {
        wxSharedPtr<Model_StockStat> s;
        double current_price = Model_StockHistory::getLastRate(i->TICKERID);
        s = new Model_StockStat(i->TICKERID, m_stock_panel->get_account_id(), current_price);

        Model_Currency::Data_Set c = Model_Currency::instance()
            .find(Model_Currency::CURRENCYID(i->CURRENCYID));
        Model_Currency::Data* currency = Model_Currency::instance().get(c.begin()->CURRENCYID);

        bool default_curr = (acc_currency == currency);

        switch (column)
        {
        case COL_SYMBOL:
            return Model_Ticker::get_ticker_symbol(m_stocks[item].TICKERID);
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
    }

    return wxEmptyString;
}

void StocksListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    m_stock_panel->OnListItemSelected(m_selected_row);
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
    double current_price = Model_StockHistory::getLastRate(m_stocks[item].TICKERID);
    s = new Model_StockStat(m_stocks[item].TICKERID, m_stock_panel->get_account_id(), current_price);

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
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE);
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
    mmStockDialog dlg(this, m_stock_panel->m_frame, -1, m_stock_panel->get_account_id());
    dlg.ShowModal();
    int id = dlg.get_ticker_id();
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
        Model_Stock::Data_Set s = Model_Stock::instance().find(Model_Stock::TICKERID(m_stocks[m_selected_row].TICKERID));
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
    int ticker_id = m_stocks[m_selected_row].TICKERID;

    wxSharedPtr<mmWebPage> e;
    e = new mmWebPage(ticker_id);
}

void StocksListCtrl::OnListItemActivated(wxListEvent& event)
{
    m_stock_panel->OnListItemActivated(m_selected_row);
}

void StocksListCtrl::OnColClick(wxListEvent& event)
{
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT)
        ColumnNr = event.GetColumn();
    else
        ColumnNr = m_ColumnHeaderNbr;
    if (0 >= ColumnNr || ColumnNr >= getColumnsNumber() || ColumnNr == COL_ICON) return;

    /* Clear previous column image */
    if (m_sortCol != ColumnNr) {
        setColumnImage(m_sortCol, -1);
    }

    if (m_selected_col == ColumnNr && event.GetId() != MENU_HEADER_SORT) m_asc = !m_asc;
    g_asc = m_asc;

    m_prevSortCol = m_sortCol;
    m_sortCol = toEColumn(ColumnNr);
    g_sortcol = m_sortCol;

   // wxListItem item;
   // item.SetMask(wxLIST_MASK_IMAGE);
   // item.SetImage(-1);
   // SetColumn(m_selected_col, item);

    m_selected_col = ColumnNr;

    Model_Setting::instance().Set("STOCKS_ASC", m_asc);
    Model_Setting::instance().Set("STOCKS_SORT_COL", m_selected_col);

    int trx_id = -1;
    if (m_selected_row >= 0) trx_id = m_stocks[m_selected_row].TICKERID;
    doRefreshItems(trx_id);
    m_stock_panel->OnListItemSelected(-1);
}

void StocksListCtrl::doRefreshItems(int trx_id)
{
    int selectedIndex = initVirtualListControl(trx_id, m_selected_col, m_asc);
    long cnt = static_cast<long>(m_stocks.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = m_asc ? cnt - 1 : 0;

    if (cnt > 0)
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

void StocksListCtrl::sortTable()
{
    std::sort(m_stocks.begin(), m_stocks.end());
    switch (m_selected_col)
    {
    case StocksListCtrl::COL_DATE:
        std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterByPURCHASEDATE());
        break;
    case StocksListCtrl::COL_SYMBOL:
        //std::stable_sort(m_stocks.begin(), m_stocks.end(), SorterBySYMBOL()); //TODO
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
    int account_id = m_stock_panel->get_account_id();

    /* Clear all the records */
    DeleteAllItems();

    wxArrayInt list;

    Model_Stock::Data_Set stocks = Model_Stock::instance()
        .find(Model_Stock::HELDAT(account_id));
    for (const auto& entry : stocks)
    {
        if (list.Index(entry.TICKERID) == wxNOT_FOUND) {
            list.Add(entry.TICKERID);
        }
    }

    m_stocks.clear();
    for (const auto& entry : list)
    {
        Model_Stock::Data* item = Model_Stock::instance().create();

        Model_Ticker::Data* t = Model_Ticker::instance().get(entry);

        wxSharedPtr<Model_StockStat> s;
        double current_price = Model_StockHistory::getLastRate(t->TICKERID);
        s = new Model_StockStat(t->TICKERID, account_id, current_price);

        item->PURCHASEDATE = s->get_init_date();
        item->NUMSHARES = s->get_count();
        item->PURCHASEPRICE = s->get_everage_price();
        item->HELDAT = m_stock_panel->get_account_id();
        item->TICKERID = entry;
        item->COMMISSION = s->get_commission();
        item->STOCKID = -1;

        m_stocks.push_back(*item);
    }

    sortTable();

    SetItemCount(m_stocks.size());
    if (!m_stocks.empty())
        EnsureVisible(m_stocks.size() - 1);

    m_stock_panel->updateHeader();


    return id;
}

wxString StocksListCtrl::getStockInfo(int selectedIndex) const
{
    auto stock = m_stocks[selectedIndex];
    Model_Ticker::Data* t = Model_Ticker::instance().get(stock.TICKERID);

    wxString additionInfo = t ? t->SOURCENAME : "$";
    return additionInfo;
}

void StocksListCtrl::setColumnImage(EColumn col, int image)
{
    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(image);

    SetColumn(col, item);
}
