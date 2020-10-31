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

class mmOnline
{
public:
    mmOnline(const wxString& ticker, const wxString& market ="", const wxString& currency = "USD", int source = 0, int type  = 0);
    //mmOnline(Model_Ticker::Data* t, const wxString& currency = "USD");
    ~mmOnline();
    bool mmYahoo();
    bool mmMOEX();
    bool mmMorningStar();
    const wxString getError() const;

private:
    wxString m_ticker;
    wxString m_market;
    wxString m_date;
    wxString m_error;
    wxString m_name;
    wxString m_currency;
    int m_source;
    int m_type;

    void saveData(std::map<time_t, float>& history);
};

inline const wxString mmOnline::getError() const { return m_error; }
