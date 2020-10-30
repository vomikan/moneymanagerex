/*******************************************************
 Copyright (C) 2020 Nikolay Akimmov

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

#ifndef MODEL_TICKER_H
#define MODEL_TICKER_H

#include "Model.h"
#include "db/DB_Table_Tickerproperties_V1.h"

class Model_Ticker : public Model<DB_Table_TICKERPROPERTIES_V1>
{
public:
    using Model<DB_Table_TICKERPROPERTIES_V1>::remove;
    using Model<DB_Table_TICKERPROPERTIES_V1>::get;

    enum SOURCE_ENUM { YAHOO = 0, MS, MOEX };
    enum TYPE_ENUM { SHARE = 0, FUND, BOND };

    static const std::vector<std::pair<SOURCE_ENUM, wxString> > SOURCE_CHOICES;
    static const std::vector<std::pair<TYPE_ENUM, wxString> > TYPE_CHOICES;

public:
    Model_Ticker();
    ~Model_Ticker();

public:

    static Model_Ticker& instance(wxSQLite3Database* db);
    static Model_Ticker& instance();

public:

    Data* get(const wxString& name);
    static wxString get_ticker_name(int account_id);
    bool remove(int id);

public:

    static wxArrayString all_sources();
    static wxArrayString all_type();
    static wxArrayString all_sectors();

    static TYPE_ENUM type(const Data* ticker);
    static TYPE_ENUM type(const Data& ticker);

    static SOURCE_ENUM source(const Data* ticker);
    static SOURCE_ENUM source(const Data& ticker);

};

#endif // 
