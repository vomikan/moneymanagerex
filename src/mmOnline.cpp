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
#include "model/Model_Stock.h"
#include "model/Model_StockHistory.h"
#include "model/Model_Account.h"
#include "model/Model_Currency.h"
#include "model/Model_CurrencyHistory.h"

#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

#include <vector>
#include <wx/sstream.h>
#include <wx/xml/xml.h>


mmOnline::mmOnline()
{


    getOnlineRatesYahoo();
    getOnlineRatesMOEX();
    getOnlineRatesMS();


}

mmOnline::~mmOnline()
{
}

int mmOnline::getOnlineRatesMOEX()
{

    wxString url_template = R"(https://iss.moex.com/iss/engines/stock/markets/%s/boards/%s/securities.xml?iss.meta=off&iss.only=securities&securities.columns=SECID,PREVADMITTEDQUOTE,FACEVALUE)";

    std::map<std::pair<wxString, int>, wxString> markets;

    Model_Ticker::Data_Set t = Model_Ticker::instance().all();
    for (const auto &ticker : t)
    {

        switch (ticker.SOURCE) {
        case Model_Ticker::MOEX:
            {
                wxString type = "shares";
                wxString market = ticker.MARKET.empty() ? "TQBR" : ticker.MARKET;
                switch (ticker.TYPE)
                {
                case Model_Ticker::FUND:
                    market = ticker.MARKET.empty() ? "TQTF" : ticker.MARKET;
                    break;
                case Model_Ticker::BOND:
                    type = "bonds";
                    market = ticker.MARKET.empty() ? "TQOB" : ticker.MARKET;
                    break;
                default:
                    break;
                }

                auto URL = wxString::Format(url_template, type, market);
                markets[std::make_pair(market, ticker.TYPE)] = URL;
            }
        }
    }

    std::map<wxString, double> prices;
    std::map<wxString, wxString> names;

    for (const auto& item : markets)
    {
        wxString xml;
        m_error_code = http_get_data(wxString(item.second), xml);
        if (m_error_code != CURLE_OK)
        {
            m_error_str = xml;
            return m_error_code;
        }

        wxXmlDocument doc;

        if (!xml.Contains("</document>")) {
            wxLogDebug("%s", "!= </document>");
            m_error_str = _("Error");
            m_error_code = CURLE_GOT_NOTHING;
            return m_error_code;
        }

        wxStringInputStream XmlContentStream(xml);

        if (!doc.Load(XmlContentStream)) {
            wxLogDebug("%s", "XmlContentStream");
            m_error_str = _("Error");
            m_error_code = CURLE_GOT_NOTHING;
            return m_error_code;
        }


        const wxString attribute1 = "SECID";
        const wxString attribute2 = "PREVADMITTEDQUOTE";
        const wxString attribute3 = "FACEVALUE";

        wxXmlNode *child = doc.GetRoot()->GetChildren()->GetChildren();

        wxDateTime today = wxDateTime::Today();
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

                    Model_Ticker::Data_Set tickers = Model_Ticker::instance()
                        .find(Model_Ticker::MARKET(wxString(item.first.first))
                            , Model_Ticker::SYMBOL(att1));
                    if (tickers.size() == 1)
                    {
                        wxString att2 =
                            child->GetAttribute(attribute2, "null");

                        double value;
                        if (att2.ToDouble(&value))
                        {
                            prices[att1] = value;
                            if (item.first.second == Model_Ticker::BOND) {
                                wxString att3 =
                                    child->GetAttribute(attribute3, "null");
                                if (att3.ToDouble(&value))
                                {
                                    prices[att1] *= value / 100.0;
                                }
                            }
                        }
                        else {
                            continue;
                        }
                        wxLogDebug("%s | %.4f", att1, prices[att1]);

                        Model_StockHistory::Data_Set d = Model_StockHistory::instance().find(Model_StockHistory::DATE(today)
                            , Model_StockHistory::TICKERID(tickers.begin()->TICKERID));
                        if (d.empty()) {
                            Model_StockHistory::Data* ndata = Model_StockHistory::instance().create();
                            ndata->TICKERID = tickers.begin()->TICKERID;
                            ndata->DATE = today.FormatISODate();
                            ndata->VALUE = prices[att1];
                            ndata->UPDTYPE = Model_StockHistory::CURRENT;
                            Model_StockHistory::instance().save(ndata);
                        }
                        else {
                            if (d.begin()->UPDTYPE != Model_StockHistory::MANUAL)
                            {
                                Model_StockHistory::Data* ndata = Model_StockHistory::instance().get(d.begin()->HISTID);
                                ndata->VALUE = prices[att1];
                                ndata->UPDTYPE = Model_StockHistory::CURRENT;
                                Model_StockHistory::instance().save(ndata);
                            }
                        }

                    }
                    child = child->GetNext();
                }
            }
            break;
        }

        //child = child->GetNext();
    }



    m_error_code = 0;
    return get_error_code();
}

int mmOnline::getOnlineRatesMS()
{
    wxLogDebug("MorningStar TBD");
    m_error_code = 0;
    return m_error_code;
}


int mmOnline::getOnlineRatesYahoo()
{

    std::map<wxString, double> prices;
    std::map<int, wxString> tickers;

    Model_Ticker::Data_Set t = Model_Ticker::instance().all();
    if (t.empty()) {
        m_error_str = _("Empty value");
        m_error_code = wxID_NO;
        return m_error_code;
    }
        
    for (const auto &ticker : t)
    {
        wxASSERT(!ticker.SYMBOL.empty());
        wxString symbol;
        switch (ticker.SOURCE) {
        case Model_Ticker::YAHOO:
            symbol = ticker.SYMBOL;
            if (!symbol.Contains(".") && !ticker.MARKET.empty()) {
                symbol += "." + ticker.MARKET;
            }
            prices[symbol] = 0.0;
            tickers[ticker.TICKERID] = symbol;

        }
    }

    wxString buffer;
    for (const auto& entry : prices)
    {
        buffer += entry.first + ",";
    }

    if (buffer.Right(1).Contains(",")) {
        buffer.RemoveLast(1);
    }

    const auto URL = wxString::Format(YahooQuotes, buffer);

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    if (err_code != CURLE_OK)
    {
        m_error_str = json_data;
        m_error_code = err_code;
        return m_error_code;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
        m_error_str = _("JSON Parse Error");
        m_error_code = err_code;
        return m_error_code;
    }

    /*
    {"finance":{"result":null,"error":{"code":"Bad Request","description":"Missing required query parameter=symbols"}}}

    */



    if (json_doc.HasMember("finance") && json_doc["finance"].IsObject()) {
        Value e = json_doc["finance"].GetObject();
        if (e.HasMember("error") && e["error"].IsObject()) {
            Value err = e["error"].GetObject();
            if (err.HasMember("description") && err["description"].IsString()) {
                m_error_code = 1;
                m_error_str = wxString::FromUTF8(err["description"].GetString());
                return m_error_code;
            }
        }
    }



    Value q;
    if (json_doc.HasMember("quoteResponse"))
        q = json_doc["quoteResponse"].GetObject();

    Value r;
    if (q.HasMember("result") && q["result"].IsArray())
        r = q["result"].GetArray();
    else {
        m_error_str = _("JSON Parse Error");
        m_error_code = err_code;
        return m_error_code;
    }

    if (r.Empty()) {
        m_error_str = _("Nothing to update");
        m_error_code = err_code;
        return m_error_code;
    }

    for (rapidjson::SizeType i = 0; i < r.Size(); i++)
    {
        if (!r[i].IsObject()) continue;
        Value v = r[i].GetObject();

        if (!v.HasMember("symbol") || !v["symbol"].IsString())
            continue;
        const auto symbol = wxString::FromUTF8(v["symbol"].GetString());

        if (!v.HasMember("currency") || !v["currency"].IsString())
            continue;

        float price = 0.0;

        if (!v.HasMember("marketState") || !v["marketState"].IsString())
            continue;
        wxString marketState = wxString::FromUTF8(v["marketState"].GetString());

        if (marketState == "PRE")
        {
            if (!v.HasMember("preMarketPrice") || !v["preMarketPrice"].IsFloat())
                continue;
            price = v["preMarketPrice"].GetFloat();
        }
        else
        {
            if (!v.HasMember("regularMarketPrice") || !v["regularMarketPrice"].IsFloat())
                continue;
            price = v["regularMarketPrice"].GetFloat();
        }

        const auto currency = wxString::FromUTF8(v["currency"].GetString());
        double k = currency == "GBp" ? 100 : 1;

        wxLogDebug("item: %u %s %f", i, symbol, price);
        prices[symbol] = price <= 0 ? 0 : price / k;
    }

    wxString  msg;

    Model_StockHistory::instance().Savepoint();
    wxDateTime now = wxDate::Now();
    for (auto &ti : tickers)
    {

        double dPrice = prices.find(ti.second) != prices.end() ? prices.at(ti.second) : 0.0;

        if (dPrice != 0.0)
        {
            msg += wxString::Format("%i\t: %0.6f \n", ti.first, dPrice);
            Model_StockHistory::instance().addUpdate(ti.first
                , now, dPrice, Model_StockHistory::CURRENT);
        }
    }
    Model_StockHistory::instance().ReleaseSavepoint();

    m_error_code = 0;
    return m_error_code;
}


mmHistoryOnline::mmHistoryOnline(Model_Currency::Data* currency)
{
    wxString base_currency_symbol;
    Model_Currency::GetBaseCurrencySymbol(base_currency_symbol);
    wxString symbol = wxString::Format("%s%s=X", currency->CURRENCY_SYMBOL, base_currency_symbol);

    m_ticker = symbol;
    m_type = Model_Ticker::MONEY;
    m_source = Model_Ticker::YAHOO;
    m_market = "";
    m_currency_symbol = currency->CURRENCY_SYMBOL;

    Model_CurrencyHistory::Data_Set d = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRENCYID(currency->CURRENCYID));
    std::stable_sort(d.begin(), d.end(), SorterByCURRDATE());
    std::reverse(d.begin(), d.end());

    if (d.empty()) {
        m_date = wxDate::Today().Subtract(wxDateSpan::Week());
    }
    else {
        m_date.ParseISODate(d.back().CURRDATE);
    }
    m_date = m_date.ToTimezone(wxDate::GMT0, true).Add(wxDateSpan::Day());
    wxLogDebug("%s %s", m_date.FormatISODate(), m_date.FormatISOTime());

    mmYahoo();
}

mmHistoryOnline::mmHistoryOnline(Model_Ticker::Data* t, int currency_id)
{
    Model_Currency::Data* curr = Model_Currency::instance().get(currency_id);

    m_ticker_id = t->TICKERID;
    m_currency_symbol = curr ? curr->CURRENCY_SYMBOL : "USD";
    m_ticker = t->SYMBOL;
    m_type = t->TYPE;
    m_source = t->SOURCE;
    m_market = t->MARKET;

    Model_StockHistory::Data_Set d = Model_StockHistory::instance().find(Model_StockHistory::TICKERID(t->TICKERID));
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

mmHistoryOnline::~mmHistoryOnline()
{

}

int mmHistoryOnline::mmYahoo()
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
    m_error_code = http_get_data(URL, json_data);
    m_error_str = json_data;

    if (m_error_code != CURLE_OK) {
        return m_error_code;
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
                m_error_str = wxString::Format("%s - %s", code, description);
                m_error_code = 1;
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

        if (m_type != Model_Ticker::MONEY) {
            saveStockHistoryData(history);
        }
        else {
            saveCurrencyHistoryData(history);
        }

        m_error_str = "";
        return 0;
    }

    return 1;
}

int mmHistoryOnline::mmMOEX()
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

    m_error_code = http_get_data(URL, m_error_str);

    if (m_error_code != CURLE_OK) {
        return m_error_code;
    }
    wxString xml = m_error_str;
    m_error_str.clear();
    wxXmlDocument doc;

    if (!xml.Contains("</document>")) {
        wxLogDebug("%s", "!= </document>");
        m_error_str = _("Error");
        m_error_code = 1;
        return m_error_code;
    }

    wxStringInputStream XmlContentStream(xml);

    if (!doc.Load(XmlContentStream)) {
        wxLogDebug("%s", "XmlContentStream");
        m_error_str = _("Error");
        m_error_code = 1;
        return m_error_code;
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

    saveStockHistoryData(history);

    return m_error_code;
}

int mmHistoryOnline::mmMorningStar()
{

    const wxString URL = wxString::Format(
        "http://tools.morningstar.es/es/util/searchfundbyname.aspx?ModuleId=69&LanguageId=es-ES&limit=100&q=%s"
        , m_ticker.Lower());

    m_error_code = http_get_data(URL, m_error_str);

    if (m_error_code != CURLE_OK) {
        return m_error_code;
    }

    wxString fund;
    bool found = false;
    wxStringTokenizer line(m_error_str, "\n");
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
        , fund, m_currency_symbol, begin, today);

    m_error_str = "";
    m_error_code = http_get_data(URL2, m_error_str);

    wxString json_data = m_error_str;

    if (m_error_code == CURLE_OK)
        m_error_str = "";

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError()) {
        m_error_str = _("Error");
        m_error_code = 1;
        return m_error_code;
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

    saveStockHistoryData(history);

    return m_error_code;
}

void mmHistoryOnline::saveCurrencyHistoryData(std::map<time_t, float>& history)
{
    const wxString today = wxDate::Today().FormatISODate();
    Model_Currency::Data_Set cs = Model_Currency::instance().find(Model_Currency::CURRENCY_SYMBOL(m_currency_symbol));
    int currencyID = -1;
    if (!cs.empty()) currencyID = cs.begin()->CURRENCYID;

    Model_Currency::Data* c = Model_Currency::instance().get(currencyID);
    if (c) currencyID = c->CURRENCYID;

    Model_CurrencyHistory::instance().Savepoint();
    for (const auto& entry : history)
    {
        const wxString date_str = wxDateTime(static_cast<time_t>(entry.first)).FormatISODate();
        float dPrice = entry.second;

        Model_CurrencyHistory::Data_Set d = Model_CurrencyHistory::instance().find(Model_CurrencyHistory::CURRDATE(entry.first)
            , Model_CurrencyHistory::CURRENCYID(currencyID));
        if (d.empty()) {
            Model_CurrencyHistory::Data* ndata = Model_CurrencyHistory::instance().create();
            ndata->CURRENCYID = currencyID;
            ndata->CURRDATE = date_str;
            ndata->CURRVALUE = dPrice;
            ndata->CURRUPDTYPE = date_str == today ? Model_CurrencyHistory::CURRENT : Model_CurrencyHistory::ONLINE;
            Model_CurrencyHistory::instance().save(ndata);
        }
        else {
            if (d.begin()->CURRUPDTYPE != Model_CurrencyHistory::MANUAL)
            {
                Model_CurrencyHistory::Data* ndata = Model_CurrencyHistory::instance().get(d.begin()->CURRHISTID);
                ndata->CURRVALUE = dPrice;
                ndata->CURRUPDTYPE = date_str == today ? Model_CurrencyHistory::CURRENT : Model_CurrencyHistory::ONLINE;
                Model_CurrencyHistory::instance().save(ndata);
            }
        }

    }

    Model_CurrencyHistory::instance().ReleaseSavepoint();
}

void mmHistoryOnline::saveStockHistoryData(std::map<time_t, float>& history)
{
    const wxString today = wxDate::Today().FormatISODate();
    Model_StockHistory::instance().Savepoint();
    for (const auto& entry : history)
    {
        float dPrice = entry.second;
        const wxString date_str = wxDateTime(static_cast<time_t>(entry.first)).FormatISODate();

        Model_StockHistory::Data_Set d = Model_StockHistory::instance().find(Model_StockHistory::DATE(entry.first)
            , Model_StockHistory::TICKERID(m_ticker_id));
        if (d.empty()) {
            Model_StockHistory::Data* ndata = Model_StockHistory::instance().create();
            ndata->TICKERID = m_ticker_id;
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

mmWebPage::mmWebPage(int id)
    : m_ticker_id(id)
{
    Model_Ticker::Data* d = Model_Ticker::instance().get(id);
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

//--------------------------------------------------------------------

bool getOnlineCurrencyRates(wxString& msg, int curr_id, bool used_only)
{
    wxString base_currency_symbol;

    if (!Model_Currency::GetBaseCurrencySymbol(base_currency_symbol))
    {
        msg = _("Could not find base currency symbol!");
        return false;
    }

    std::map<wxString, double> fiat;
    const wxDateTime today = wxDateTime::Today();
    const wxString today_str = today.FormatISODate();

    auto currencies = Model_Currency::instance().find(Model_Currency::CURRENCY_SYMBOL(base_currency_symbol, NOT_EQUAL));
    for (const auto& currency : currencies)
    {
        if (curr_id > 0 && currency.CURRENCYID != curr_id)
            continue;
        if (curr_id < 0 && !Model_Account::is_used(currency))
            continue;
        const auto symbol = currency.CURRENCY_SYMBOL;
        if (symbol.IsEmpty())
            continue;

        fiat[symbol] = Model_CurrencyHistory::getDayRate(currency.CURRENCYID, today_str);
    }

    if (fiat.empty())
    {
        msg = _("Nothing to update");
        return false;
    }

    wxString output;
    std::map<wxString, double> currency_data;

    if (get_yahoo_prices(fiat, currency_data, output))
    {

        msg << _("Currency rates have been updated");
        msg << "\n\n";
        for (const auto & item : fiat)
        {
            auto value0 = item.second;
            if (currency_data.find(item.first) != currency_data.end())
            {
                auto value1 = currency_data[item.first];
                msg << wxString::Format("%s %f -> %f\n", item.first, value0, value1);
            }
            else
            {
                msg << wxString::Format("%s %f -> %s\n", item.first, value0, _("Invalid value"));
            }
        }
    }
    else
    {
        msg = output;
    }

    Model_Currency::instance().Savepoint();
    Model_CurrencyHistory::instance().Savepoint();
    for (auto& currency : currencies)
    {
        if (!used_only && !Model_Account::is_used(currency)) continue;

        const wxString currency_symbol = currency.CURRENCY_SYMBOL;
        if (!currency_symbol.IsEmpty() && currency_data.find(currency_symbol) != currency_data.end())
        {
            double new_rate = currency_data[currency_symbol];
            if (new_rate > 0)
            {
                if (Option::instance().getCurrencyHistoryEnabled())
                    Model_CurrencyHistory::instance().addUpdate(currency.CURRENCYID
                        , today, new_rate, Model_CurrencyHistory::CURRENT);
                else
                {
                    currency.BASECONVRATE = new_rate;
                    Model_Currency::instance().save(&currency);
                }
            }
        }
    }
    Model_Currency::instance().ReleaseSavepoint();
    Model_CurrencyHistory::instance().ReleaseSavepoint();

    return true;
}

/* Currencies & stock prices */

bool get_yahoo_prices(std::map<wxString, double>& symbols
    , std::map<wxString, double>& out
    , wxString& output)
{
    wxString base_currency_symbol;
    Model_Currency::GetBaseCurrencySymbol(base_currency_symbol);

    wxString buffer;
    for (const auto& entry : symbols)
    {
        buffer += wxString::Format("%s%s=X,", entry.first, base_currency_symbol);
    }

    if (buffer.Right(1).Contains(",")) {
        buffer.RemoveLast(1);
    }

    const auto URL = wxString::Format(YahooQuotes, buffer);

    wxString json_data;
    auto err_code = http_get_data(URL, json_data);
    if (err_code != CURLE_OK)
    {
        output = json_data;
        return false;
    }

    Document json_doc;
    if (json_doc.Parse(json_data.utf8_str()).HasParseError())
        return false;


    Value r = json_doc["quoteResponse"].GetObject();

    Value e = r["result"].GetArray();

    if (e.Empty()) {
        output = _("Nothing to update");
        return false;
    }


    wxRegEx pattern("^(...)...=X$");
    for (rapidjson::SizeType i = 0; i < e.Size(); i++)
    {
        if (!e[i].IsObject()) continue;
        Value v = e[i].GetObject();

        if (!v.HasMember("symbol") || !v["symbol"].IsString())
            continue;
        auto currency_symbol = wxString::FromUTF8(v["symbol"].GetString());
        if (pattern.Matches(currency_symbol))
        {
            if (!v.HasMember("regularMarketPrice") || !v["regularMarketPrice"].IsFloat())
                continue;
            const auto price = v["regularMarketPrice"].GetFloat();
            currency_symbol = pattern.GetMatch(currency_symbol, 1);

            wxLogDebug("item: %u %s %f", i, currency_symbol, price);
            out[currency_symbol] = (price <= 0 ? 0 : price);
        }
    }


    return true;
}


// Yahoo API
const wxString YahooQuotes = "https://query1.finance.yahoo.com/v7/finance/quote?symbols=%s&fields=preMarketPrice,regularMarketPrice,currency,shortName,marketState";

const wxString YahooQuotesHistory = "https://query1.finance.yahoo.com/v8/finance/chart/%s?range=%s&interval=%s&fields=currency";
