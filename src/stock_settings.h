/*******************************************************
 Copyright (C) 2020 Nikolay Akimov

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
#pragma once

//#include "rapidjson/document.h"
//#include "option.h"
//#include <wx/webview.h>
//#include <wx/wizard.h>
//#include <wx/frame.h>
#include "defs.h"
#include "mmTextCtrl.h"
#include "model/Model_Stock.h"

//using namespace rapidjson;

class mmStockSetup : public wxDialog
{
public:
    mmStockSetup(wxWindow* parent, const wxString& symbol, int account_id);

    ~mmStockSetup();
    const wxString getSymbol() const;

private:

    void CreateControls();
    void dataToControls();
    void OnOk(wxCommandEvent& event);
    void OnCurrency(wxCommandEvent& event);
    wxString m_unique_name;
    wxString m_currency_symbol;
    wxChoice* m_choiceType;
    wxChoice* m_choiceSource;
    wxChoice* m_choiceSector;

    wxDatePickerCtrl* m_history_date_ctrl;
    mmTextCtrl* m_history_price_ctrl;
    wxListCtrl* m_price_listbox;
    mmTextCtrl* m_stock_name_ctrl;
    mmTextCtrl* m_stock_symbol_ctrl;
    mmTextCtrl* m_stock_unique_name_ctrl;
    mmTextCtrl* m_stock_notes_ctrl;
    wxButton* m_currency_button;

    int m_account_id;
    int m_precision;
    bool m_edit;

    void OnHistoryImportButton(wxCommandEvent& event);
    void OnHistoryDownloadButton(wxCommandEvent& event);
    void OnHistoryAddButton(wxCommandEvent& event);
    void OnHistoryDeleteButton(wxCommandEvent& event);
    void ShowStockHistory();
    void OnListItemSelected(wxListEvent& event);
    void OnTextEntered(wxCommandEvent& event);

    wxDECLARE_EVENT_TABLE();

};

inline const wxString mmStockSetup::getSymbol() const { return m_unique_name; }
