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

#ifndef MM_EX_STOCKSPANEL_H_
#define MM_EX_STOCKSPANEL_H_

#include "stocks_list.h"
#include "money_list.h"
#include <wx/tglbtn.h>
#include "model/Model_Stock.h"
#include "model/Model_Currency.h"
#include "model/Model_Account.h"
#include "mmframe.h"

//class StocksListCtrl;
class mmStocksPanel;

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
public:
    void RefreshList(int transID = -1);
    void CreateControls();
    void dataToControls(int transID = -1);
    void DisplayAccountDetails(int accountID);

    /* Event handlers for Buttons */
    void OnNewStocks(wxCommandEvent& event);
    void OnDeleteStocks(wxCommandEvent& event);
    void OnEditRecord(wxCommandEvent& event);
    void OnRefreshQuotes(wxCommandEvent& event);
    //Unhide the Edit and Delete buttons if any record selected
    void enableEditDeleteButtons(bool en);
    void OnListItemActivated(int selectedIndex);
    void OnListItemSelected(int selectedIndex);
    void OnNotebookPageChanged(wxBookCtrlEvent& event);

    int get_account_id() const;
    int get_view_mode() const;
    Model_Currency::Data * m_currency;
    void updateExtraStocksData(int selIndex);
    void updateHeader();

    wxString BuildPage() const;
    mmGUIFrame* m_frame;

private:
    wxNotebook* m_notebook;
    int m_view_mode;
    int m_account_id;
    MoneyListCtrl* m_listCtrlMoney;
    StocksListCtrl* listCtrlAccount_;
    wxStaticText* stock_details_;
    void call_dialog(int selectedIndex);
    void sortTable();

    wxStaticText* header_text_;
    wxStaticText* header_total_;
    wxBitmapButton* refresh_button_;

    bool onlineQuoteRefresh(wxString& sError);
    wxString GetPanelTitle(const Model_Account::Data& account) const;

};

inline int mmStocksPanel::get_account_id() const { return m_account_id; }
inline int mmStocksPanel::get_view_mode() const { return m_view_mode; }
inline void mmStocksPanel::sortTable() {}

#endif
