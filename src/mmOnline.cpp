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

#include "mmOnline.h"
#include "util.h"
#include "constants.h"
#include "model/Model_StockHistory.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <vector>
#include <wx/sstream.h>
#include <wx/xml/xml.h>

mmOnline::mmOnline(const wxString& ticker, const wxString& market, const wxString& currency, int source, int type) :
    m_ticker(ticker)
    , m_type(type)
    , m_source(source)
    , m_market(market)
    , m_currency(currency)
{

    Model_StockHistory::Data_Set d = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(m_ticker));
    std::stable_sort(d.begin(), d.end(), SorterByDATE());
    std::reverse(d.begin(), d.end());

    m_date = d.empty() ? wxDate::Today().Subtract(wxDateSpan::Week()).FormatISODate() : d.back().DATE;
    wxLogDebug("%s", m_date);

    switch (m_source)
    {
    case Model_Ticker::MOEX:
        mmMOEX();
        break;

    case Model_Ticker::MS:
        mmMorningStar();
        break;

    default:
        mmYahoo();
        break;
    }

}

//mmOnline::mmOnline(Model_Ticker::Data* t, const wxString& currency)
//{
//    mmOnline(t->SYMBOL, t->MARKET, currency, t->SOURCE, t->TYPE);
//}

mmOnline::~mmOnline()
{

}

bool mmOnline::mmYahoo()
{
    /*"ValidRanges":["1d","5d","1mo","3mo","6mo","1y","2y","5y","10y","ytd","max"]*/
    /* Valid intervals : [1m, 2m, 5m, 15m, 30m, 60m, 90m, 1h, 1d, 5d, 1wk, 1mo, 3mo] */

    wxString range = "5d", interval = "1d";

    // https://query1.finance.yahoo.com/v8/finance/chart/%s?range=%s&interval=%s&fields=currency
    const wxString URL = wxString::Format(mmex::weblink::YahooQuotesHistory
        , m_ticker, range, interval);

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    m_error = json_data;

    if (err_code != CURLE_OK) {
        return false;
    }

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
                m_error = wxString::Format("%s - %s", code, description);
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
        saveData(history);
        return true;
    }
    m_error = _("Error");
    return false;
}

bool mmOnline::mmMOEX()
{
    const wxString today = wxDate::Today().FormatISODate();
    size_t date = wxDate::Today().Subtract(wxDateSpan::Days(7)).GetTicks();
    const wxString dateStr = wxDateTime(static_cast<time_t>(date)).FormatISODate();
    std::map<time_t, float> history;

    wxString type;
    switch (m_type)
    {
    case Model_Ticker::FUND:
        type = "shares/boards/TQTF";
        break;
    case Model_Ticker::BOND:
        type = "bonds/boards/TQOB";
        break;
    case Model_Ticker::CBOND:
        type = "bonds/boards/TQCB";
        break;
        
    default:
        type = "shares/boards/TQBR";
        break;
    }

    const wxString URL = wxString::Format(
        "https://iss.moex.com/iss/history/engines/stock/markets/%s/securities/%s/candles.xml?from=%s&till=%s&interval=24&start=0"
        , type, m_ticker, dateStr, today);

    auto err_code = http_get_data(URL, m_error);

    if (err_code != CURLE_OK) {
        return false;
    }
    wxString xml = m_error;
    m_error.clear();
    wxXmlDocument doc;

    if (!xml.Contains("</document>"))
        wxLogDebug("%s", "!= </document>");

    wxStringInputStream XmlContentStream(xml);

    if (!doc.Load(XmlContentStream))
        wxLogDebug("%s", "XmlContentStream");

    wxXmlNode *child = doc.GetRoot()->GetChildren()->GetChildren();
    wxString attribute1 = "TRADEDATE";
    wxString attribute2 = "CLOSE";

    wxString result;
    while (child)
    {
        wxLogDebug("%i %s", child->GetType(), child->GetName());

        if (child->GetName() == "rows")
        {
            child = child->GetChildren();
            while (child)
            {
                wxString content = child->GetNodeContent();

                wxString att1 =
                    child->GetAttribute(attribute1, "null");
                wxDateTime lineDate;
                lineDate.ParseDate(att1);
                if (!lineDate.IsValid())
                    continue;

                date = lineDate.GetTicks();
                wxString att2 =
                    child->GetAttribute(attribute2, "null");
                wxLogDebug("att1 = %s | att2 = %s", att1, att2);

                double value;
                if (att2.ToDouble(&value))
                {
                    history[date] = value;
                }

                child = child->GetNext();
            }
            break;
        }

        child = child->GetNext();
    }

    saveData(history);

    return err_code == CURLE_OK;
}

bool mmOnline::mmMorningStar()
{

    const wxString URL = wxString::Format(
        "http://tools.morningstar.es/es/util/searchfundbyname.aspx?ModuleId=69&LanguageId=es-ES&limit=100&q=%s"
        , m_ticker.Lower());

    auto err_code = http_get_data(URL, m_error);

    if (err_code != CURLE_OK) {
        return false;
    }

    wxString fund;
    bool found = false;
    wxStringTokenizer line(m_error, "\n");
    while (line.HasMoreTokens() && !found)
    {
        wxString l = line.GetNextToken();
        wxLogDebug("%s", l);

        size_t pos = 0;
        wxStringTokenizer item(l, "|");
        while (item.HasMoreTokens())
        {
            wxString i = item.GetNextToken();

            switch (pos)
            {
            case 0:
                m_name = i;
                break;
            case 1:
                fund = i;
                break;
            case 2:
                if (i == "STOCK")
                    m_type = Model_Ticker::SHARE;
                else if (i == "FUND")
                    m_type = Model_Ticker::FUND;
                break;
            default:
                break;
            }

            wxLogDebug("%zu - %s", pos, i);

            if (i.CmpNoCase(m_market) == 0) {
                found = true;
                break;
            }
            pos++;
        }
    }

    const wxString today = wxDate::Today().FormatISODate();
    const wxString URL2 = wxString::Format(
        "http://tools.morningstar.es/api/rest.svc/timeseries_price/2nhcdckzon?id=%s&currencyId=%s&idtype=Morningstar&priceType=&frequency=daily&startDate=%s&endDate=%s&outputType=COMPACTJSON"
        , fund, m_currency, m_date, today);

    m_error = "";
    err_code = http_get_data(URL2, m_error);

    wxString json_data = m_error;

    if (err_code == CURLE_OK)
        m_error = "";

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
        m_error = _("Error");
        return false;
    }

    std::map<time_t, float> history;

    const Value& data = json_doc.GetArray();
    for (rapidjson::SizeType i = 0; i < data.Size(); i++)
    {

        const Value& c = data[i];
        if (!c[rapidjson::SizeType(0)].IsInt64())
            continue;
        long long date = c[rapidjson::SizeType(0)].GetInt64() / 1000;
        wxLogDebug("%lld - %s", date, wxDateTime(static_cast<time_t>(date)).FormatISODate());

        if (!c[rapidjson::SizeType(1)].IsFloat())
            continue;
        double price = c[rapidjson::SizeType(1)].GetFloat();
        wxLogDebug("%.2f \n", price);

        history[date] = price;
    }

    saveData(history);

    return err_code == CURLE_OK;
}

void mmOnline::saveData(std::map<time_t, float>& history)
{
    const wxString today = wxDate::Today().FormatISODate();
    Model_StockHistory::instance().Savepoint();
    for (const auto& entry : history)
    {
        float dPrice = entry.second;
        const wxString date_str = wxDateTime(static_cast<time_t>(entry.first)).FormatISODate();

        Model_StockHistory::Data_Set d = Model_StockHistory::instance().find(Model_StockHistory::DATE(entry.first)
            , Model_StockHistory::SYMBOL(m_ticker));
        if (d.empty()) {
            Model_StockHistory::Data* ndata = Model_StockHistory::instance().create();
            ndata->SYMBOL = m_ticker;
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
}
