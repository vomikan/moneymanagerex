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

#include "stock_settings.h"
#include "constants.h"
#include "mmTips.h"
#include "images_list.h"
#include "validators.h"
#include "paths.h"
#include "util.h"

#include "model/Model_Setting.h"
#include "model/Model_StockHistory.h"
#include "model/Model_Ticker.h"

#include "mmSimpleDialogs.h"

wxBEGIN_EVENT_TABLE(mmStockSetup, wxDialog)
    EVT_BUTTON(wxID_OK, mmStockSetup::OnOk)
    EVT_BUTTON(wxID_ADD, mmStockSetup::OnHistoryAddButton)
    EVT_BUTTON(wxID_DELETE, mmStockSetup::OnHistoryDeleteButton)
    EVT_BUTTON(wxID_REFRESH, mmStockSetup::OnHistoryDownloadButton)
    EVT_BUTTON(wxID_OPEN, mmStockSetup::OnHistoryImportButton)
    EVT_LIST_ITEM_SELECTED(wxID_ANY, mmStockSetup::OnListItemSelected)
wxEND_EVENT_TABLE()


mmStockSetup::~mmStockSetup()
{

}

mmStockSetup::mmStockSetup(wxWindow* parent, const wxString& symbol)
    : m_symbol(symbol)
{

    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    bool isDialogCreated = mmStockSetup::Create(parent, wxID_ANY, _("Stock Settings")
        , wxDefaultPosition, wxDefaultSize
        , wxCAPTION | wxRESIZE_BORDER | wxCLOSE_BOX, "mmStockSettings");

    if (isDialogCreated) 
    {
        SetMinSize(wxSize(600, 500));

        CreateControls();
        dataToControls();
        ShowStockHistory();

        this->SetIcon(mmex::getProgramIcon());
        this->Centre();
        this->Layout();
    }
}

void mmStockSetup::dataToControls()
{
    Model_Ticker::Data* i = Model_Ticker::instance().get(m_symbol);

    wxTextCtrl* market = static_cast<wxTextCtrl*>(FindWindow(wxID_FILE1));
    wxTextCtrl* webSite = static_cast<wxTextCtrl*>(FindWindow(wxID_FILE2));

    if (i)
    {
        m_stock_symbol_ctrl->Enable(false);
        market->Enable(false);
        m_stock_name_ctrl->ChangeValue(i->NAME);
        //m_stock_notes_ctrl->ChangeValue(i->NOTES);
        webSite->ChangeValue(i->DASHBOARD);
        m_choiceType->SetSelection(i->TYPE);
        m_choiceSource->SetSelection(i->SOURCE);
        m_choiceSector->SetStringSelection(wxGetTranslation(i->SECTOR));

        //i->INDUSTRY = "INDUSTRY";
    }
    else
    {

    }
    wxLogDebug("---------------");

}

void mmStockSetup::CreateControls()
{
    wxBoxSizer* mainBoxSizer = new wxBoxSizer(wxHORIZONTAL);

    wxBoxSizer* leftBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* rightBoxSizer = new wxBoxSizer(wxVERTICAL);
    mainBoxSizer->Add(leftBoxSizer, g_flagsExpand);
    mainBoxSizer->Add(rightBoxSizer, g_flagsExpand);

    /******************************************************************************
        Items Panel
    *******************************************************************************/
    wxStaticBox* static_box_sizer = new wxStaticBox(this, wxID_ANY, m_symbol);
    wxStaticBoxSizer* itemStaticBoxSizer4 = new wxStaticBoxSizer(static_box_sizer, wxVERTICAL);
    leftBoxSizer->Add(itemStaticBoxSizer4, 1, wxGROW | wxALL, 10);
    wxPanel* itemPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    itemStaticBoxSizer4->Add(itemPanel, g_flagsExpand);

    wxBoxSizer* itemBoxSizer4 = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* flex_sizer = new wxFlexGridSizer(0, 2, 0, 0);
    flex_sizer->AddGrowableCol(1, 1);

    itemPanel->SetSizer(itemBoxSizer4);
    itemBoxSizer4->Add(flex_sizer, g_flagsExpand);

    // Source --------------------------------------------
    wxStaticText* sourceLabel = new wxStaticText(itemPanel, wxID_STATIC, _("Source"));
    flex_sizer->Add(sourceLabel, g_flagsH);
    sourceLabel->SetFont(this->GetFont().Bold());

    m_choiceSource = new wxChoice(itemPanel, wxID_ANY);
    for (const auto& i : Model_Ticker::all_sources()) {
        m_choiceSource->Append(wxGetTranslation(i), new wxStringClientData(i));
    }
    m_choiceSource->Select(0);

    flex_sizer->Add(m_choiceSource, g_flagsExpand);

    // Type --------------------------------------------
    wxStaticText* choiceLabel = new wxStaticText(itemPanel, wxID_STATIC, _("Type"));
    flex_sizer->Add(choiceLabel, g_flagsH);
    choiceLabel->SetFont(this->GetFont().Bold());

    m_choiceType = new wxChoice(itemPanel, wxID_ANY);
    for (const auto& i : { wxTRANSLATE("Stock"), wxTRANSLATE("Fund")
        , wxTRANSLATE("Bond") }) {
        m_choiceType->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    m_choiceType->Select(0);

    flex_sizer->Add(m_choiceType, g_flagsExpand);

    // Symbol --------------------------------------------
    wxStaticText* symbolLabel = new wxStaticText(itemPanel, wxID_STATIC, _("Stock Symbol"));
    flex_sizer->Add(symbolLabel, g_flagsH);
    symbolLabel->SetFont(this->GetFont().Bold());

    m_stock_symbol_ctrl = new mmTextCtrl(itemPanel, wxID_ANY, m_symbol, wxDefaultPosition
        , wxDefaultSize, wxTE_PROCESS_ENTER);
    flex_sizer->Add(m_stock_symbol_ctrl, g_flagsExpand);

    // Market --------------------------------------------
    flex_sizer->Add(new wxStaticText(itemPanel, wxID_STATIC, _("Market")), g_flagsH);
    wxTextCtrl* textMarket = new wxTextCtrl(itemPanel, wxID_FILE1, "");
    flex_sizer->Add(textMarket, g_flagsExpand);

    // Name --------------------------------------------
    flex_sizer->Add(new wxStaticText(itemPanel, wxID_STATIC, _("Description")), g_flagsH);
    m_stock_name_ctrl = new mmTextCtrl(itemPanel, wxID_ANY, "");
    flex_sizer->Add(m_stock_name_ctrl, g_flagsExpand);
    m_stock_name_ctrl->SetToolTip(_("Enter the stock description"));

    // Sector --------------------------------------------
    m_choiceSector = new wxChoice(itemPanel, wxID_ANY);

    for (const auto& i : Model_Ticker::all_sectors())
    {
        m_choiceSector->Append(wxGetTranslation(i), new wxStringClientData(i));
    }

    flex_sizer->Add(new wxStaticText(itemPanel, wxID_STATIC, _("Sector")), g_flagsH);
    flex_sizer->Add(m_choiceSector, g_flagsExpand);

    // Web --------------------------------------------
    flex_sizer->Add(new wxStaticText(itemPanel, wxID_STATIC, _("Dashboard")), g_flagsH);
    wxTextCtrl* webSite = new wxTextCtrl(itemPanel, wxID_FILE2, "", wxDefaultPosition
        , wxSize(200, 90), wxTE_MULTILINE);

    itemBoxSizer4->Add(webSite, g_flagsExpand);

    //---------------------------------------------

    //History Panel

    wxStaticBox* historyStaticBox = new wxStaticBox(this, wxID_ANY, _("Historical Stock Prices"));
    wxStaticBoxSizer* historyStaticBoxSizer = new wxStaticBoxSizer(historyStaticBox, wxVERTICAL);
    rightBoxSizer->Add(historyStaticBoxSizer, g_flagsExpand);

    m_price_listbox = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(220, 150)
        , wxLC_REPORT);
    historyStaticBoxSizer->Add(m_price_listbox, g_flagsExpand);
    m_price_listbox->SetToolTip(_("Stock Price History"));

    // Add first column
    wxListItem col0;
    col0.SetId(0);
    col0.SetText(_("Date"));
    col0.SetWidth(90);
    m_price_listbox->InsertColumn(0, col0);

    // Add second column
    wxListItem col1;
    col1.SetId(1);
    col1.SetText(_("Price"));
    col1.SetWidth(80);
    m_price_listbox->InsertColumn(1, col1);

    // Add 3rd column
    wxListItem col2;
    col2.SetId(2);
    col2.SetText(_("Type"));
    col2.SetWidth(80);
    m_price_listbox->InsertColumn(2, col2);

    //History Buttons
    wxPanel* buttons_panel = new wxPanel(this, wxID_ANY);
    historyStaticBoxSizer->Add(buttons_panel, wxSizerFlags(g_flagsV).Centre());
    wxBoxSizer* buttons_sizer = new wxBoxSizer(wxVERTICAL);
    buttons_panel->SetSizer(buttons_sizer);

    //
    wxFlexGridSizer* date_price = new wxFlexGridSizer(0, 2, 0, 0);
    date_price->Add(new wxStaticText(buttons_panel, wxID_STATIC, _("Price Date")), g_flagsH);
    m_history_date_ctrl = new wxDatePickerCtrl(buttons_panel, wxID_ANY
        , wxDefaultDateTime, wxDefaultPosition, wxSize(150, -1), wxDP_DROPDOWN | wxDP_SHOWCENTURY);
    date_price->Add(m_history_date_ctrl, g_flagsH);
    m_history_date_ctrl->SetToolTip(_("Specify the stock/share price date."));

    //
    date_price->Add(new wxStaticText(buttons_panel, wxID_STATIC, _("Price")), g_flagsH);
    m_history_price_ctrl = new mmTextCtrl(buttons_panel, wxID_ANY, ""
        , wxDefaultPosition, wxSize(150, -1), wxALIGN_RIGHT | wxTE_PROCESS_ENTER, mmCalcValidator());
    date_price->Add(m_history_price_ctrl, g_flagsH);
    m_history_price_ctrl->Connect(wxID_ANY, wxEVT_COMMAND_TEXT_ENTER
        , wxCommandEventHandler(mmStockSetup::OnTextEntered), nullptr, this);
    buttons_sizer->Add(date_price);

    wxStdDialogButtonSizer*  std_buttons_sizer = new wxStdDialogButtonSizer;
    wxBitmapButton* buttonDownload = new wxBitmapButton(buttons_panel, wxID_REFRESH, mmBitmap(png::UPDATE));
    buttonDownload->SetToolTip(_("Download Stock Price history"));
    wxBitmapButton* buttonImport = new wxBitmapButton(buttons_panel, wxID_OPEN, mmBitmap(png::IMPORT));
    buttonImport->SetToolTip(_("Import Stock Price history (CSV Format)"));
    wxButton* buttonDel = new wxButton(buttons_panel, wxID_DELETE, _("&Delete "));
    buttonDel->SetToolTip(_("Delete selected Stock Price"));
    wxButton* buttonAdd = new wxButton(buttons_panel, wxID_ADD, _("Add/Edit"));
    buttonAdd->SetToolTip(_("Add or Edit Stock Price"));

    std_buttons_sizer->Add(buttonDownload, g_flagsH);
    std_buttons_sizer->Add(buttonImport, g_flagsH);
    std_buttons_sizer->Add(buttonDel, g_flagsH);
    std_buttons_sizer->Add(buttonAdd, g_flagsH);
    buttons_sizer->Add(std_buttons_sizer);



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


void mmStockSetup::OnHistoryDownloadButton(wxCommandEvent& /*event*/)
{
    wxString symbol = m_stock_symbol_ctrl->GetValue();
    if (symbol.IsEmpty())
        return;

    m_symbol = symbol.Upper();
    /*"ValidRanges":["1d","5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max"]*/
    enum { DAY5, MON, MON3, MON6, YEAR, YEAR2, YEAR5, YEAR10, YTD, MAX };
    const wxString ranges[] = { "5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max" };
    const std::vector<std::pair<int, wxString> > RANGE_PAIRS =
    {
        { DAY5, _("5 Days") }
        ,{ MON, _("1 Month") }
        ,{ MON3, _("3 Months") }
        ,{ MON6, _("6 Months") }
        ,{ YEAR, _("1 Year") }
        ,{ YEAR2, _("2 Years") }
        ,{ YEAR5, _("5 Years") }
        ,{ YEAR10, _("10 Years") }
        ,{ YTD, _("Current Year to Date") }
        ,{ MAX, _("Max") }
    };

    wxArrayString items;
    for (const auto& entry : RANGE_PAIRS) { items.Add(entry.second); }

    int range_menu_item_no = wxGetSingleChoiceIndex(_("Specify type frequency of stock history")
        , _("Stock History Update"), items);

    if (range_menu_item_no < 0) return;
    const wxString range = ranges[range_menu_item_no];

    wxString interval = "1d";
    if (range != "5d")
    {
        /* Valid intervals : [1m, 2m, 5m, 15m, 30m, 60m, 90m, 1h, 1d, 5d, 1wk, 1mo, 3mo] */
        enum class i { DAY, DAY5, WEEK, MON, MON3 };
        const wxString intervals[] = { "1d","5d","1wk","1mo","3mo" };
        const std::vector<std::pair<int, wxString> > INTERVAL_PAIRS =
        {
              { static_cast<int>(i::DAY), _("1 Day") }
            , { static_cast<int>(i::DAY5), _("5 Days") }
            , { static_cast<int>(i::WEEK), _("1 Week") }
            , { static_cast<int>(i::MON), _("1 Month") }
            , { static_cast<int>(i::MON3), _("3 Months") }
        };

        items.clear();
        for (const auto& entry : INTERVAL_PAIRS) { items.Add(entry.second); }

        int interval_menu_item_no = wxGetSingleChoiceIndex(_("Specify interval of stock history")
            , _("Stock History Update"), items);

        if (interval_menu_item_no < 0) return;
        interval = intervals[interval_menu_item_no];
    }
    const wxString URL = wxString::Format(mmex::weblink::YahooQuotesHistory
        , m_symbol, range, interval);

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    wxString sOutput = json_data;

    if (err_code != CURLE_OK)
    {
        if (sOutput.empty()) sOutput = _("Stock history not found!");
        return mmErrorDialogs::MessageError(this, sOutput, _("Stock History Error"));
    }

    sOutput = _("Stock history not found!");

    while (true)
    {
        Document json_doc;
        if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
            break;
        }
        else if (!json_doc.HasMember("chart") || !json_doc["chart"].IsObject()) {
            break;
        }

        Value chart = json_doc["chart"].GetObject();
        wxASSERT(chart.HasMember("error"));
        if (chart.HasMember("error"))
        {
            if (chart["error"].IsObject())
            {

                // {"chart":{"result":null,"error":{"code":"Not Found","description":"No data found, symbol may be delisted"}}}

                Value e = chart["error"].GetObject();
                if (!e.HasMember("code") || !e.HasMember("description") || !e["code"].IsString() || !e["description"].IsString()) {
                    break;
                }

                const wxString code = wxString::FromUTF8(e["code"].GetString());
                const wxString description = wxString::FromUTF8(e["description"].GetString());
                sOutput = wxString::Format("%s - %s", code, description);
                break;
            }
        }

        if (!chart.HasMember("result") || !chart["result"].IsArray())
            break;
        Value result = chart["result"].GetArray();
        if (!result.IsArray() || !result.Begin()->IsObject())
            break;
        Value data = result.Begin()->GetObject();

        if (!data.HasMember("meta") || !data["meta"].IsObject())
            break;
        Value meta = data["meta"].GetObject();

        float k = 1.0L;
        if (meta.HasMember("currency") && meta["currency"].IsString()) {

            const auto currency = wxString::FromUTF8(meta["currency"].GetString());
            k = (currency == "GBp" ? 100 : 1);
        }

        if (!data.HasMember("timestamp") || !data["timestamp"].IsArray())
            break;
        Value timestamp = data["timestamp"].GetArray();

        if (!data.HasMember("indicators") || !data.IsObject())
            break;
        Value indicators = data["indicators"].GetObject();

        if (!indicators.HasMember("adjclose") || !indicators["adjclose"].IsArray())
            break;
        Value quote_array = indicators["adjclose"].GetArray();
        Value quotes = quote_array.Begin()->GetObject();
        if (!quotes.HasMember("adjclose") || !quotes["adjclose"].IsArray())
            break;
        Value quotes_closed = quotes["adjclose"].GetArray();

        if (timestamp.Size() != quotes_closed.Size())
            break;

        std::map<time_t, float> history;
        for (rapidjson::SizeType i = 0; i < timestamp.Size(); i++)
        {
            if (!timestamp[i].IsInt()) continue;
            time_t time = timestamp[i].GetInt();
            if (!quotes_closed[i].IsFloat()) continue;
            float rate = quotes_closed[i].GetFloat() / k;
            history[time] = rate;
        }

        const wxString today = wxDate::Today().FormatISODate();
        Model_StockHistory::instance().Savepoint();
        for (const auto& entry : history)
        {
            float dPrice = entry.second;
            const wxString date_str = wxDateTime(static_cast<time_t>(entry.first)).FormatISODate();

            Model_StockHistory::Data_Set d = Model_StockHistory::instance().find(Model_StockHistory::DATE(entry.first)
                , Model_StockHistory::SYMBOL(m_symbol));
            if (d.empty()) {
                Model_StockHistory::Data* ndata = Model_StockHistory::instance().create();
                ndata->SYMBOL = m_symbol;
                ndata->DATE = date_str;
                ndata->VALUE = dPrice;
                ndata->UPDTYPE = date_str == today ? Model_StockHistory::CURRENT : Model_StockHistory::ONLINE;
                Model_StockHistory::instance().save(ndata);
            }
            else {
                if (d.begin()->UPDTYPE != Model_StockHistory::MANUAL)
                {
                    Model_StockHistory::Data* ndata = Model_StockHistory::instance().get(d.begin()->HISTID);
                    ndata->VALUE = dPrice;
                    ndata->UPDTYPE = date_str == today ? Model_StockHistory::CURRENT : Model_StockHistory::ONLINE;
                    Model_StockHistory::instance().save(ndata);
                }
            }

        }
        Model_StockHistory::instance().ReleaseSavepoint();
        return ShowStockHistory();
    }
    mmErrorDialogs::MessageError(this, sOutput, _("Stock History Error"));
}

void mmStockSetup::ShowStockHistory()
{
    m_price_listbox->DeleteAllItems();

    if (m_symbol.IsEmpty())
        return;

    Model_StockHistory::Data_Set histData = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(m_symbol));
    std::stable_sort(histData.begin(), histData.end(), SorterByDATE());
    std::reverse(histData.begin(), histData.end());
    if (histData.size() > 300)
        histData.resize(300);
    size_t rows = histData.size() - 1;
    if (!histData.empty())
    {
        for (size_t idx = 0; idx < histData.size(); idx++)
        {
            wxListItem item;
            item.SetId(static_cast<long>(idx));
            item.SetData(histData.at(idx).HISTID);
            m_price_listbox->InsertItem(item);


            const wxDate dtdt = Model_StockHistory::DATE(histData.at(idx));
            const wxString dispAmount = Model_Currency::toString(histData.at(idx).VALUE, nullptr, Option::instance().SharePrecision());
            m_price_listbox->SetItem(static_cast<long>(idx), 0, mmGetDateForDisplay(histData.at(idx).DATE));
            m_price_listbox->SetItem(static_cast<long>(idx), 1, dispAmount);

            wxString update_type;
            switch (histData.at(idx).UPDTYPE)
            {
            case Model_StockHistory::MANUAL:
                update_type = "M";
                break;
            case Model_StockHistory::CURRENT:
                update_type = "*";
                break;
            default:
                break;
            }

            m_price_listbox->SetItem(static_cast<long>(idx), 2, update_type);
            if (idx == 0)
            {
                m_history_date_ctrl->SetValue(dtdt);
                m_history_price_ctrl->SetValue(dispAmount);
            }
        }
        m_price_listbox->RefreshItems(0, rows);
    }
}

void mmStockSetup::OnHistoryImportButton(wxCommandEvent& /*event*/)
{
    wxString symbol = m_stock_symbol_ctrl->GetValue();
    if (symbol.IsEmpty())
        return;

    const wxString fileName = wxFileSelector(_("Choose CSV data file to import")
        , wxEmptyString, wxEmptyString, wxEmptyString, "*.csv", wxFD_FILE_MUST_EXIST);

    if (!fileName.IsEmpty())
    {
        wxFileName csv_file(fileName);
        if (fileName.IsEmpty() || !csv_file.FileExists())
            return;
        wxTextFile tFile(fileName);
        if (!tFile.Open())
            return;
        wxProgressDialog* progressDlg = new wxProgressDialog(_("Stock History CSV Import")
            , _("Quotes imported from CSV: "), tFile.GetLineCount()
            , NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_SMOOTH | wxPD_CAN_ABORT);

        bool canceledbyuser = false;
        long countNumTotal = 0;
        long countImported = 0;
        double price;
        wxString dateStr, priceStr;
        Model_StockHistory::Data *data;
        Model_StockHistory::Cache stockData;

        wxString line;
        std::vector<wxString> rows;
        for (line = tFile.GetFirstLine(); !tFile.Eof(); line = tFile.GetNextLine())
        {
            wxString progressMsg;
            progressMsg << _("Quotes imported from CSV: ") << countImported;
            if (!progressDlg->Update(countImported, progressMsg))
            {
                canceledbyuser = true;
                break; // abort processing
            }

            if (!line.IsEmpty())
                ++countNumTotal;
            else
                continue;

            dateStr.clear();
            priceStr.clear();

            const wxString& delimiter = Model_Infotable::instance().GetStringInfo("DELIMITER", mmex::DEFDELIMTER);
            csv2tab_separated_values(line, delimiter);
            wxStringTokenizer tkz(line, "\t", wxTOKEN_RET_EMPTY_ALL);
            if (static_cast<int>(tkz.CountTokens()) < 2)
                continue;

            std::vector<wxString> tokens;
            while (tkz.HasMoreTokens())
            {
                wxString token = tkz.GetNextToken();
                tokens.push_back(token);
            }

            // date
            wxDateTime dt;
            dateStr = tokens[0];
            mmParseDisplayStringToDate(dt, dateStr, Option::instance().getDateFormat());
            dateStr = dt.FormatISODate();
            // price
            priceStr = tokens[1];
            priceStr.Replace(" ", wxEmptyString);
            if (!Model_Currency::fromString(priceStr, price) || price <= 0.0)
                continue;

            data = Model_StockHistory::instance().create();
            data->SYMBOL = m_symbol;
            data->DATE = dateStr;
            data->VALUE = price;
            data->UPDTYPE = 2;
            stockData.push_back(data);

            if (rows.size() < 10)
            {
                dateStr << wxT("  ") << priceStr;
                rows.push_back(dateStr);
            }
            countImported++;
        }

        progressDlg->Destroy();

        wxString msg = wxString::Format(_("Total Lines : %ld"), countNumTotal);
        msg << "\n";
        msg << wxString::Format(_("Total Imported : %ld"), countImported);
        msg << "\n";
        msg << _("Date") << "              " << _("Price");
        msg << "\n";
        for (std::vector<wxString>::const_iterator d = rows.begin(); d != rows.end(); ++d)
            msg << *d << "\n";
        wxString confirmMsg = msg + _("Please confirm saving...");
        if (!canceledbyuser && wxMessageBox(confirmMsg
            , _("Importing CSV"), wxOK | wxCANCEL | wxICON_INFORMATION) == wxCANCEL)
        {
            canceledbyuser = true;
        }

        // Since all database transactions are only in memory,
        if (!canceledbyuser)
        {
            // we need to save them to the database. 
            for (auto &d : stockData)
                Model_StockHistory::instance().save(d);
            // show the data
            ShowStockHistory();
        }
        else
        {
            //TODO: and discard the database changes.
        }
    }
}

void mmStockSetup::OnListItemSelected(wxListEvent& event)
{
    long selectedIndex = event.GetIndex();
    long histId = m_price_listbox->GetItemData(selectedIndex);
    Model_StockHistory::Data *histData = Model_StockHistory::instance().get(histId);

    if (histData->HISTID > 0)
    {
        m_history_date_ctrl->SetValue(Model_StockHistory::DATE(*histData));
        m_history_price_ctrl->SetValue(Model_Currency::toString(histData->VALUE, nullptr, Option::instance().SharePrecision()));
    }
}

void mmStockSetup::OnHistoryDeleteButton(wxCommandEvent& /*event*/)
{
    if (m_price_listbox->GetSelectedItemCount() <= 0)
        return;

    long item = -1;
    Model_StockHistory::instance().Savepoint();
    for (;;)
    {
        item = m_price_listbox->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

        if (item == -1)
            break;
        Model_StockHistory::instance().remove(static_cast<int>(m_price_listbox->GetItemData(item)));
    }
    Model_StockHistory::instance().ReleaseSavepoint();
    ShowStockHistory();
}

void mmStockSetup::OnHistoryAddButton(wxCommandEvent& /*event*/)
{
    if (m_symbol.IsEmpty())
        return;

    wxString listStr;
    wxDateTime dt;
    double dPrice = 0.0;

    wxString currentPriceStr = m_history_price_ctrl->GetValue().Trim();
    if (!Model_Currency::fromString(currentPriceStr, dPrice) || (dPrice < 0.0))
        return;

    long histID = Model_StockHistory::instance().addUpdate(m_symbol, m_history_date_ctrl->GetValue(), dPrice, Model_StockHistory::MANUAL);
    long i;
    for (i = 0; i < m_price_listbox->GetItemCount(); i++)
    {
        listStr = m_price_listbox->GetItemText(i, 0);
        mmParseDisplayStringToDate(dt, listStr, Option::instance().getDateFormat());
        if (dt.IsSameDate(m_history_date_ctrl->GetValue()))
            break;
    }
    if (i == m_price_listbox->GetItemCount())
    {
        //add
        for (i = 0; i < m_price_listbox->GetItemCount(); i++)
        {
            listStr = m_price_listbox->GetItemText(i, 0);
            mmParseDisplayStringToDate(dt, listStr, Option::instance().getDateFormat());
            if (dt.GetDateOnly() < m_history_date_ctrl->GetValue().GetDateOnly())
                break;
        }
        wxListItem item;
        item.SetId(i);
        item.SetData(histID);
        m_price_listbox->InsertItem(item);
    }
    if (i != m_price_listbox->GetItemCount())
    {
        listStr = Model_Currency::toString(dPrice, nullptr, Option::instance().SharePrecision());
        m_price_listbox->SetItem(i, 0, mmGetDateForDisplay(m_history_date_ctrl->GetValue().FormatISODate()));
        m_price_listbox->SetItem(i, 1, listStr);
    }
    ShowStockHistory();
}

void mmStockSetup::OnTextEntered(wxCommandEvent& event)
{
    Model_Currency::Data *currency = Model_Currency::GetBaseCurrency();

    int currency_precision = Model_Currency::precision(currency);
    if (currency_precision < Option::instance().SharePrecision())
        currency_precision = Option::instance().SharePrecision();

    if (event.GetId() == m_history_price_ctrl->GetId())
    {
        m_history_price_ctrl->Calculate(Option::instance().SharePrecision());
    }

}

void mmStockSetup::OnOk(wxCommandEvent& WXUNUSED(event))
{
    m_symbol = m_stock_symbol_ctrl->GetValue();
    wxTextCtrl* webSite = static_cast<wxTextCtrl*>(FindWindow(wxID_FILE2));

    Model_Ticker::Data* i = Model_Ticker::instance().get(m_symbol);

    if (!i)
    {
        i = Model_Ticker::instance().create();
        i->SYMBOL = m_symbol;
    }

    int t = m_choiceType->GetSelection();
    if (t < 0) t = 0;
    i->TYPE = t;
    wxString sectorStr;
    int sector = m_choiceSector->GetSelection();
    if (sector >= 0) {
        sectorStr = Model_Ticker::all_sectors()[sector];
    }
    i->SECTOR = sectorStr;
    int s = m_choiceSource->GetSelection();
    if (s < 0) s = 0;
    i->SOURCE = s;
    //i->INDUSTRY = "INDUSTRY";
    i->DASHBOARD = webSite->GetValue();
    i->NAME = m_stock_name_ctrl->GetValue();
    //i->NOTES = "NOTES";

    Model_Ticker::instance().save(i);

    EndModal(wxID_OK);
}
