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

#include "attachmentdialog.h"
#include "mmreportspanel.h"
#include "mmex.h"
#include "mmframe.h"
#include "mmcheckingpanel.h"
#include "paths.h"
#include "platfdep.h"
#include "transdialog.h"
#include "util.h"
#include "reports/htmlbuilder.h"
#include "model/allmodel.h"
#include <wx/wrapsizer.h>

wxBEGIN_EVENT_TABLE(mmReportsPanel, wxPanel)
EVT_CHOICE(ID_CHOICE_DATE_RANGE, mmReportsPanel::OnDateRangeChanged)
EVT_CHOICE(ID_CHOICE_ACCOUNTS, mmReportsPanel::OnAccountChanged)
EVT_DATE_CHANGED(wxID_ANY, mmReportsPanel::OnStartEndDateChanged)
EVT_CHOICE(ID_CHOICE_CHART, mmReportsPanel::OnChartChanged)
EVT_BUTTON(wxID_ANY, mmReportsPanel::OnShiftPressed)
wxEND_EVENT_TABLE()

mmReportsPanel::mmReportsPanel(
    mmPrintableBase* rb, bool cleanupReport, wxWindow *parent, mmGUIFrame* frame,
    wxWindowID winid, const wxPoint& pos,
    const wxSize& size, long style,
    const wxString& name)
    : rb_(rb)
    , m_frame(frame)
    , m_date_ranges(nullptr)
    , m_start_date(nullptr)
    , m_end_date(nullptr)
    , m_accounts(nullptr)
    , m_chart(nullptr)
    , cleanup_(cleanupReport)
    , cleanupmem_(false)
    , m_shift(0)
{
    m_all_date_ranges.push_back(new mmCurrentMonth());
    m_all_date_ranges.push_back(new mmCurrentMonthToDate());
    m_all_date_ranges.push_back(new mmLastMonth());
    m_all_date_ranges.push_back(new mmLast30Days());
    m_all_date_ranges.push_back(new mmLast90Days());
    m_all_date_ranges.push_back(new mmLast3Months());
    m_all_date_ranges.push_back(new mmLast12Months());
    m_all_date_ranges.push_back(new mmCurrentYear());
    m_all_date_ranges.push_back(new mmCurrentYearToDate());
    m_all_date_ranges.push_back(new mmLastYear());

    int day = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_DAY", 1);
    int month = Model_Infotable::instance().GetIntInfo("FINANCIAL_YEAR_START_MONTH", 7);

    m_all_date_ranges.push_back(new mmCurrentFinancialYear(day, month));
    m_all_date_ranges.push_back(new mmCurrentFinancialYearToDate(day, month));
    m_all_date_ranges.push_back(new mmLastFinancialYear(day, month));
    m_all_date_ranges.push_back(new mmAllTime());
    m_all_date_ranges.push_back(new mmLast365Days());
    m_all_date_ranges.push_back(new mmSpecifiedRange(wxDate::Today().SetDay(1), wxDate::Today()));

    Create(parent, winid, pos, size, style, name);
}

mmReportsPanel::~mmReportsPanel()
{
    if (cleanup_ && rb_) {
        delete rb_;
    }
    std::for_each(m_all_date_ranges.begin(), m_all_date_ranges.end(), std::mem_fun(&mmDateRange::destroy));
    m_all_date_ranges.clear();
    clearVFprintedFiles("rep");
}

bool mmReportsPanel::Create(wxWindow *parent, wxWindowID winid
    , const wxPoint& pos, const wxSize& size, long style
    , const wxString& name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);
    wxPanel::Create(parent, winid, pos, size, style, name);

    rb_->restoreReportSettings();

    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    saveReportText();
    this->SetLabel(rb_->getReportTitle(false));
    Model_Usage::instance().pageview(this);

    return TRUE;
}

bool mmReportsPanel::saveReportText(bool initial)
{

    if (!rb_) return false;

    rb_->initial_report(initial);
    if (m_date_ranges)
    {
        int selectedItem = m_date_ranges->GetSelection();
        wxASSERT(selectedItem >= 0 && selectedItem < static_cast<int>(m_date_ranges->GetCount()));

        int rp = rb_->report_parameters();
        if (rp & rb_->RepParams::DATE_RANGE)
        {
            mmDateRange* date_range = static_cast<mmDateRange*>(m_date_ranges->GetClientData(selectedItem));

            if (date_range->title() == "Custom")
            {
                wxDateTime begin_date = m_start_date->GetValue();
                wxDateTime end_date = m_end_date->GetValue();
                date_range->start_date(begin_date);
                date_range->end_date(end_date);
                m_start_date->Enable();
                m_end_date->Enable();
            }
            rb_->date_range(date_range, selectedItem);
        }

        if (rp & (rb_->RepParams::BUDGET_DATES | rb_->RepParams::ONLY_YEARS))
        {
            wxString id_str = "0";
            wxStringClientData* obj =
                static_cast<wxStringClientData*>(m_date_ranges->GetClientObject(selectedItem));
            if (obj) id_str = obj->GetData();
            int id = wxAtoi(id_str);
            rb_->setSelection(id);
        }
    }
    /**/
    StringBuffer json_buffer;
    Writer<StringBuffer> json_writer(json_buffer);

    json_writer.StartObject();
    json_writer.Key("module");
    json_writer.String("Report");
    json_writer.Key("name");
    json_writer.String(rb_->getReportTitle(false).utf8_str());

    const auto time = wxDateTime::UNow();

    const auto& name = getVFname4print("rep", rb_->getHTMLText());
    browser_->LoadURL(name);

    json_writer.Key("seconds");
    json_writer.Double((wxDateTime::UNow() - time).GetMilliseconds().ToDouble() / 1000);
    json_writer.EndObject();

    Model_Usage::instance().AppendToUsage(wxString::FromUTF8(json_buffer.GetString())); //saveReportText

    return true;
}

// Adjust wxStaticText size after font change
// Workaround for not auto Layout() after SetFont()
void mmSetOwnFont(wxStaticText* w, const wxFont& font)
{
    w->SetOwnFont(font);
    wxString label = w->GetLabelText();
    if (!label.IsEmpty())
        w->SetInitialSize(w->GetTextExtent(label));
}


void mmReportsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    SetSizer(itemBoxSizer2);

    wxPanel* itemPanel3 = new wxPanel(this, wxID_ANY);
    itemBoxSizer2->Add(itemPanel3, 0, wxGROW | wxALL, 0);

    wxWrapSizer* itemBoxSizerHeader = new wxWrapSizer();
    itemPanel3->SetSizer(itemBoxSizerHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(itemPanel3, wxID_ANY, "");
    itemBoxSizerHeader->Add(itemStaticText9, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2);

    if (rb_)
    {
        int rp = rb_->report_parameters();

        if (rp == 0) {
            itemPanel3->SetMinSize(wxSize(0, 0));
            itemPanel3->Fit();
        }

        if (rp & rb_->RepParams::DATE_RANGE)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _("Period:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            m_date_ranges = new wxChoice(itemPanel3, ID_CHOICE_DATE_RANGE);
            m_date_ranges->SetName("DateRanges");

            for (const auto & date_range : m_all_date_ranges) {
                m_date_ranges->Append(date_range->local_title(), date_range);
            }

            int sel_id = rb_->getDateSelection();
            if (sel_id < 0 || static_cast<size_t>(sel_id) >= m_all_date_ranges.size()) {
                sel_id = 0;
            }
            m_date_ranges->SetSelection(sel_id);

            itemBoxSizerHeader->Add(m_date_ranges, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            const mmDateRange* date_range = m_all_date_ranges.at(sel_id);
            long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;
            m_start_date = new wxDatePickerCtrl(itemPanel3, ID_CHOICE_START_DATE
                , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, date_style);
            m_start_date->SetValue(date_range->start_date());
            m_start_date->Enable(false);

            m_end_date = new wxDatePickerCtrl(itemPanel3, ID_CHOICE_END_DATE
                , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, date_style);
            m_end_date->SetValue(date_range->end_date());
            m_end_date->Enable(false);

            itemBoxSizerHeader->Add(m_start_date, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            itemBoxSizerHeader->Add(m_end_date, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
        else if (rp & rb_->RepParams::SINGLE_DATE)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _("Date"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            long date_style = wxDP_DROPDOWN | wxDP_SHOWCENTURY;
            m_start_date = new wxDatePickerCtrl(itemPanel3, ID_CHOICE_START_DATE
                , wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, date_style);
            m_start_date->SetValue(wxDateTime::Today());
            m_start_date->Enable(true);

            m_end_date = nullptr;

            itemBoxSizerHeader->Add(m_start_date, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
        else if (rp & rb_->RepParams::MONTHES)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _("Date"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            mmDateYearMonth* up_down_month = new mmDateYearMonth(itemPanel3);
            up_down_month->Connect(wxEVT_BUTTON, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(mmDateYearMonth::OnButtonPress), nullptr, this);
            rb_->setSelection(m_shift);

            itemBoxSizerHeader->Add(up_down_month, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & (rb_->RepParams::BUDGET_DATES | rb_->RepParams::ONLY_YEARS))
        {
            cleanupmem_ = true;
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _("Period:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);

            m_date_ranges = new wxChoice(itemPanel3, ID_CHOICE_DATE_RANGE
                , wxDefaultPosition, wxDefaultSize, 0, NULL, wxCB_SORT);

            for (const auto& e : Model_Budgetyear::instance().all(Model_Budgetyear::COL_BUDGETYEARNAME))
            {
                const wxString& name = e.BUDGETYEARNAME;

                if ((rp & rb_->RepParams::ONLY_YEARS)
                    && name.length() >= 5) // Only add YEARS
                    continue;

                m_date_ranges->Append(name, new wxStringClientData(wxString::Format("%i", e.BUDGETYEARID)));

            }

            int sel_id = rb_->getDateSelection();
            if (sel_id < 0 || static_cast<size_t>(sel_id) >= m_date_ranges->GetCount()) {
                sel_id = 0;
            }
            m_date_ranges->SetSelection(sel_id);

            itemBoxSizerHeader->Add(m_date_ranges, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & rb_->RepParams::ACCOUNTS_LIST)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3, wxID_ANY, _("Accounts:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            m_accounts = new wxChoice(itemPanel3, ID_CHOICE_ACCOUNTS);
            m_accounts->Append(_("All Accounts"));
            m_accounts->Append(_("Specific Accounts"));
            for (const auto& e : Model_Account::instance().TYPE_CHOICES)
            {
                if (e.first != Model_Account::INVESTMENT) {
                    m_accounts->Append(wxGetTranslation(e.second), new wxStringClientData(e.second));
                }
            }
            m_accounts->SetSelection(rb_->getAccountSelection());

            itemBoxSizerHeader->Add(m_accounts, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }

        if (rp & rb_->RepParams::CHART)
        {
            wxStaticText* itemStaticTextH1 = new wxStaticText(itemPanel3
                , wxID_ANY, _("Chart:"));
            mmSetOwnFont(itemStaticTextH1, GetFont().Larger());
            itemBoxSizerHeader->Add(itemStaticTextH1, 0, wxALL | wxALIGN_CENTER_VERTICAL, 1);
            itemBoxSizerHeader->AddSpacer(5);
            m_chart = new wxChoice(itemPanel3, ID_CHOICE_CHART);
            m_chart->Append(_("Show"));
            m_chart->Append(_("Hide"));
            m_chart->SetSelection(rb_->getChartSelection());

            itemBoxSizerHeader->Add(m_chart, 0, wxALL, 1);
            itemBoxSizerHeader->AddSpacer(30);
        }
    }

    browser_ = wxWebView::New(this, mmID_BROWSER);
    browser_->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new wxWebViewFSHandler("memory")));

    Bind(wxEVT_WEBVIEW_NEWWINDOW, &mmReportsPanel::OnNewWindow, this, browser_->GetId());

    itemBoxSizer2->Add(browser_, 1, wxGROW | wxALL, 1);
}

void mmReportsPanel::PrintPage()
{
    browser_->Print();
}

void mmReportsPanel::OnDateRangeChanged(wxCommandEvent& WXUNUSED(event))
{
    auto i = this->m_date_ranges->GetSelection();
    const mmDateRange* date_range = static_cast<mmDateRange*>(this->m_date_ranges->GetClientData(i));
    if (date_range)
    {
        m_start_date->Enable(false);
        this->m_start_date->SetValue(date_range->start_date());
        m_end_date->Enable(false);
        this->m_end_date->SetValue(date_range->end_date());
        rb_->setSelection(i);
        rb_->setReportSettings();
    }

    saveReportText(false);
}

void mmReportsPanel::OnAccountChanged(wxCommandEvent& WXUNUSED(event))
{
    if (rb_)
    {
        int sel = m_accounts->GetSelection();
        if ((sel == 1) || (sel != rb_->getAccountSelection()))
        {
            wxString accountSelection;
            wxStringClientData* type_obj = static_cast<wxStringClientData *>(m_accounts->GetClientObject(m_accounts->GetSelection()));
            if (type_obj) {
                accountSelection = type_obj->GetData();
            }
            rb_->setAccounts(sel, accountSelection);

            saveReportText(false);
            rb_->setReportSettings();
        }
    }
}

void mmReportsPanel::OnStartEndDateChanged(wxDateEvent& WXUNUSED(event))
{
    if (rb_)
    {
        saveReportText(false);
        rb_->setReportSettings();
    }
}

void mmReportsPanel::OnChartChanged(wxCommandEvent& WXUNUSED(event))
{
    if (rb_)
    {
        int sel = m_chart->GetSelection();
        if ((sel == 1) || (sel != rb_->getChartSelection()))
        {
            rb_->chart(sel);
            saveReportText(false);
            rb_->setReportSettings();
        }
    }
}

void mmReportsPanel::OnShiftPressed(wxCommandEvent& event)
{
    if (rb_)
    {
        m_shift = event.GetInt();
        rb_->setSelection(m_shift);
        saveReportText(false);
    }
}

void mmReportsPanel::OnNewWindow(wxWebViewEvent& evt)
{
    const wxString uri = evt.GetURL();
    wxString sData;

    wxRegEx pattern(R"(^https?:\/\/)");
    if (pattern.Matches(uri))
    {
        wxLaunchDefaultBrowser(uri);
    }
    else if (uri.StartsWith("trxid:", &sData))
    {
        long transID = -1;
        if (sData.ToLong(&transID)) {
            const Model_Checking::Data* transaction = Model_Checking::instance().get(transID);
            if (transaction && transaction->TRANSID > -1)
            {
                const Model_Account::Data* account = Model_Account::instance().get(transaction->ACCOUNTID);
                if (account) {
                    m_frame->setAccountNavTreeSection(account->ACCOUNTNAME);
                    m_frame->setGotoAccountID(transaction->ACCOUNTID, transID);
                    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, MENU_GOTOACCOUNT);
                    m_frame->GetEventHandler()->AddPendingEvent(event);
                }
            }
        }
    }
    else if (uri.StartsWith("trx:", &sData))
    {
        long transId = -1;
        if (sData.ToLong(&transId))
        {
            Model_Checking::Data* transaction = Model_Checking::instance().get(transId);
            if (transaction && transaction->TRANSID > -1)
            {
                mmTransDialog dlg(m_frame, -1, transId);
                if (dlg.ShowModal() == wxID_OK)
                {
                    rb_->getHTMLText();
                    saveReportText();
                }

                const auto name = getVFname4print("rep", getPrintableBase()->getHTMLText());
                browser_->LoadURL(name);
            }
        }
    }
    else if (uri.StartsWith("attachment:", &sData))
    {
        const wxString RefType = sData.BeforeFirst('|');
        int RefId = wxAtoi(sData.AfterFirst('|'));

        if (Model_Attachment::instance().all_type().Index(RefType) != wxNOT_FOUND && RefId > 0)
        {
            mmAttachmentManage::OpenAttachmentFromPanelIcon(m_frame, RefType, RefId);
            const auto name = getVFname4print("rep", getPrintableBase()->getHTMLText());
            browser_->LoadURL(name);
        }
    }

    evt.Skip();
}
