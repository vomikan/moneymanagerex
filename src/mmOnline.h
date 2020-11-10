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

class Model_Ticker;

class mmHistoryOnline
{
public:
    //mmHistoryOnline(const wxString& unique_name, const wxString& market ="", const wxString& currency = "USD", int source = 0, int type  = 0);
    mmHistoryOnline(Model_Ticker::Data* t, const wxString& currency = "USD");
    ~mmHistoryOnline();
    const wxString getError() const;

private:
    bool mmYahoo();
    bool mmMOEX();
    bool mmMorningStar();

    wxString m_unique_name;
    wxString m_ticker;
    wxString m_market;
    wxDateTime m_date;
    wxString m_error;
    wxString m_name;
    wxString m_currency;
    int m_source;
    int m_type;

    void saveData(std::map<time_t, float>& history);
};

inline const wxString mmHistoryOnline::getError() const { return m_error; }

class mmWebPage
{
public:
    mmWebPage(const wxString& ticker);
    ~mmWebPage();
private:
    bool mmYahoo(Model_Ticker::Data* d);
    bool mmMOEX(Model_Ticker::Data* d);
    bool mmMorningStar(Model_Ticker::Data* d);

    wxString m_unique_name;
    wxString m_market;
    int m_source;
    int m_type;
};

extern const wxString YahooQuotes;
extern const wxString YahooQuotesHistory;

bool getOnlineHistory(std::map<wxDateTime, double> &historical_rates, const wxString &symbol, wxString &msg);
enum yahoo_price_type { FIAT = 0, SHARES };
bool getOnlineRates(wxString& msg);
bool getOnlineCurrencyRates(wxString& msg, int curr_id = -1, bool used_only = true);
bool get_yahoo_prices(std::map<wxString, double>& symbols
    , std::map<wxString, double>& out
    , const wxString base_currency_symbol
    , wxString& output
    , int type);

