/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2011-2018 Nikolay Akimov
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

#include "transdialog.h"
#include "attachmentdialog.h"
#include "categdialog.h"
#include "constants.h"
#include "images_list.h"
#include "maincurrencydialog.h"
#include "mmcustomdata.h"
#include "mmSimpleDialogs.h"
#include "mmTextCtrl.h"
#include "paths.h"
#include "splittransactionsdialog.h"
#include "util.h"
#include "validators.h"
#include "webapp.h"

#include "option.h"
#include "model/Model_Account.h"
#include "model/Model_Attachment.h"
#include "model/Model_Category.h"
#include "model/Model_CurrencyHistory.h"
#include "model/Model_CustomFieldData.h"
#include "model/Model_Subcategory.h"
#include "model/Model_Ticker.h"

#include <wx/numformatter.h>
#include <wx/collpane.h>

wxIMPLEMENT_DYNAMIC_CLASS(mmTransDialog, wxDialog);

wxBEGIN_EVENT_TABLE(mmTransDialog, wxDialog)
    EVT_CHILD_FOCUS(mmTransDialog::OnFocusChange)
    EVT_COMBOBOX(wxID_ANY, mmTransDialog::OnAccountOrPayeeUpdated)
    EVT_BUTTON(wxID_VIEW_DETAILS, mmTransDialog::OnCategs)
    EVT_CHOICE(ID_TRX_TYPE, mmTransDialog::OnTransTypeChanged)
    EVT_CHECKBOX(ID_TRX_ADVANCED, mmTransDialog::OnAdvanceChecked)
    EVT_CHECKBOX(wxID_FORWARD, mmTransDialog::OnSplitChecked)
    EVT_BUTTON(wxID_FILE, mmTransDialog::OnAttachments)
    EVT_BUTTON(ID_TRX_CUSTOMFIELDS, mmTransDialog::OnMoreFields)
    EVT_MENU_RANGE(wxID_LOWEST, wxID_LOWEST + 20, mmTransDialog::OnNoteSelected)
    EVT_MENU_RANGE(wxID_HIGHEST , wxID_HIGHEST + 8, mmTransDialog::OnColourSelected)
    EVT_BUTTON(wxID_INFO, mmTransDialog::OnColourButton)
    EVT_BUTTON(wxID_OK, mmTransDialog::OnOk)
    EVT_BUTTON(wxID_CANCEL, mmTransDialog::OnCancel)
    EVT_BUTTON(ID_TRX_CURRENCY, mmTransDialog::OnCurrency)
    EVT_CLOSE(mmTransDialog::OnQuit)
wxEND_EVENT_TABLE()

void mmTransDialog::SetEventHandlers()
{
    m_trx_payee_box->Connect(ID_TRX_PAYEE, wxEVT_COMMAND_TEXT_UPDATED
        , wxCommandEventHandler(mmTransDialog::OnAccountOrPayeeUpdated), nullptr, this);
    m_trx_amount->Connect(ID_TRX_TEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::OnTextEntered), nullptr, this);
    m_trx_to_amount->Connect(ID_TRX_TOTEXTAMOUNT, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::OnTextEntered), nullptr, this);
    m_trx_number->Connect(ID_TRX_TEXTNUMBER, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmTransDialog::OnTextEntered), nullptr, this);

#ifdef __WXGTK__ // Workaround for bug http://trac.wxwidgets.org/ticket/11630
    dpc_->Connect(ID_DIALOG_TRANS_BUTTONDATE, wxEVT_KILL_FOCUS
        , wxFocusEventHandler(mmTransDialog::OnDpcKillFocus), nullptr, this);
#endif
}

mmTransDialog::mmTransDialog(wxWindow* parent
    , int account_id
    , int transaction_id
    , bool duplicate
    , int type
) : m_is_transfer(false)
    , m_is_duplicate(duplicate)
    , m_is_categ_updated(false)
    , m_is_advanced(false)
    , m_trx_advanced(nullptr)
    , m_account_id(account_id)
    , m_trx_currency_btn(nullptr)
    , m_to_currency(nullptr)
    , skip_date_init_(false)
    , skip_account_init_(false)
    , skip_amount_init_(false)
    , skip_payee_init_(false)
    , skip_status_init_(false)
    , skip_notes_init_(false)
    , skip_category_init_(false)
    , skip_tooltips_init_(false)
{

    Model_Checking::Data *transaction = Model_Checking::instance().get(transaction_id);
    m_is_new_trx = (transaction || m_is_duplicate) ? false : true;
    m_is_transfer = m_is_new_trx ? type == Model_Checking::TRANSFER : Model_Checking::is_transfer(transaction);
    if (m_is_new_trx)
    {
        Model_Checking::getEmptyTransaction(m_trx_data, account_id);
        m_trx_data.TRANSCODE = Model_Checking::all_type()[type];
    }
    else
    {
        Model_Checking::getTransactionData(m_trx_data, transaction);
        const auto s = Model_Checking::splittransaction(transaction);
        for (const auto& item : s)
            m_local_splits.push_back({ item.CATEGID, item.SUBCATEGID, item.SPLITTRANSAMOUNT });
    }

    Model_Account::Data* acc = Model_Account::instance().get(m_trx_data.ACCOUNTID);
    if (acc && Model_Account::is_multicurrency(acc)) {
        m_currency = Model_Currency::instance().get(m_trx_data.CURRENCYID);
        if (!m_currency)
            m_currency = Model_Account::currency(acc);
    }
    else if (acc)
    {
        m_currency = Model_Account::currency(acc);
    }

    if (!acc || !m_currency)
    {
        m_currency = Model_Currency::GetBaseCurrency();
    }

    m_precision = m_currency->SCALE;

    if (m_is_transfer)
    {
        Model_Account::Data* to_acc = Model_Account::instance().get(m_trx_data.TOACCOUNTID);
        if (to_acc) {
            m_to_currency = Model_Account::currency(to_acc);
        }
        if (m_to_currency) {
            m_is_advanced = !m_is_new_trx
                && (Model_Account::get_account_type(m_account_id) != Model_Account::INVESTMENT
                     && Model_Account::get_account_type(to_acc->ACCOUNTID) != Model_Account::INVESTMENT)
                && (m_currency->CURRENCYID != m_to_currency->CURRENCYID
                    || m_trx_data.TRANSAMOUNT != m_trx_data.TOTRANSAMOUNT);
        }
    }

    int ref_id = m_trx_data.TRANSID;
    if (m_is_duplicate || m_is_new_trx) ref_id = -1;
    m_custom_fields = new mmCustomDataTransaction(this, ref_id, ID_TRX_CUSTOMFIELD);

    Create(parent);
    this->SetMinSize(wxSize(500, 400));

    dataToControls();
}

mmTransDialog::~mmTransDialog()
{
    m_currency->SCALE = m_precision;
}

bool mmTransDialog::Create(wxWindow* parent, wxWindowID id, const wxString& caption
    , const wxPoint& pos, const wxSize& size, long style, const wxString& name)
{
    SetExtraStyle(GetExtraStyle()|wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create(parent, id, caption, pos, size, style, name);

    SetEvtHandlerEnabled(false);
    CreateControls();

    SetIcon(mmex::getProgramIcon());
    m_is_duplicate ? SetDialogTitle(_("Duplicate Transaction")) : SetDialogTitle(m_is_new_trx ? _("New Transaction") : _("Edit Transaction"));

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
    Centre();
    Fit();

    SetEventHandlers();
    SetEvtHandlerEnabled(true);

    return TRUE;
}

void mmTransDialog::dataToControls()
{
    Model_Checking::getFrequentUsedNotes(frequentNotes_, m_trx_data.ACCOUNTID);
    wxButton* frequent_used_notes_btn = static_cast<wxButton*>(FindWindow(ID_TRX_FREQENTNOTES));
    frequent_used_notes_btn->Enable(!frequentNotes_.empty());
    
    m_trx_colours_btn->SetBackgroundColour(getUDColour(m_trx_data.COLOURID));

    m_trx_currency_btn->SetLabel(m_currency->CURRENCYNAME);

    if (!skip_date_init_) //Date
    {
        wxDateTime trx_date;
        trx_date.ParseDate(m_trx_data.TRANSDATE);
        m_date_picker->SetValue(trx_date);
        m_date_picker->SetFocus();

        trx_date.ParseTime(m_trx_data.TRANSTIME);
        m_time_picker->SetValue(trx_date);

        skip_date_init_ = true;
    }

    if (!skip_status_init_) //Status
    {
        m_status = m_trx_data.STATUS;
        m_trx_status->SetSelection(Model_Checking::status(m_status));
        skip_status_init_ = true;
    }

    //Type
    m_trx_type->SetSelection(Model_Checking::type(m_trx_data.TRANSCODE));

    //Advanced
    m_trx_advanced->Enable(m_is_transfer);
    m_trx_advanced->SetValue(m_is_advanced && m_is_transfer);
    m_trx_to_amount->Enable(m_is_advanced && m_is_transfer);

    if (!skip_amount_init_) //Amounts
    {
        if (m_is_transfer)
        {
            if (m_is_advanced)
            {
                m_trx_to_amount->SetValue(m_trx_data.TOTRANSAMOUNT, Model_Currency::precision(m_trx_data.TOACCOUNTID));
            }
        }
        else
            m_trx_to_amount->ChangeValue("");

        if (m_trx_data.TRANSID != -1)
        {
            m_trx_amount->SetValue(m_trx_data.TRANSAMOUNT, Model_Currency::precision(m_trx_data.ACCOUNTID));
        }
        skip_amount_init_ = true;
    }

    if (!skip_account_init_) //Account
    {
        m_trx_account->SetEvtHandlerEnabled(false);
        m_trx_account->Clear();
        const wxArrayString account_list = Model_Account::instance().all_account_names(true);
        m_trx_account->Append(account_list);
        m_trx_account->AutoComplete(account_list);

        bool acc_closed = false;
        const auto& accounts = Model_Account::instance().all();
        for (const auto &account : accounts)
        {
            if (account.ACCOUNTID == m_trx_data.ACCOUNTID)
            {
                m_trx_account->ChangeValue(account.ACCOUNTNAME);
                if (account.STATUS == Model_Account::all_status()[Model_Account::CLOSED])
                {
                    m_trx_account->Append(account.ACCOUNTNAME);
                    acc_closed = true;
                }
            }
        }

        if (account_list.size() == 1 && !acc_closed)
            m_trx_account->ChangeValue(account_list[0]);

        m_trx_account->Enable(!acc_closed && (account_list.size() > 1));

        m_trx_account->SetEvtHandlerEnabled(true);
        skip_account_init_ = true;
    }

    if (!skip_payee_init_) //Payee or To Account
    {
        m_trx_payee_box->SetEvtHandlerEnabled(false);

        m_trx_payee_box->Clear();
        m_trx_account->UnsetToolTip();
        m_trx_payee_box->UnsetToolTip();
        wxString payee_tooltip = "";
        if (!m_is_transfer)
        {
            if (!Model_Checking::is_deposit(m_trx_data.TRANSCODE))
            {
                m_payee_label->SetLabelText(_("Payee"));
            }
            else {
                m_payee_label->SetLabelText(_("From"));
            }

            m_account_label->SetLabelText(_("Account"));

            wxArrayString all_payees = Model_Payee::instance().all_payee_names();
            if (!all_payees.empty()) {
                m_trx_payee_box->Insert(all_payees, 0);
                m_trx_payee_box->AutoComplete(all_payees);
            }

            Model_Account::Data* account = Model_Account::instance().get(m_trx_account->GetValue());
            if (m_is_new_trx && !m_is_duplicate && Option::instance().TransPayeeSelection() == Option::LASTUSED)
            {
                Model_Checking::Data_Set transactions = Model_Checking::instance().find(
                    Model_Checking::TRANSCODE(Model_Checking::TRANSFER, NOT_EQUAL)
                    , Model_Checking::ACCOUNTID(account->ACCOUNTID, EQUAL)
                    , Model_Checking::TRANSDATE(wxDateTime::Today(), LESS_OR_EQUAL));

                if (!transactions.empty()) {
                    Model_Payee::Data* payee = Model_Payee::instance().get(transactions.back().PAYEEID);
                    m_trx_payee_box->ChangeValue(payee->PAYEENAME);
                }
            }

            if (m_is_new_trx && (Option::instance().TransPayeeSelection() == Option::UNUSED))
            {
                m_trx_payee_box->ChangeValue(_("Unknown"));
            }

            Model_Payee::Data* payee = Model_Payee::instance().get(m_trx_data.PAYEEID);
            if (payee)
            {
                m_trx_payee_box->ChangeValue(payee->PAYEENAME);

                if (Model_Account::type(account) == Model_Account::INVESTMENT)
                {
                    m_currency->SCALE = static_cast<int>(pow(10, 4));
                    Model_Ticker::Data_Set t = Model_Ticker::instance().find(Model_Ticker::SYMBOL(payee->PAYEENAME));
                    //TODO:
                }

            }

            mmTransDialog::SetCategoryForPayee();
        }
        else //transfer
        {
            m_trx_payee_box->Enable(true);
            if (m_split_box->IsChecked())
            {
                m_split_box->SetValue(false);
                m_local_splits.clear();
            }

            if (m_is_new_trx && !m_is_duplicate)
            {
                const auto &categs = Model_Category::instance().find(Model_Category::CATEGNAME(_("Transfer")));
                if (!categs.empty())
                {
                    m_trx_data.SUBCATEGID = -1;
                    m_trx_data.CATEGID = categs.begin()->CATEGID;
                    m_trx_curegory_btn->SetLabelText(Model_Category::full_name(m_trx_data.CATEGID, -1));
                }
            }

            wxArrayString account_names = Model_Account::instance().all_account_names(true);
            m_trx_payee_box->Insert(account_names, 0);
            Model_Account::Data *account = Model_Account::instance().get(m_trx_data.TOACCOUNTID);
            if (account)
                m_trx_payee_box->ChangeValue(account->ACCOUNTNAME);

            m_trx_payee_box->AutoComplete(account_names);

            m_payee_label->SetLabelText(_("To"));
            m_trx_data.PAYEEID = -1;
            m_account_label->SetLabelText(_("From"));
        }
        skip_payee_init_ = true;
        m_trx_payee_box->SetEvtHandlerEnabled(true);
    }

    if (!skip_category_init_)
    {
        bool has_split = !m_local_splits.empty();
        wxString fullCategoryName;
        m_trx_curegory_btn->UnsetToolTip();
        if (has_split)
        {
            fullCategoryName = _("Categories");
            m_trx_amount->SetValue(Model_Splittransaction::get_total(m_local_splits));
            m_trx_data.CATEGID = -1;
            m_trx_data.SUBCATEGID = -1;
        }
        else
        {
            Model_Category::Data *category = Model_Category::instance().get(m_trx_data.CATEGID);
            Model_Subcategory::Data *subcategory = (Model_Subcategory::instance().get(m_trx_data.SUBCATEGID));
            fullCategoryName = Model_Category::full_name(category, subcategory);
            if (fullCategoryName.IsEmpty())
            {
                fullCategoryName = _("Select Category");
            }
        }

        m_trx_curegory_btn->SetLabelText(fullCategoryName);
        m_split_box->SetValue(has_split);
        skip_category_init_ = true;
    }
    m_trx_amount->Enable(m_local_splits.empty());
    m_split_box->Enable(!m_is_transfer);

    if (!skip_notes_init_) //Notes & Transaction Number
    {
        m_trx_number->SetValue(m_trx_data.TRANSACTIONNUMBER);
        m_trx_notes->SetValue(m_trx_data.NOTES);
        skip_notes_init_ = true;
    }

    if (!skip_tooltips_init_)
        SetTooltips();
}

void mmTransDialog::CreateControls()
{
    wxBoxSizer* box_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* box_sizer2 = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* box_sizer3 = new wxBoxSizer(wxVERTICAL);
    box_sizer->Add(box_sizer1, g_flagsExpand);
    box_sizer1->Add(box_sizer2, g_flagsExpand);
    box_sizer1->Add(box_sizer3, g_flagsExpand);

    wxStaticBox* static_box = new wxStaticBox(this, wxID_ANY, _("Transaction Details"));
    wxStaticBoxSizer* box_sizer_left = new wxStaticBoxSizer(static_box, wxVERTICAL);
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    box_sizer_left->Add(flex_sizer, g_flagsV);
    box_sizer2->Add(box_sizer_left, g_flagsExpand);

    // Date --------------------------------------------
    long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;

    m_date_picker = new wxDatePickerCtrl(this, ID_TRX_DATE
        , wxDateTime::Today(), wxDefaultPosition, wxDefaultSize, date_style);

    m_time_picker = new wxTimePickerCtrl(this, ID_TRX_TIME
        , wxDateTime::Now());

    wxStaticText* name_label = new wxStaticText(this, wxID_STATIC, _("Date/Time"));
    flex_sizer->Add(name_label, g_flagsH);
    name_label->SetFont(this->GetFont().Bold());
    wxBoxSizer* date_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(date_sizer);
    date_sizer->Add(m_date_picker, g_flagsH);
    date_sizer->Add(m_time_picker, g_flagsH);

    // Status --------------------------------------------
    m_trx_status = new wxChoice(this, ID_TRX_STATUS);

    for (const auto& i : Model_Checking::all_status()) {
        m_trx_status->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Status")), g_flagsH);
    flex_sizer->Add(m_trx_status, g_flagsH);

    // Type --------------------------------------------
    m_trx_type = new wxChoice(this, ID_TRX_TYPE);

    for (const auto& i : Model_Checking::all_type())
    {
        if (i != Model_Checking::TRANSFER_STR || Model_Account::instance().all().size() > 1)
        {
            m_trx_type->Append(wxGetTranslation(i), new wxStringClientData(i));
        }
    }

    m_trx_advanced = new wxCheckBox(this
        , ID_TRX_ADVANCED, _("&Advanced")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Type")), g_flagsH);
    flex_sizer->Add(typeSizer);
    typeSizer->Add(m_trx_type, g_flagsH);
    typeSizer->Add(m_trx_advanced, g_flagsH);

    // Amount Fields --------------------------------------------
    m_trx_amount = new mmTextCtrl(this, ID_TRX_TEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());

    m_trx_to_amount = new mmTextCtrl( this, ID_TRX_TOTEXTAMOUNT, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());

    wxBoxSizer* amountSizer = new wxBoxSizer(wxHORIZONTAL);
    amountSizer->Add(m_trx_amount, g_flagsExpand);
    amountSizer->Add(m_trx_to_amount, g_flagsExpand);

    wxStaticText* amount_label = new wxStaticText(this, wxID_STATIC, _("Amount"));
    amount_label->SetFont(this->GetFont().Bold());
    flex_sizer->Add(amount_label, g_flagsH);
    flex_sizer->Add(amountSizer);

    // Currency ---------------------------------------------

    wxStaticText* currency_label = new wxStaticText(this, wxID_STATIC, _("Currency"));
    Model_Account::Data* account = Model_Account::instance().get(m_account_id);

    m_trx_currency_btn = new wxButton(this, ID_TRX_CURRENCY, m_currency->CURRENCYNAME);
    bool multicurrency = account && Model_Account::is_multicurrency(account);
    if (!multicurrency) {
        m_trx_currency_btn->Hide();
        currency_label->Hide();
    }
    else {
        m_trx_currency_btn->SetToolTip(_("Specify the currency to be used by this account."));
    }

    flex_sizer->Add(currency_label, g_flagsH);
    flex_sizer->Add(m_trx_currency_btn, g_flagsExpand);

    // Account ---------------------------------------------
    m_trx_account = new wxComboBox(this, wxID_ANY);

    m_account_label = new wxStaticText(this, wxID_STATIC, _("Account"));
    m_account_label->SetFont(this->GetFont().Bold());
    flex_sizer->Add(m_account_label, g_flagsH);
    flex_sizer->Add(m_trx_account, g_flagsExpand);

    // Payee ---------------------------------------------
    m_payee_label = new wxStaticText(this, wxID_STATIC, _("Payee"));
    m_payee_label->SetFont(this->GetFont().Bold());

    /*Note: If you want to use EVT_TEXT_ENTER(id,func) to receive wxEVT_COMMAND_TEXT_ENTER events,
      you have to add the wxTE_PROCESS_ENTER window style flag.
      If you create a wxComboBox with the flag wxTE_PROCESS_ENTER, the tab key won't jump to the next control anymore.*/
    m_trx_payee_box = new wxComboBox(this, ID_TRX_PAYEE);

    flex_sizer->Add(m_payee_label, g_flagsH);
    flex_sizer->Add(m_trx_payee_box, g_flagsExpand);

    // Categories -------------------------------------------
    wxStaticText* categ_label = new wxStaticText(this, wxID_STATIC, _("Category"));
    categ_label->SetFont(this->GetFont().Bold());
    flex_sizer->Add(categ_label, g_flagsH);

    wxBoxSizer* categ_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(categ_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));

    m_split_box = new wxCheckBox(this, wxID_FORWARD, _("Split")
        , wxDefaultPosition, wxDefaultSize, wxCHK_2STATE );
    m_split_box->SetValue(FALSE);

    m_trx_curegory_btn = new wxButton(this, wxID_VIEW_DETAILS);

    categ_sizer->Add(m_trx_curegory_btn, g_flagsExpand);
    categ_sizer->Add(m_split_box, g_flagsH);

    // Number  ---------------------------------------------
    m_trx_number = new mmTextCtrl(this, ID_TRX_TEXTNUMBER, ""
        , wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);

    wxBitmapButton* bAuto = new wxBitmapButton(this, ID_TRX_TRANSNUM, mmBitmap(png::TRXNUM));
    bAuto->Connect(ID_TRX_TRANSNUM
        , wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmTransDialog::OnAutoTransNum), nullptr, this);
    bAuto->SetToolTip(_("Populate Transaction #"));

    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Number")), g_flagsH);
    wxBoxSizer* number_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(number_sizer, wxSizerFlags(g_flagsExpand).Border(wxALL, 0));
    number_sizer->Add(m_trx_number, g_flagsExpand);
    number_sizer->Add(bAuto, g_flagsH);

    // Attachments ---------------------------------------------
    m_trx_attachments_btn = new wxBitmapButton(this, wxID_FILE, mmBitmap(png::CLIP)
        , wxDefaultPosition, wxSize(m_trx_payee_box->GetSize().GetY(), m_trx_payee_box->GetSize().GetY()));
    m_trx_attachments_btn->SetToolTip(_("Organize attachments of this transaction"));

    // Colours ---------------------------------------------
    m_trx_colours_btn = new wxButton(this, wxID_INFO, " ", wxDefaultPosition, m_trx_attachments_btn->GetSize(), 0);
    //bColours->SetBackgroundColour(mmColors::userDefColor1);
    m_trx_colours_btn->SetToolTip(_("User Colors"));


    // Notes ---------------------------------------------
    flex_sizer->Add(new wxStaticText(this, wxID_STATIC, _("Notes")), g_flagsH);
    wxButton* bFrequentUsedNotes = new wxButton(this
        , ID_TRX_FREQENTNOTES, "...", wxDefaultPosition, m_trx_attachments_btn->GetSize(), 0);
    bFrequentUsedNotes->SetToolTip(_("Select one of the frequently used notes"));
    bFrequentUsedNotes->Connect(ID_TRX_FREQENTNOTES
        , wxEVT_COMMAND_BUTTON_CLICKED
        , wxCommandEventHandler(mmTransDialog::OnFrequentUsedNotes), nullptr, this);


    wxBoxSizer* RightAlign_sizer = new wxBoxSizer(wxHORIZONTAL);
    flex_sizer->Add(RightAlign_sizer, wxSizerFlags(g_flagsH).Align(wxALIGN_RIGHT));
    RightAlign_sizer->Add(m_trx_attachments_btn, wxSizerFlags().Border(wxRIGHT, 5));
    RightAlign_sizer->Add(m_trx_colours_btn, wxSizerFlags().Border(wxRIGHT, 5));
    RightAlign_sizer->Add(bFrequentUsedNotes, wxSizerFlags().Border(wxRIGHT, 5));

    m_trx_notes = new wxTextCtrl(this, ID_TRX_TEXTNOTES, ""
        , wxDefaultPosition, wxSize(-1, m_date_picker->GetSize().GetHeight() * 5), wxTE_MULTILINE);
    box_sizer_left->Add(m_trx_notes, wxSizerFlags(g_flagsExpand).Border(wxLEFT | wxRIGHT | wxBOTTOM, 10));

    /**********************************************************************************************
     Button Panel with OK and Cancel Buttons
    ***********************************************************************************************/
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    box_sizer_left->Add(buttons_panel, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));

    wxStdDialogButtonSizer*  buttons_sizer = new wxStdDialogButtonSizer;
    buttons_panel->SetSizer(buttons_sizer);

    wxButton* itemButtonOK = new wxButton(buttons_panel, wxID_OK, _("&OK "));
    m_button_cancel = new wxButton(buttons_panel, wxID_CANCEL, wxGetTranslation(g_CancelLabel));

    wxBitmapButton* itemButtonHide = new wxBitmapButton(buttons_panel, ID_TRX_CUSTOMFIELDS, mmBitmap(png::RIGHTARROWSIMPLE));
    itemButtonHide->SetToolTip(_("Show/Hide custom fields window"));
    if (m_custom_fields->GetCustomFieldsCount() == 0) {
        itemButtonHide->Hide();
    }

    buttons_sizer->Add(itemButtonOK, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(m_button_cancel, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer->Add(itemButtonHide, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    if (!m_is_new_trx && !m_is_duplicate) m_button_cancel->SetFocus();

    buttons_sizer->Realize();

    // Custom fields -----------------------------------

    m_custom_fields->FillCustomFields(box_sizer3);
    if (m_custom_fields->GetActiveCustomFieldsCount() > 0) {
        wxCommandEvent evt(wxEVT_BUTTON, ID_TRX_CUSTOMFIELDS);
        this->GetEventHandler()->AddPendingEvent(evt);
    }

    Center();
    this->SetSizer(box_sizer);
}

bool mmTransDialog::doValidateData()
{
    Model_Account::Data* account = Model_Account::instance().get(m_trx_account->GetValue());
    if (!account) {
        mmErrorDialogs::InvalidAccount(m_trx_account);
        return false;
    }
    m_trx_data.ACCOUNTID = account->ACCOUNTID;

    /* Check if transaction is to proceed.*/
    if (Model_Account::is_positive(account->STATEMENTLOCKED))
    {
        if (m_date_picker->GetValue() <= Model_Account::get_date_by_string(account->STATEMENTDATE))
        {
            if (wxMessageBox(_(wxString::Format(
                "Locked transaction to date: %s\n\n"
                "Do you wish to continue ? "
                , mmGetDateForDisplay(account->STATEMENTDATE)))
                , _("MMEX Transaction Check"), wxYES_NO | wxICON_WARNING) == wxNO)
            {
                return false;
            }
        }
    }

    if (!m_trx_amount->checkValue(m_trx_data.TRANSAMOUNT))
        return false;

    if (!m_is_transfer)
    {
        wxString payee_name = m_trx_payee_box->GetValue();
        if (payee_name.IsEmpty())
        {
            mmErrorDialogs::InvalidPayee(m_trx_payee_box);
            return false;
        }

        // Get payee string from populated list to address issues with case compare differences between autocomplete and payee list
        int payee_loc = m_trx_payee_box->FindString(payee_name);
        if (payee_loc != wxNOT_FOUND)
            payee_name = m_trx_payee_box->GetString(payee_loc);

        Model_Payee::Data* payee = Model_Payee::instance().get(payee_name);
        if (!payee)
        {
            wxMessageDialog msgDlg( this
                , wxString::Format(_("Do you want to add new payee: \n%s?"), payee_name)
                , _("Confirm to add new payee")
                , wxYES_NO | wxYES_DEFAULT | wxICON_WARNING);
            if (Option::instance().TransCategorySelection() == Option::UNUSED || msgDlg.ShowModal() == wxID_YES)
            {
                payee = Model_Payee::instance().create();
                payee->PAYEENAME = payee_name;
                Model_Payee::instance().save(payee);
                mmWebApp::MMEX_WebApp_UpdatePayee();
            }
            else
                return false;
        }
        m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
        m_trx_data.PAYEEID = payee->PAYEEID;

        payee->CATEGID = m_trx_data.CATEGID;
        payee->SUBCATEGID = m_trx_data.SUBCATEGID;
        Model_Payee::instance().save(payee);
        mmWebApp::MMEX_WebApp_UpdatePayee();
    }
    else //transfer
    {
        Model_Account::Data *to_account = Model_Account::instance().get(m_trx_payee_box->GetValue());
        if (!to_account || to_account->ACCOUNTID == m_trx_data.ACCOUNTID)
        {
            mmErrorDialogs::InvalidAccount(m_trx_payee_box, true);
            return false;
        }
        m_trx_data.TOACCOUNTID = to_account->ACCOUNTID;

        if (m_is_advanced)
        {
            if (!m_trx_to_amount->checkValue(m_trx_data.TOTRANSAMOUNT))
                return false;
        }
        else {
            m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
        }
        m_trx_data.PAYEEID = -1;
    }

    if ((m_split_box->IsChecked() && m_local_splits.empty())
        || (!m_split_box->IsChecked() && Model_Category::full_name(m_trx_data.CATEGID, m_trx_data.SUBCATEGID).empty()))
    {
        mmErrorDialogs::InvalidCategory(m_trx_curegory_btn, false);
        return false;
    }

    if (!m_currency) {
        mmErrorDialogs::ToolTip4Object(m_trx_currency_btn,
            _("Invalid value"), _("Error"));
        return false;
    }

    m_trx_data.CURRENCYID = m_currency->CURRENCYID;

    //Checking account does not exceed limits
    {
        bool abort_transaction = Model_Account::instance().is_limit_reached(&m_trx_data);

        
        if (abort_transaction && wxMessageBox(_(
            "This transaction will exceed your account limit.\n\n"
            "Do you wish to continue?")
            , _("MMEX Transaction Check"), wxYES_NO | wxICON_WARNING) == wxNO)
        {
            return false;
        }
    }

    return true;
}

//----------------------------------------------------------------------------
// Workaround for bug http://trac.wxwidgets.org/ticket/11630
void mmTransDialog::OnDpcKillFocus(wxFocusEvent& event)
{
    if (wxGetKeyState(WXK_TAB) && wxGetKeyState(WXK_SHIFT))
        m_button_cancel->SetFocus();
    else if (wxGetKeyState(WXK_TAB))
        m_trx_status->SetFocus();
    else if (wxGetKeyState(WXK_UP))
    {
        wxCommandEvent evt(wxEVT_SPIN, wxID_ANY);
        evt.SetInt(1);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else if (wxGetKeyState(WXK_DOWN))
    {
        wxCommandEvent evt(wxEVT_SPIN, wxID_ANY);
        evt.SetInt(-1);
        this->GetEventHandler()->AddPendingEvent(evt);
    }
    else
        event.Skip();
}

void mmTransDialog::OnFocusChange(wxChildFocusEvent& event)
{
    wxWindow *w = event.GetWindow();
    if (w)
    {
        m_object_in_focus_id = w->GetId();
    }

    wxString accountName = m_trx_account->GetValue();
    wxString toAccountName = m_trx_payee_box->GetValue();
    for (const auto& acc : Model_Account::instance().all_account_names())
    {
        if (acc.CmpNoCase(accountName) == 0) accountName = acc;
        if (acc.CmpNoCase(toAccountName) == 0) toAccountName = acc;
    }

    Model_Account::Data* account = Model_Account::instance().get(accountName);
    if (account && !Model_Account::is_multicurrency(account))
    {
        m_currency = Model_Account::currency(account);
        m_trx_account->SetValue(account->ACCOUNTNAME);
    }

    if (!m_is_transfer)
    {
        Model_Payee::Data* payee = Model_Payee::instance().get(m_trx_payee_box->GetValue());
        if (payee)
        {
            m_trx_payee_box->ChangeValue(payee->PAYEENAME);
            SetCategoryForPayee(payee);
        }
        m_trx_to_amount->ChangeValue("");
    }
    else
    {
        const Model_Account::Data* to_account = Model_Account::instance().get(toAccountName);
        if (to_account)
        {
            m_to_currency = Model_Account::currency(to_account);
            m_trx_payee_box->ChangeValue(to_account->ACCOUNTNAME);
            m_trx_data.TOACCOUNTID = to_account->ACCOUNTID;
        }
    }

    if (m_object_in_focus_id == m_trx_amount->GetId())
    {
        m_trx_amount->SelectAll();
    }
    else
    {
        if (m_trx_amount->Calculate(Model_Currency::precision(m_trx_data.ACCOUNTID)))
        {
            m_trx_amount->GetDouble(m_trx_data.TRANSAMOUNT);
        }
        skip_amount_init_ = false;
    }

    if (m_is_advanced && m_object_in_focus_id == m_trx_to_amount->GetId())
    {
        m_trx_to_amount->SelectAll();
    }

    if (m_is_transfer)
    {
        if (m_trx_to_amount->Calculate(Model_Currency::precision(m_trx_data.TOACCOUNTID))) {
            m_trx_to_amount->GetDouble(m_trx_data.TOTRANSAMOUNT);
        }
        if (!m_is_advanced) {
            m_trx_to_amount->ChangeValue("");
        }
    }


    dataToControls();
    event.Skip();
}

void mmTransDialog::ActivateSplitTransactionsDlg()
{
    bool bDeposit = Model_Checking::is_deposit(m_trx_data.TRANSCODE);

    if (!m_trx_amount->GetDouble(m_trx_data.TRANSAMOUNT))
        m_trx_data.TRANSAMOUNT = 0;

    const auto full_category_name = Model_Category::full_name(m_trx_data.CATEGID, m_trx_data.SUBCATEGID);
    if (!full_category_name.empty() && m_local_splits.empty() && m_trx_data.TRANSAMOUNT != 0)
    {
        Split s;
        s.SPLITTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
        s.CATEGID = m_trx_data.CATEGID;
        s.SUBCATEGID = m_trx_data.SUBCATEGID;
        m_local_splits.push_back(s);
    }

    SplitTransactionDialog dlg(this, m_local_splits
        , bDeposit ? Model_Checking::DEPOSIT : Model_Checking::WITHDRAWAL
        , m_trx_data.ACCOUNTID
        , m_trx_data.TRANSAMOUNT);

    if (dlg.ShowModal() == wxID_OK)
    {
        m_local_splits = dlg.getResult();
    }
    if (!m_local_splits.empty())
    {
        m_trx_data.TRANSAMOUNT = Model_Splittransaction::get_total(m_local_splits);
        skip_category_init_ = !dlg.isItemsChanged();
    }
}

void mmTransDialog::SetDialogTitle(const wxString& title)
{
    this->SetTitle(title);
}

//** --------------=Event handlers=------------------ **//

void mmTransDialog::OnTransTypeChanged(wxCommandEvent& event)
{
    const wxString old_type = m_trx_data.TRANSCODE;
    wxStringClientData *client_obj = static_cast<wxStringClientData*>(event.GetClientObject());
    if (client_obj) m_trx_data.TRANSCODE = client_obj->GetData();
    if (old_type != m_trx_data.TRANSCODE)
    {
        skip_payee_init_ = false;
        m_is_transfer = Model_Checking::is_transfer(m_trx_data.TRANSCODE);
        if (m_is_transfer) {
            m_trx_data.PAYEEID = -1;
        } else {
            m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
            m_trx_data.TOACCOUNTID = -1;
        }
        skip_account_init_ = false;
        skip_tooltips_init_ = false;
        dataToControls();
    }
}

#if defined (__WXMAC__)
void mmTransDialog::OnAccountOrPayeeUpdated(wxCommandEvent& event)
{
    // Filtering the combobox as the user types because on Mac autocomplete function doesn't work
    // PLEASE DO NOT REMOVE!!!
    if (!m_transfer)
    {
        wxString payeeName = event.GetString();
        if (m_trx_payee_box->GetSelection() == -1) // make sure nothing is selected (ex. user presses down arrow)
        {
            m_trx_payee_box->SetEvtHandlerEnabled(false); // things will crash if events are handled during Clear
            m_trx_payee_box->Clear();
            Model_Payee::Data_Set filtd = Model_Payee::instance().FilterPayees(payeeName);
            std::sort(filtd.rbegin(), filtd.rend(), SorterByPAYEENAME());
            for (const auto &payee : filtd) {
                m_trx_payee_box->Insert(payee.PAYEENAME, 0);
            }
            m_trx_payee_box->ChangeValue(payeeName);
            m_trx_payee_box->SetInsertionPointEnd();
            m_trx_payee_box->SetEvtHandlerEnabled(true);
        }
    }
#else
void mmTransDialog::OnAccountOrPayeeUpdated(wxCommandEvent& WXUNUSED(event))
{
#endif
    wxChildFocusEvent evt;
    OnFocusChange(evt);
}

void mmTransDialog::SetCategoryForPayee(const Model_Payee::Data *payee)
{
    // Only for new transactions: if user do not want to use categories.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (Option::instance().TransCategorySelection() == Option::UNUSED
        && !m_is_categ_updated && m_local_splits.empty() && m_is_new_trx && !m_is_duplicate)
    {
        Model_Category::Data *category = Model_Category::instance().get(_("Unknown"));
        if (!category)
        {
            category = Model_Category::instance().create();
            category->CATEGNAME = _("Unknown");
            Model_Category::instance().save(category);
        }

        m_trx_data.CATEGID = category->CATEGID;
        m_trx_curegory_btn->SetLabelText(_("Unknown"));
        return;
    }

    if (!payee)
    {
        payee = Model_Payee::instance().get(m_trx_payee_box->GetValue());
        if (!payee)
            return;
    }

    // Only for new transactions: if user want to autofill last category used for payee.
    // If this is a Split Transaction, ignore displaying last category for payee
    if (Option::instance().TransCategorySelection() == Option::LASTUSED
        && !m_is_categ_updated && m_local_splits.empty() && m_is_new_trx && !m_is_duplicate)
    {
        // if payee has memory of last category used then display last category for payee
        Model_Category::Data *category = Model_Category::instance().get(payee->CATEGID);
        if (category)
        {
            Model_Subcategory::Data *subcategory = (payee->SUBCATEGID != -1 ? Model_Subcategory::instance().get(payee->SUBCATEGID) : 0);
            wxString fullCategoryName = Model_Category::full_name(category, subcategory);

            m_trx_data.CATEGID = payee->CATEGID;
            m_trx_data.SUBCATEGID = payee->SUBCATEGID;
            m_trx_curegory_btn->SetLabelText(fullCategoryName);
            wxLogDebug("Category: %s = %.2f", m_trx_curegory_btn->GetLabel(), m_trx_data.TRANSAMOUNT);
        }
        else
        {
            m_trx_curegory_btn->SetLabelText(_("Select Category"));
            m_trx_data.CATEGID = -1;
            m_trx_data.SUBCATEGID = -1;
        }
    }
}

void mmTransDialog::OnSplitChecked(wxCommandEvent& WXUNUSED(event))
{
    if (m_split_box->IsChecked())
    {
        if (!m_trx_amount->IsEmpty() && !m_trx_amount->checkValue(m_trx_data.TRANSAMOUNT)) {
            m_split_box->SetValue(false);
            return;
        }
        ActivateSplitTransactionsDlg();
    }
    else
    {
        if (m_local_splits.size() > 1)
        {
            //Delete split items first (data protection)
            m_split_box->SetValue(true);
        }
        else
        {
            if (m_local_splits.size() == 1)
            {
                m_trx_data.CATEGID = m_local_splits.begin()->CATEGID;
                m_trx_data.SUBCATEGID = m_local_splits.begin()->SUBCATEGID;
                m_trx_data.TRANSAMOUNT = m_local_splits.begin()->SPLITTRANSAMOUNT;

                if (m_trx_data.TRANSAMOUNT < 0)
                {
                    m_trx_data.TRANSAMOUNT = -m_trx_data.TRANSAMOUNT;
                    m_trx_type->SetSelection(Model_Checking::WITHDRAWAL);
                }
            }
            else
            {
                m_trx_data.TRANSAMOUNT = 0;
            }
            m_local_splits.clear();
        }
    }
    skip_category_init_ = false;
    skip_tooltips_init_ = false;
    dataToControls();
}

void mmTransDialog::OnAutoTransNum(wxCommandEvent& WXUNUSED(event))
{
    auto d = Model_Checking::TRANSDATE(m_trx_data).Subtract(wxDateSpan::Days(300));
    double next_number = 0, temp_num;
    const auto numbers = Model_Checking::instance().find(Model_Checking::ACCOUNTID(m_trx_data.ACCOUNTID, EQUAL)
        , Model_Checking::TRANSDATE(d, GREATER_OR_EQUAL));
    for (const auto &num : numbers)
    {
        if (num.TRANSACTIONNUMBER.empty() || !num.TRANSACTIONNUMBER.IsNumber()) continue;
        if (num.TRANSACTIONNUMBER.ToDouble(&temp_num) && temp_num > next_number)
            next_number = temp_num;
    }

    next_number++;
    m_trx_number->SetValue(wxString::FromDouble(next_number, 0));
}

void mmTransDialog::OnAdvanceChecked(wxCommandEvent& event)
{
    int id = event.GetSelection();

    if (id == wxCHK_CHECKED)
    {
        double exch = 1.0;
        Model_Account::Data* to_acc = Model_Account::instance().get(m_trx_data.TOACCOUNTID);
        if (m_to_currency && to_acc
            && to_acc->ACCOUNTTYPE != Model_Account::all_type()[Model_Account::INVESTMENT])
        {
            const double convRateTo = Model_CurrencyHistory::getDayRate(m_to_currency->CURRENCYID, m_trx_data.TRANSDATE);
            if (convRateTo > 0.0) {
                const double convRate = Model_CurrencyHistory::getDayRate(m_currency->CURRENCYID, m_trx_data.TRANSDATE);
                exch = convRate / convRateTo;
            }
        }
        m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT * exch;
        m_trx_to_amount->SetValue(m_trx_data.TOTRANSAMOUNT, Model_Currency::precision(m_trx_data.TOACCOUNTID));
    }
    else if (id == wxCHK_UNCHECKED)
    {
        m_trx_data.TOTRANSAMOUNT = m_trx_data.TRANSAMOUNT;
        m_trx_to_amount->ChangeValue("");
    }

    m_is_advanced = m_trx_advanced->IsChecked();
    skip_amount_init_ = false;
    dataToControls();
}

void mmTransDialog::OnCategs(wxCommandEvent& WXUNUSED(event))
{
    if (m_split_box->IsChecked())
    {
        ActivateSplitTransactionsDlg();
    }
    else
    {
        mmCategDialog dlg(this, true, m_trx_data.CATEGID, m_trx_data.SUBCATEGID);
        if (dlg.ShowModal() == wxID_OK)
        {
            m_trx_data.CATEGID = dlg.getCategId();
            m_trx_data.SUBCATEGID = dlg.getSubCategId();
            m_trx_curegory_btn->SetLabelText(dlg.getFullCategName());
            m_is_categ_updated = true;
        }
    }
    skip_amount_init_ = false;
    skip_tooltips_init_ = false;
    dataToControls();
}

void mmTransDialog::OnAttachments(wxCommandEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    int TransID = m_trx_data.TRANSID;
    if (m_is_duplicate) TransID = -1;
    mmAttachmentDialog dlg(this, RefType, TransID);
    dlg.ShowModal();
}

void mmTransDialog::OnTextEntered(wxCommandEvent& WXUNUSED(event))
{
    if (m_object_in_focus_id == m_trx_amount->GetId())
    {
        if (m_trx_amount->Calculate(Model_Currency::precision(m_currency)))
        {
            m_trx_amount->GetDouble(m_trx_data.TRANSAMOUNT);
        }
        skip_amount_init_ = false;
        dataToControls();
    }
    else if (m_object_in_focus_id == m_trx_to_amount->GetId())
    {
        if (m_trx_to_amount->Calculate(Model_Currency::precision(m_trx_data.TOACCOUNTID)))
        {
            m_trx_to_amount->GetDouble(m_trx_data.TOTRANSAMOUNT);
        }
    }
    else if (m_object_in_focus_id == m_trx_number->GetId())
    {
        m_trx_amount->SetFocus();
    }
}

void mmTransDialog::OnFrequentUsedNotes(wxCommandEvent& WXUNUSED(event))
{
    wxMenu menu;
    int id = wxID_LOWEST;
    for (const auto& entry : frequentNotes_)
    {
        const wxString& label = entry.Mid(0, 30) + (entry.size() > 30 ? "..." : "");
        menu.Append(++id, label);
    }

    if (!frequentNotes_.empty())
        PopupMenu(&menu);
}

void mmTransDialog::OnNoteSelected(wxCommandEvent& event)
{
    int i = event.GetId() - wxID_LOWEST;
    if (i > 0 && static_cast<size_t>(i) <= frequentNotes_.size())
        m_trx_notes->ChangeValue(frequentNotes_[i - 1]);
}

void mmTransDialog::OnOk(wxCommandEvent& WXUNUSED(event))
{
    m_trx_data.STATUS = "";
    m_trx_data.NOTES = m_trx_notes->GetValue();
    m_trx_data.TRANSACTIONNUMBER = m_trx_number->GetValue();
    m_trx_data.TRANSDATE = m_date_picker->GetValue().FormatISODate();
    m_trx_data.TRANSTIME = m_time_picker->GetValue().FormatISOTime();
    wxStringClientData* status_obj = static_cast<wxStringClientData*>(
        m_trx_status->GetClientObject(m_trx_status->GetSelection())
    );
    if (status_obj) {
        m_status = Model_Checking::toShortStatus(status_obj->GetData());
        m_trx_data.STATUS = m_status;
    }

    if (!doValidateData()) return;
    if(!m_custom_fields->doValidateCustomValues(m_trx_data.TRANSID)) return;

    Model_Checking::Data *r = (m_is_new_trx || m_is_duplicate)
        ? Model_Checking::instance().create()
        : Model_Checking::instance().get(m_trx_data.TRANSID);

    Model_Checking::putDataToTransaction(r, m_trx_data);
    m_trx_data.TRANSID = Model_Checking::instance().save(r);

    Model_Splittransaction::Data_Set splt;
    for (const auto& entry : m_local_splits)
    {
        Model_Splittransaction::Data *s = Model_Splittransaction::instance().create();
        s->CATEGID = entry.CATEGID;
        s->SUBCATEGID = entry.SUBCATEGID;
        s->SPLITTRANSAMOUNT = entry.SPLITTRANSAMOUNT;
        splt.push_back(*s);
    }
    Model_Splittransaction::instance().update(splt, m_trx_data.TRANSID);

    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    if (m_is_new_trx || m_is_duplicate)
    {
        mmAttachmentManage::RelocateAllAttachments(RefType, -1, m_trx_data.TRANSID);
    }

    m_custom_fields->SaveCustomValues(m_trx_data.TRANSID);

    const Model_Checking::Data& tran(*r);
    Model_Checking::Full_Data trx(tran);
    wxLogDebug("%s", trx.to_json());
    EndModal(wxID_OK);
}

void mmTransDialog::OnCancel(wxCommandEvent& WXUNUSED(event))
{
#ifndef __WXMAC__
    if (m_object_in_focus_id != m_button_cancel->GetId() && wxGetKeyState(WXK_ESCAPE))
            return m_button_cancel->SetFocus();
#endif

    if (m_is_new_trx)
    {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
        mmAttachmentManage::DeleteAllAttachments(RefType, m_trx_data.TRANSID);
        Model_CustomFieldData::instance().DeleteAllData(RefType, m_trx_data.TRANSID);
    }
    EndModal(wxID_CANCEL);
}

void mmTransDialog::SetTooltips()
{
    m_trx_curegory_btn->UnsetToolTip();
    skip_tooltips_init_ = true;
    if (this->m_local_splits.empty())
        m_trx_curegory_btn->SetToolTip(_("Specify the category for this transaction"));
    else {
        const Model_Currency::Data* currency = Model_Currency::GetBaseCurrency();
        const Model_Account::Data* account = Model_Account::instance().get(m_trx_data.ACCOUNTID);
        if (account)
            currency = Model_Account::currency(account);

        m_trx_curegory_btn->SetToolTip(Model_Splittransaction::get_tooltip(m_local_splits, currency));
    }
    if (!m_is_new_trx) return;

    m_trx_amount->UnsetToolTip();
    m_trx_to_amount->UnsetToolTip();
    m_trx_account->UnsetToolTip();
    m_trx_payee_box->UnsetToolTip();

    if (m_is_transfer)
    {
        m_trx_account->SetToolTip(_("Specify account the money is taken from"));
        m_trx_payee_box->SetToolTip(_("Specify account the money is moved to"));
        m_trx_amount->SetToolTip(_("Specify the transfer amount in the From Account."));

        if (m_is_advanced)
            m_trx_to_amount->SetToolTip(_("Specify the transfer amount in the To Account"));
    }
    else
    {
        m_trx_amount->SetToolTip(_("Specify the amount for this transaction"));
        m_trx_account->SetToolTip(_("Specify account for the transaction"));
        if (!Model_Checking::is_deposit(m_trx_data.TRANSCODE))
            m_trx_payee_box->SetToolTip(_("Specify to whom the transaction is going to"));
        else
            m_trx_payee_box->SetToolTip(_("Specify where the transaction is coming from"));
    }

    // Not dynamically changed tooltips
    m_date_picker->SetToolTip(_("Specify the date of the transaction"));
    m_trx_status->SetToolTip(_("Specify the status for the transaction"));
    m_trx_type->SetToolTip(_("Specify the type of transactions to be created."));
    m_split_box->SetToolTip(_("Use split Categories"));
    m_trx_number->SetToolTip(_("Specify any associated check number or transaction number"));
    m_trx_notes->SetToolTip(_("Specify any text notes you want to add to this transaction."));
    m_trx_advanced->SetToolTip(_("Allows the setting of different amounts in the FROM and TO accounts."));
}

void mmTransDialog::OnQuit(wxCloseEvent& WXUNUSED(event))
{
    const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::TRANSACTION);
    if (m_is_new_trx || m_is_duplicate) {
        mmAttachmentManage::DeleteAllAttachments(RefType, m_trx_data.TRANSID);
    }
    EndModal(wxID_CANCEL);
}

void mmTransDialog::OnMoreFields(wxCommandEvent& WXUNUSED(event))
{
    wxBitmapButton* button = static_cast<wxBitmapButton*>(FindWindow(ID_TRX_CUSTOMFIELDS));

    if (button)
        button->SetBitmap(mmBitmap(m_custom_fields->IsCustomPanelShown() ? png::RIGHTARROWSIMPLE : png::LEFTARROWSIMPLE));

    m_custom_fields->ShowHideCustomPanel();

    this->SetMinSize(wxSize(0, 0));
    this->Fit();
}

void mmTransDialog::OnColourButton(wxCommandEvent& /*event*/)
{
    wxCommandEvent ev(wxEVT_COMMAND_MENU_SELECTED, wxID_INFO);
    ev.SetEventObject(this);

    wxMenu* mainMenu = new wxMenu;

    wxMenuItem* menuItem = new wxMenuItem(mainMenu, wxID_HIGHEST, wxString::Format(_("Clear colour"), 0));
    mainMenu->Append(menuItem);

    for (int i = 1; i <= 7; ++i)
    {
        menuItem = new wxMenuItem(mainMenu, wxID_HIGHEST + i, wxString::Format(_("Colour #%i"), i));
#ifdef __WXMSW__
        menuItem->SetBackgroundColour(getUDColour(i)); //only available for the wxMSW port.
#endif
        wxBitmap bitmap(mmBitmap(png::EMPTY).GetSize());
        wxMemoryDC memoryDC(bitmap);
        wxRect rect(memoryDC.GetSize());

        memoryDC.SetBackground(wxBrush(getUDColour(i)));
        memoryDC.Clear();
        memoryDC.DrawBitmap(mmBitmap(png::EMPTY), 0, 0, true);
        memoryDC.SelectObject(wxNullBitmap);
        menuItem->SetBitmap(bitmap);

        mainMenu->Append(menuItem);
    }

    PopupMenu(mainMenu);
    delete mainMenu;
}

void mmTransDialog::OnColourSelected(wxCommandEvent& event)
{
    int selected_nemu_item = event.GetId() - wxID_HIGHEST;
    m_trx_colours_btn->SetBackgroundColour(getUDColour(selected_nemu_item));
    m_trx_data.COLOURID = selected_nemu_item;
}

void mmTransDialog::OnCurrency(wxCommandEvent& /*event*/)
{
    int currency_id;
    if (mmMainCurrencyDialog::Execute(this, currency_id))
    {
        m_currency = Model_Currency::instance().get(currency_id);
        m_trx_currency_btn->SetLabelText(m_currency->CURRENCYNAME);
    }
}
