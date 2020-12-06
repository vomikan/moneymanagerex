/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011-2017 Nikolay
 Copyright (C) 2011-2017 Stefano Giorgio [stef145g]

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

#ifndef MM_EX_TRANSDIALOG_H_
#define MM_EX_TRANSDIALOG_H_

#include "mmcustomdata.h"
#include "defs.h"

#include "Model_Checking.h"
#include "Model_Payee.h"
#include "Model_CustomFieldData.h"
#include "Model_Splittransaction.h"

#include <wx/timectrl.h>

class wxDatePickerCtrl;
class mmTextCtrl;
class wxString;
class mmCustomData;

class mmTransDialog : public wxDialog
{
    wxDECLARE_DYNAMIC_CLASS(mmTransDialog);
    wxDECLARE_EVENT_TABLE();

public:
    mmTransDialog() {}
    virtual ~mmTransDialog();

    mmTransDialog(
        wxWindow* parent
        , int account_id
        , int transaction_id
        , bool duplicate = false
        , int type = Model_Checking::WITHDRAWAL
    );

    bool Create(wxWindow* parent
        , wxWindowID id = wxID_ANY
        , const wxString& caption = ""
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size = wxDefaultSize
        , long style = wxCAPTION | wxSYSTEM_MENU | wxCLOSE_BOX
        , const wxString& name = "mmTransDialog"
    );

    void SetDialogTitle(const wxString& title);
    int GetAccountID() { return m_trx_data.ACCOUNTID; }
    int GetToAccountID() { return m_trx_data.TOACCOUNTID; }
    int GetTransactionID() { return m_trx_data.TRANSID; }

private:
    wxSharedPtr<mmCustomData> m_custom_fields;
    void CreateControls();
    void dataToControls();
    bool doValidateData();
    void SetEventHandlers();

    void OnSplitChecked(wxCommandEvent& event);
    void OnOk(wxCommandEvent& event);
    void OnCancel(wxCommandEvent& event);
    void OnMoreFields(wxCommandEvent& event);
    void OnQuit(wxCloseEvent& event);
    void OnCategs(wxCommandEvent& event);
    void OnCurrency(wxCommandEvent& event);
    void OnAttachments(wxCommandEvent& event);
    void OnColourButton(wxCommandEvent& event);
    void OnColourSelected(wxCommandEvent& event);
    void OnAccountOrPayeeUpdated(wxCommandEvent& event);
    void OnDpcKillFocus(wxFocusEvent& event);
    void OnAutoTransNum(wxCommandEvent& event);
    void OnFrequentUsedNotes(wxCommandEvent& event);
    void OnNoteSelected(wxCommandEvent& event);
    void OnTransTypeChanged(wxCommandEvent& event);
    void OnFocusChange(wxChildFocusEvent& event);
    void OnTextEntered(wxCommandEvent& event);
    void OnAdvanceChecked(wxCommandEvent& event);
    void ActivateSplitTransactionsDlg();
    void SetTooltips();
    void SetCategoryForPayee(const Model_Payee::Data *payee = nullptr);
private:
    mmTextCtrl* m_trx_number;
    mmTextCtrl* m_trx_amount;
    mmTextCtrl* m_trx_to_amount;
    wxTextCtrl* m_trx_notes;
    wxButton* m_trx_curegory_btn;
    wxButton* m_trx_attachments_btn;
    wxButton* m_trx_colours_btn;
    wxButton* m_trx_currency_btn;
    wxComboBox* m_trx_account;
    wxComboBox* m_trx_payee_box;
    wxCheckBox* m_split_box;
    wxCheckBox* m_trx_advanced;
    wxButton* m_button_cancel;
    wxChoice* m_trx_status;
    wxChoice* m_trx_type;
    wxDatePickerCtrl* m_date_picker;
    wxTimePickerCtrl* m_time_picker;

    wxStaticText* m_account_label;
    wxStaticText* m_payee_label;

    bool m_is_transfer;
    bool m_is_new_trx;
    bool m_is_duplicate;
    bool m_is_categ_updated;
    bool m_is_advanced;

    int m_object_in_focus_id;
    int m_account_id;
    wxString m_status;

    DB_Table_CHECKINGACCOUNT_V1::Data m_trx_data;
    std::vector<Split> m_local_splits;
    Model_Currency::Data* m_currency;
    Model_Currency::Data* m_to_currency;
    int m_precision;

    std::vector<wxString> frequentNotes_;

    bool skip_date_init_;
    bool skip_account_init_;
    bool skip_amount_init_;
    bool skip_payee_init_;
    bool skip_status_init_;
    bool skip_notes_init_;
    bool skip_category_init_;
    bool skip_tooltips_init_;

    enum
    {
        /* Transaction Dialog */
        ID_TRX_TYPE = wxID_HIGHEST + 900,
        ID_TRX_TEXTNUMBER,
        ID_TRX_DATE,
        ID_TRX_TEXTNOTES,
        ID_TRX_TEXTAMOUNT,
        ID_TRX_TOTEXTAMOUNT,
        ID_TRX_STATUS,
        ID_TRX_ADVANCED,
        ID_TRX_ADVANCED_FROM,
        ID_TRX_ADVANCED_TO,
        ID_TRX_SPLIT,
        ID_TRX_TRANSNUM,
        ID_TRX_PAYEE,
        ID_TRX_FREQENTNOTES,
        ID_TRX_TIME,
        ID_TRX_CUSTOMFIELDS,
        ID_TRX_CURRENCY,
        ID_TRX_CUSTOMFIELD,
    };

};

#endif
