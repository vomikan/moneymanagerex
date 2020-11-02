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

#include "stock_item.h"
#include "constants.h"
#include "attachmentdialog.h"

#include "validators.h"
#include "paths.h"
#include "util.h"

#include "mmSimpleDialogs.h"
#include "model/Model_Ticker.h"
#include "model/Model_Attachment.h"
#include <wx/webviewfshandler.h>

wxBEGIN_EVENT_TABLE(mmStockItem, wxDialog)
    EVT_BUTTON(wxID_OK, mmStockItem::OnOk)
    EVT_BUTTON(wxID_FILE, mmStockItem::OnAttachments)
wxEND_EVENT_TABLE()


mmStockItem::~mmStockItem()
{

}

mmStockItem::mmStockItem(wxWindow* parent, int acc, int id, const wxString& symbol, int type)
    : m_acc(acc)
    , m_id(id)
    , m_symbol(symbol)
    , m_type(type)
{

    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    bool isDialogCreated = mmStockItem::Create(parent, wxID_ANY, _("Stock Item")
        , wxDefaultPosition, wxDefaultSize
        , wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX, "mmStockItem");

    if (isDialogCreated) 
    {
        SetMinSize(wxSize(600, 500));

        CreateControls();

        SetEvtHandlerEnabled(false);
        fillControls();
        SetEvtHandlerEnabled(true);

        this->SetIcon(mmex::getProgramIcon());
        this->Centre();
        this->Layout();
    }
}

void mmStockItem::fillControls()
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();
    int currency_precision = Model_Currency::precision(currency);
    m_share_precision = currency_precision;
    Model_Ticker::Data* t = Model_Ticker::instance().get(m_symbol);
    if (t) m_share_precision = t->PRECISION;
    if (currency_precision < m_share_precision)
        currency_precision = m_share_precision;

    Model_Stock::Data* stock = Model_Stock::instance().get(m_id);
    if (stock)
    {
        m_purchase_date_ctrl->SetValue(Model_Stock::PURCHASEDATE(stock));
        m_num_shares_ctrl->SetValue(fabs(stock->NUMSHARES), floor(stock->NUMSHARES) ? 0 : m_share_precision);
        if (stock->NUMSHARES < 0) {
            m_type = 1;
            m_choiceType->Select(m_type);
        }
        m_purchase_price_ctrl->SetValue(stock->PURCHASEPRICE, m_share_precision);
        m_commission_ctrl->SetValue(stock->COMMISSION, m_share_precision);
        m_notes_ctrl->SetValue(stock->NOTES);
    }
    else
    {
        m_choiceType->Select(m_type);
    }

}

void mmStockItem::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* leftBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(leftBoxSizer, g_flagsExpand);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    /******************************************************************************
        Items Panel
    *******************************************************************************/
    wxStaticBox* static_box_sizer = new wxStaticBox(this, wxID_ANY, _("Specify"));
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(static_box_sizer, wxVERTICAL);
    leftBoxSizer->Add(itemStaticBoxSizer4, 1, wxGROW | wxALL, 10);
    wxPanel* itemPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel, g_flagsExpand);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    flex_sizer->AddGrowableCol(1, 1);

    itemPanel->SetSizer(itemBoxSizer4);
    itemBoxSizer4->Add(flex_sizer, g_flagsExpand);

    // Type --------------------------------------------
    wxStaticText* typeLabel = new wxStaticText(itemPanel, wxID_STATIC, _("Source"));
    flex_sizer->Add(typeLabel, g_flagsH);
    typeLabel->SetFont(this->GetFont().Bold());

    m_choiceType = new wxChoice(itemPanel, wxID_ANY);
    for (const auto& i : { wxTRANSLATE("Buy"), wxTRANSLATE("Sale"), wxTRANSLATE("Dividend") }) {
        m_choiceType->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    m_choiceType->Select(0);
    m_choiceType->Enable(false);

    flex_sizer->Add(m_choiceType, g_flagsExpand);

    // Date --------------------------------------------
    wxStaticText* date_label = new wxStaticText(itemPanel, wxID_STATIC, _("*Date"));
    flex_sizer->Add(date_label, g_flagsH);
    date_label->SetFont(this->GetFont().Bold());
    m_purchase_date_ctrl = new wxDatePickerCtrl(itemPanel, ID_STOCK_DATE
        , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    flex_sizer->Add(m_purchase_date_ctrl, g_flagsExpand);

    //Number of Shares --------------------------------------------
    wxStaticText* number = new wxStaticText(itemPanel, wxID_STATIC, _("Number of shares"));
    flex_sizer->Add(number, g_flagsH);
    number->SetFont(this->GetFont().Bold());
    m_num_shares_ctrl = new mmTextCtrl(itemPanel, ID_TEXTCTRL_NUMBER_SHARES, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    flex_sizer->Add(m_num_shares_ctrl, g_flagsExpand);

    //Purchase Price --------------------------------------------
    wxStaticText* pprice = new wxStaticText(itemPanel, wxID_STATIC, _("Price"));
    flex_sizer->Add(pprice, g_flagsH);
    pprice->SetFont(this->GetFont().Bold());

    m_purchase_price_ctrl = new mmTextCtrl(itemPanel, ID_TEXTCTRL_STOCK_PP, ""
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    flex_sizer->Add(m_purchase_price_ctrl, g_flagsExpand);

    // Commission --------------------------------------------
    flex_sizer->Add(new wxStaticText(itemPanel, wxID_STATIC, _("Commission")), g_flagsH);

    m_commission_ctrl = new mmTextCtrl(itemPanel, ID_TEXTCTRL_STOCK_COMMISSION, "0"
        , wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    flex_sizer->Add(m_commission_ctrl, g_flagsExpand);

    // Notes --------------------------------------------
    m_notes_ctrl = new mmTextCtrl(itemPanel, wxID_STATIC, "", wxDefaultPosition, wxSize(200, 90), wxTE_MULTILINE);
    itemBoxSizer4->Add(m_notes_ctrl, g_flagsExpand);
    itemBoxSizer4->AddSpacer(1);

    leftBoxSizer->AddSpacer(20);

    m_purchase_date_ctrl->SetToolTip(_("Specify the date of the investment"));
    m_num_shares_ctrl->SetToolTip(_("Enter the number of items (shares, bonds, funds)."));
    m_purchase_price_ctrl->SetToolTip(_("Enter the price of investment."));
    m_commission_ctrl->SetToolTip(_("Enter any commission paid."));
    m_notes_ctrl->SetToolTip(_("Enter the notes associated with this investment"));

    m_num_shares_ctrl->Connect(ID_TEXTCTRL_NUMBER_SHARES, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockItem::OnTextEntered), nullptr, this);
    m_purchase_price_ctrl->Connect(ID_TEXTCTRL_STOCK_PP, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockItem::OnTextEntered), nullptr, this);
    m_commission_ctrl->Connect(ID_TEXTCTRL_STOCK_COMMISSION, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockItem::OnTextEntered), nullptr, this);

    //---------------------------------------------

    // Bar
    wxStaticBox* infoStaticBox = new wxStaticBox(this, wxID_ANY, _("Info"));
    wxStaticBoxSizer* historyStaticBoxSizer = new wxStaticBoxSizer(infoStaticBox, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBoxSizer, g_flagsExpand);
    wxPanel* infoPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    historyStaticBoxSizer->Add(infoPanel, g_flagsExpand);

    wxBoxSizer* itemBoxSizer5 = new wxBoxSizer(wxVERTICAL);
    infoPanel->SetSizer(itemBoxSizer5);

    browser_ = wxWebView::New(infoPanel, mmID_BROWSER);
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));

    //Bind(wxEVT_WEBVIEW_NEWWINDOW, &mmReportsPanel::OnNewWindow, this, browser_->GetId());

    itemBoxSizer5->Add(browser_, 1, wxGROW | wxALL, 1);


    //
    wxPanel* buttons_panel2 = new wxPanel(this, wxID_ANY);
    leftBoxSizer->Add(buttons_panel2, wxSizerFlags(g_flagsV).Center().Border(wxALL, 0));


    wxStdDialogButtonSizer*  buttons_sizer2 = new wxStdDialogButtonSizer;
    buttons_panel2->SetSizer(buttons_sizer2);

    wxButton* itemButtonOK = new wxButton(buttons_panel2, wxID_OK, _("&OK "));
    wxButton* itemButtonCancel_ = new wxButton(buttons_panel2, wxID_CANCEL, wxGetTranslation(g_CancelLabel));
    buttons_sizer2->Add(itemButtonOK, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));
    buttons_sizer2->Add(itemButtonCancel_, wxSizerFlags(g_flagsH).Border(wxBOTTOM | wxRIGHT, 10));

    buttons_sizer2->Realize();

    this->SetSizer(mainBoxSizer);
    mainBoxSizer->Fit(this);
}


void mmStockItem::OnTextEntered(wxCommandEvent& event)
{


    if (event.GetId() == m_num_shares_ctrl->GetId())
    {
        double amount;
        m_num_shares_ctrl->checkValue(amount);
        m_num_shares_ctrl->Calculate(floor(amount) ? 0 : m_share_precision);
    }
    else if (event.GetId() == m_purchase_price_ctrl->GetId())
    {
        m_purchase_price_ctrl->Calculate(m_share_precision);
    }
    else if (event.GetId() == m_commission_ctrl->GetId())
    {
        m_commission_ctrl->Calculate(m_share_precision);
    }

}

void mmStockItem::OnOk(wxCommandEvent& WXUNUSED(event))
{
    Model_Stock::Data* stock = Model_Stock::instance().get(m_id);
    if (!stock) {
        stock = Model_Stock::instance().create();
        stock->HELDAT = m_acc;
        stock->SYMBOL = m_symbol;
    }

    double num, comm, price;
    wxString date, notes;

    Model_Account::Data* acc = Model_Account::instance().get(m_acc);
    Model_Currency::Data* currency = Model_Account::currency(acc);
    date = m_purchase_date_ctrl->GetValue().FormatISODate();
    notes = m_notes_ctrl->GetValue();

    m_num_shares_ctrl->Calculate(Model_Currency::precision(currency));
    if (!m_num_shares_ctrl->checkValue(num, true))
        return;

    m_purchase_price_ctrl->Calculate(Model_Currency::precision(currency));
    if (!m_purchase_price_ctrl->checkValue(price, true))
        return;

    m_commission_ctrl->Calculate(Model_Currency::precision(currency));
    if (!m_commission_ctrl->checkValue(comm, true))
        return;

    stock->PURCHASEDATE = date;
    stock->NUMSHARES = m_type == 0 ? num : -num;
    stock->COMMISSION = comm;
    stock->NOTES = notes;
    stock->PURCHASEPRICE = price;

    Model_Stock::instance().Savepoint();
    Model_Stock::instance().save(stock);
    Model_Stock::instance().ReleaseSavepoint();

    EndModal(wxID_OK);
}

void mmStockItem::OnAttachments(wxCommandEvent& /*event*/)
{
    const wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = m_id;

    if (RefId < 0)
        RefId = 0;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();
}

void mmStockItem::OnQuit(wxCloseEvent& /*event*/)
{
    if (m_id < 0) {
        const wxString& RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
        mmAttachmentManage::DeleteAllAttachments(RefType, 0);
    }
    EndModal(wxID_CANCEL);
}

#if 0
void mmStockItem::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = m_stocks[m_selected_row].STOCKID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    doRefreshItems(RefId);
}

void mmStockItem::OnOpenAttachment(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::STOCK);
    int RefId = m_stocks[m_selected_row].STOCKID;

    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, RefId);
    doRefreshItems(RefId);
}

void mmStocksPanel::OnOpenAttachment(wxCommandEvent& event)
{
    listCtrlAccount_->OnOpenAttachment(event);
}

#endif

