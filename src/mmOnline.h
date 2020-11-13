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
#include "option.h"
#include <wx/string.h>
#include "model/Model_Ticker.h"
#include "model/Model_Currency.h"

class Model_Ticker;

class mmOnline
{
public:
    mmOnline();
    ~mmOnline();

    int getOnlineRatesYahoo();
    int getOnlineRatesMOEX();
    int getOnlineRatesMS();

    int get_error_code() const;
    const wxString get_error_str() const;
private:
    wxString m_error_str;
    int m_error_code;

};

inline int mmOnline::get_error_code() const { return m_error_code; }
inline const wxString mmOnline::get_error_str() const { return m_error_str; }

class mmHistoryOnline
{
public:
    mmHistoryOnline(Model_Ticker::Data* t, int currency_id);
    mmHistoryOnline(Model_Currency::Data* currency);
    ~mmHistoryOnline();
    const wxString getError() const;
    const int get_error_code() const;

private:
    int mmYahoo();
    int mmMOEX();
    int mmMorningStar();

    int m_ticker_id;
    wxString m_ticker;
    wxString m_market;
    wxString m_currency_symbol;
    wxDateTime m_date;
    wxString m_error_str;
    int m_error_code;
    wxString m_name;
    int m_source;
    int m_type;

    void saveStockHistoryData(std::map<time_t, float>& history);
    void saveCurrencyHistoryData(std::map<time_t, float>& history);
};

inline const wxString mmHistoryOnline::getError() const { return m_error_str; }
inline const int mmHistoryOnline::get_error_code() const { return m_error_code; }

class mmWebPage
{
public:
    mmWebPage(int id);
    ~mmWebPage();
private:
    bool mmYahoo(Model_Ticker::Data* d);
    bool mmMOEX(Model_Ticker::Data* d);
    bool mmMorningStar(Model_Ticker::Data* d);

    int m_ticker_id;
    wxString m_market;
    int m_source;
    int m_type;
};

extern const wxString YahooQuotes;
extern const wxString YahooQuotesHistory;

//enum yahoo_price_type { FIAT = 0, SHARES };


bool getOnlineCurrencyRates(wxString& msg, int curr_id = -1, bool used_only = true);
bool get_yahoo_prices(std::map<wxString, double>& symbols
    , std::map<wxString, double>& out
    , wxString& output);

