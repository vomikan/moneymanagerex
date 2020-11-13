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

#include "defs.h"
#include "mmTextCtrl.h"
#include "model/Model_Stock.h"
#include <wx/webview.h>

class mmStockItem : public wxDialog
{
public:
    mmStockItem(wxWindow* parent, int acc, int id, int ticker_id, int type = 0);

    ~mmStockItem();
    int get_id() const;

private:

    void CreateControls();
    void OnOk(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);

    int m_id;
    int m_acc;
    int m_type;
    int m_share_precision;
    int m_ticker_id;
    wxChoice* m_choiceType;

    wxDatePickerCtrl* m_purchase_date_ctrl;
    mmTextCtrl* m_num_shares_ctrl;
    mmTextCtrl* m_purchase_price_ctrl;
    mmTextCtrl* m_commission_ctrl;
    mmTextCtrl* m_stock_symbol_ctrl;
    mmTextCtrl* m_notes_ctrl;
    wxWebView* browser_;

    void OnTextEntered(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void fillControls();

    wxDECLARE_EVENT_TABLE();

    enum
    {
        ID_STOCK_DATE = wxID_HIGHEST + 800,
        ID_TEXTCTRL_NUMBER_SHARES,
        ID_TEXTCTRL_STOCK_PP,
        ID_TEXTCTRL_STOCK_COMMISSION,
    };

};

inline int mmStockItem::get_id() const { return m_id; }
