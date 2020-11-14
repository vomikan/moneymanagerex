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

#ifndef MM_EX_STOCKDIALOG_H_
#define MM_EX_STOCKDIALOG_H_

#include "defs.h"
#include "model/Model_Stock.h"

class wxDatePickerCtrl;
class mmTextCtrl;
class mmGUIFrame;

class mmStockDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmStockDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmStockDialog();
    mmStockDialog(wxWindow* parent
        , mmGUIFrame* gui_frame
        , int ticker_id
        , int accountID
        , const wxString& name = "mmStockDialog"
        );

    bool Create(wxWindow* parent, wxWindowID id
        , const wxString& caption
        , const wxPoint& pos
        , const wxSize& size
        , long style
        , const wxString& name = "mmStockDialog"
        );
    int get_ticker_id() const;

private:
    struct Data
    {
        int id;
        wxString DATE;
        wxString type;
        double number;
        double price;
        double commission;
        wxString notes;
    };
    std::vector<Data> m_list;

    void OnQuit(wxCloseEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnStockWebButton(wxCommandEvent& event);
    void OnStockEventEditButton(wxCommandEvent& event);
    void OnStockEventDeleteButton(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnNewEntry(wxCommandEvent& event);
    void OnStockSetup(wxCommandEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnFocusChange(wxChildFocusEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnListRightClick(wxListEvent& event);
    void OnStockItemMenu(wxCommandEvent& event);

    void CreateControls();
    void DataToControls();
    void ShowStockHistory();

    void mmStockDialog::OnBuy();
    void mmStockDialog::OnSell();
    void mmStockDialog::OnDividend();

    mmTextCtrl* m_stock_symbol_ctrl;
    wxStaticText* m_info_txt;
    wxBitmapButton* m_bAttachments;
    wxListCtrl* m_stock_event_listbox;

    //Model_Stock::Data* m_stock;
    int m_stock_id;
    wxString m_symbol;
    int m_ticker_id;
    bool m_edit;
    int m_account_id;
    int m_precision;
    mmGUIFrame* m_gui_frame;
    enum
    {
        ID_DPC_STOCK_PDATE = wxID_HIGHEST + 800,
        ID_TEXTCTRL_STOCK_SYMBOL,
        ID_TEXTCTRL_STOCK_CP,
        ID_STATIC_STOCK_VALUE,
        ID_TEXTCTRL_STOCK_CURR_PRICE,
        ID_DIALOG_STOCKS,
        ID_DPC_CP_PDATE
    };
};

#endif
