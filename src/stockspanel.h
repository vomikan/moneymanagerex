/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef MM_EX_STOCKSPANEL_H_
#define MM_EX_STOCKSPANEL_H_

#include "mmpanelbase.h"
#include <wx/tglbtn.h>
#include "model/Model_Stock.h"
#include "model/Model_Currency.h"
#include "model/Model_Account.h"
#include "mmframe.h"

class wxListEvent;
class mmStocksPanel;

/* Custom ListCtrl class that implements virtual LC style */
class StocksListCtrl: public mmListCtrl
{
    DECLARE_NO_COPY_CLASS(StocksListCtrl)
    wxDECLARE_EVENT_TABLE();

public:
    StocksListCtrl(mmStocksPanel* sp, wxWindow *parent, wxWindowID winid = wxID_ANY);
    ~StocksListCtrl();

    void doRefreshItems(int trx_id = -1);
    void OnNewStocks(wxCommandEvent& event);
    void OnDeleteStocks(wxCommandEvent& event);
    void OnEditStocks(wxCommandEvent& event);
    void OnStockWebPage(wxCommandEvent& event);
    long get_selectedIndex() { return m_selected_row; }
    int getColumnsNumber() { return COL_MAX; }
    int col_sort() { return COL_DATE; }
    wxString getStockInfo(int selectedIndex) const;
    /* Helper Functions/data */
    Model_Stock::Data_Set m_stocks;
    /* updates thstockide checking panel data */
    int initVirtualListControl(int trx_id = -1, int col = 0, bool asc = true);

private:
    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemImage(long item) const;

    void OnMouseRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnColClick(wxListEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);

    mmStocksPanel* m_stock_panel;
    enum EColumn
    {
        COL_ICON = 0,
        COL_SYMBOL,
        COL_CURRPRICE,
        COL_DATE,
        COL_NUMBER,
        COL_PRICE,
        COL_CURRVALUE,
        COL_GAIN_LOSS,
        COL_SECTOR,
        COL_MAX, // number of columns
    };
    wxImageList* m_imageList;
    void sortTable();


    struct Data {
        int ID;
        wxString date;
        wxString sector;
        double number;
        double value;
        double current_value;
        double gain_loss;
        double commission;
        double purchase_price;
    };

};

/* ------------------------------------------------------- */
class mmStocksPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmStocksPanel(
        int accountID,
        mmGUIFrame* frame,
        wxWindow *parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxTAB_TRAVERSAL | wxNO_BORDER,
        const wxString& name = "mmStocksPanel"
    );
    ~mmStocksPanel();

    bool Create( wxWindow *parent, wxWindowID winid,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL | wxNO_BORDER,
                 const wxString& name = "mmStocksPanel");

    void CreateControls();

    /* Event handlers for Buttons */
    void OnNewStocks(wxCommandEvent& event);
    void OnDeleteStocks(wxCommandEvent& event);
    void OnEditStocks(wxCommandEvent& event);
    void OnRefreshQuotes(wxCommandEvent& event);
    //Unhide the Edit and Delete buttons if any record selected
    void enableEditDeleteButtons(bool en);
    void OnListItemActivated(int selectedIndex);
    void OnListItemSelected(int selectedIndex);

    int get_account_id() { return m_account_id; };
    Model_Currency::Data * m_currency;
    void updateExtraStocksData(int selIndex);
    void updateHeader();

    wxString BuildPage() const;
    mmGUIFrame* m_frame;

private:
    int m_account_id;
    StocksListCtrl* listCtrlAccount_;
    wxStaticText* stock_details_;
    void call_dialog(int selectedIndex);
    void sortTable() {}

    wxStaticText* header_text_;
    wxStaticText* header_total_;
    wxBitmapButton* refresh_button_;

    bool onlineQuoteRefresh(wxString& sError);
    wxString GetPanelTitle(const Model_Account::Data& account) const;

};

#endif
