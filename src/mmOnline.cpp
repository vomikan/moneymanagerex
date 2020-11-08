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

mmHistoryOnline::mmHistoryOnline(Model_Ticker::Data* t, const wxString& currency) :
    m_currency(currency)
{

    m_unique_name = t->UNIQUENAME;
    m_ticker = t->SYMBOL;
    m_type = t->TYPE;
    m_source = t->SOURCE;
    m_market = t->MARKET;

    Model_StockHistory::Data_Set d = Model_StockHistory::instance().find(Model_StockHistory::SYMBOL(m_unique_name));
    std::stable_sort(d.begin(), d.end(), SorterByDATE());
    std::reverse(d.begin(), d.end());

    if (d.empty()) {
        m_date = wxDate::Today().Subtract(wxDateSpan::Week());
    }
    else {
        m_date.ParseISODate(d.back().DATE);
    }
    m_date = m_date.ToTimezone(wxDate::GMT0, true).Add(wxDateSpan::Day());
    wxLogDebug("%s %s", m_date.FormatISODate(), m_date.FormatISOTime());

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

mmHistoryOnline::~mmHistoryOnline()
{

}

bool mmHistoryOnline::mmYahoo()
{
    wxLongLong begin = m_date.GetValue();

    wxString ticker = m_ticker;
    if (!ticker.Contains(".") && !m_market.empty()) {
        ticker = m_ticker + "." + m_market;
    }
    const wxString URL = wxString::Format(
        "https://query1.finance.yahoo.com/v8/finance/chart/%s?period1=%zu&period2=%zu&interval=1d&fields=currency"
        , ticker, m_date.GetTicks(), wxDate::Today().GetTicks());

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
        m_error = "";
        return true;
    }

    return false;
}

bool mmHistoryOnline::mmMOEX()
{
    /*
    shortname="VTBG ETF"
    https://iss.moex.com/iss/securities.xml?q=VTBG
    Get primary_boardid="TQTD" or marketprice_boardid="TQTD" from row for m_market
    */

    const wxString today = wxDate::Today().FormatISODate();
    const wxString dateStr = m_date.FormatISODate();
    std::map<time_t, float> history;

    wxString type = "shares";
    m_market = m_market.empty() ? "TQBR" : m_market;
    switch (m_type)
    {
    case Model_Ticker::BOND:
        type = "bonds";
        m_market = m_market.empty() ? "TQOB" : m_market;
        break;
    case Model_Ticker::FUND:
        m_market = m_market.empty() ? "TQTF" : m_market;
        break;

    default:
        break;
    }

    const wxString URL = wxString::Format(
        "https://iss.moex.com/iss/history/engines/stock/markets/%s/boards/%s/securities/%s/candles.xml?from=%s&till=%s&interval=24&start=0"
        , type, m_market, m_ticker, dateStr, today);

    auto err_code = http_get_data(URL, m_error);

    if (err_code != CURLE_OK) {
        return false;
    }
    wxString xml = m_error;
    m_error.clear();
    wxXmlDocument doc;

    if (!xml.Contains("</document>")) {
        wxLogDebug("%s", "!= </document>");
        m_error = _("Error");
        return false;
    }

    wxStringInputStream XmlContentStream(xml);

    if (!doc.Load(XmlContentStream)) {
        wxLogDebug("%s", "XmlContentStream");
        m_error = _("Error");
        return false;
    }

    const wxString attribute1 = "TRADEDATE";
    const wxString attribute2 = "CLOSE";
    const wxString attribute3 = "FACEVALUE";

    wxXmlNode *child = doc.GetRoot()->GetChildren()->GetChildren();

    size_t date;
    wxString result;
    while (child)
    {
        wxLogDebug("%i %s", child->GetType(), child->GetName());

        if (child->GetName() == "rows")
        {
            child = child->GetChildren();
            while (child)
            {

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

                if (m_type == Model_Ticker::BOND) {
                    wxString att3 =
                        child->GetAttribute(attribute3, "null");

                    if (att3.ToDouble(&value))
                    {
                        history[date] = value / 100 * history[date];
                    }
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

bool mmHistoryOnline::mmMorningStar()
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
    const wxString begin = m_date.FormatISODate();
    const wxString URL2 = wxString::Format(
        "http://tools.morningstar.es/api/rest.svc/timeseries_price/2nhcdckzon?id=%s&currencyId=%s&idtype=Morningstar&priceType=&frequency=daily&startDate=%s&endDate=%s&outputType=COMPACTJSON"
        , fund, m_currency, begin, today);

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

void mmHistoryOnline::saveData(std::map<time_t, float>& history)
{
    const wxString today = wxDate::Today().FormatISODate();
    Model_StockHistory::instance().Savepoint();
    for (const auto& entry : history)
    {
        float dPrice = entry.second;
        const wxString date_str = wxDateTime(static_cast<time_t>(entry.first)).FormatISODate();

        Model_StockHistory::Data_Set d = Model_StockHistory::instance().find(Model_StockHistory::DATE(entry.first)
            , Model_StockHistory::SYMBOL(m_unique_name));
        if (d.empty()) {
            Model_StockHistory::Data* ndata = Model_StockHistory::instance().create();
            ndata->SYMBOL = m_unique_name;
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

// ----------------------------------------------------------

mmWebPage::~mmWebPage()
{
}

mmWebPage::mmWebPage(const wxString & name)
    : m_unique_name(name)
{
    Model_Ticker::Data* d = Model_Ticker::instance().get(name);
    if (d)
    {
        m_market = d->MARKET;

        wxString httpString = wxString::Format(d->WEBPAGE, d->SYMBOL);
        if (!httpString.empty())
        {
            wxLaunchDefaultBrowser(httpString);
            return;
        }

        switch (d->SOURCE)
        {
        case Model_Ticker::MOEX:
            mmMOEX(d);
            break;

        case Model_Ticker::MS:
            mmMorningStar(d);
            break;

        default:
            mmYahoo(d);
            break;
        }
    }
    else
    {

    }

}

bool mmWebPage::mmYahoo(Model_Ticker::Data* d)
{
    // "https://www.google.com/search?q=%s"
    // "https://www.marketwatch.com/investing/stock/%s"
    // "https://www.ifcmarkets.co.in/en/market-data/stocks-prices/%s"

    // Will display the stock page when using Looks up the current value
    const wxString stockURL = "http://finance.yahoo.com/echarts?s=%s";

    wxString ticker = d->SYMBOL;
    if (!ticker.Contains(".") && !m_market.empty()) {
        ticker = ticker + "." + m_market;
    }

    //const wxString& stockURL = Model_Infotable::instance().GetStringInfo("STOCKURL", mmex::weblink::DefStockUrl);
    const wxString& httpString = wxString::Format(stockURL, ticker);
    wxLaunchDefaultBrowser(httpString);
    return true;

}

bool mmWebPage::mmMOEX(Model_Ticker::Data* d)
{
    const wxString stockURL = "https://www.moex.com/ru/issue.aspx?code=%s";

    const wxString& httpString = wxString::Format(stockURL, d->SYMBOL);
    wxLaunchDefaultBrowser(httpString);
    return true;

}

bool mmWebPage::mmMorningStar(Model_Ticker::Data* d)
{

    const wxString stockURL = "http://quotes.morningstar.com/stockq/c-header?&t=%s";
    const wxString& httpString = wxString::Format(stockURL, d->SYMBOL);
    wxLaunchDefaultBrowser(httpString);
    return true;

}

