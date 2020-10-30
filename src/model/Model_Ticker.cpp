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

#include "Model_Ticker.h"
//#include "Model_Stock.h"

const std::vector<std::pair<Model_Ticker::SOURCE_ENUM, wxString> > Model_Ticker::SOURCE_CHOICES =
{
    {Model_Ticker::YAHOO, wxString(wxTRANSLATE("Yahoo"))},
    {Model_Ticker::MS, wxString(wxTRANSLATE("Morningstar"))},
    {Model_Ticker::MOEX, wxString(wxTRANSLATE("Moscow Exchange"))}
};

const std::vector<std::pair<Model_Ticker::TYPE_ENUM, wxString> > Model_Ticker::TYPE_CHOICES =
{
    {Model_Ticker::SHARE, wxString(wxTRANSLATE("Stock"))},
    {Model_Ticker::FUND, wxString(wxTRANSLATE("Fund"))},
    {Model_Ticker::BOND, wxString(wxTRANSLATE("Bond"))},

};

Model_Ticker::Model_Ticker()
: Model<DB_Table_TICKERPROPERTIES_V1>()
{
}

Model_Ticker::~Model_Ticker()
{
}

/**
* Initialize the global Model_Ticker table.
* Reset the Model_Ticker table or create the table if it does not exist.
*/
Model_Ticker& Model_Ticker::instance(wxSQLite3Database* db)
{
    Model_Ticker& ins = Singleton<Model_Ticker>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);
    ins.preload();

    return ins;
}

/** Return the static instance of Model_Ticker table */
Model_Ticker& Model_Ticker::instance()
{
    return Singleton<Model_Ticker>::instance();
}

wxArrayString Model_Ticker::all_sectors()
{
    wxArrayString sector;
    sector.Add(wxTRANSLATE("Basic Materials"));
    sector.Add(wxTRANSLATE("Consumer Cyclical"));
    sector.Add(wxTRANSLATE("Financial Services"));
    sector.Add(wxTRANSLATE("Real Estate"));
    sector.Add(wxTRANSLATE("Consumer Defensive"));
    sector.Add(wxTRANSLATE("Healthcare"));
    sector.Add(wxTRANSLATE("Utilities"));
    sector.Add(wxTRANSLATE("Communication Services"));
    sector.Add(wxTRANSLATE("Energy"));
    sector.Add(wxTRANSLATE("Industrials"));
    sector.Add(wxTRANSLATE("Technology"));
    sector.Add(wxTRANSLATE("Other"));
    return sector;
}


wxArrayString Model_Ticker::all_sources()
{
    wxArrayString source;
    for (const auto& item : SOURCE_CHOICES) source.Add(item.second);
    return source;
}

wxArrayString Model_Ticker::all_type()
{
    wxArrayString type;
    for (const auto& item : TYPE_CHOICES) type.Add(item.second);
    return type;
}

wxString Model_Ticker::get_ticker_name(int ticker_id)
{
    Data* ticker = instance().get(ticker_id);
    if (ticker)
        return ticker->NAME;
    else
        return "";
}

/** Remove the Data record instance from memory and the database. */
bool Model_Ticker::remove(int id)
{
    this->Savepoint();
    for (const auto& r: Model_Ticker::instance().find(Model_Ticker::TICKERID(id)))
    {
        Model_Ticker::instance().remove(r.TICKERID);
    }
    this->ReleaseSavepoint();

    return this->remove(id, db_);
}

Model_Ticker::SOURCE_ENUM Model_Ticker::source(const Data& ticker)
{
    return static_cast<SOURCE_ENUM>(ticker.SOURCE);
}

Model_Ticker::SOURCE_ENUM Model_Ticker::source(const Data* ticker)
{
    return static_cast<SOURCE_ENUM>(ticker->SOURCE);
}


DB_Table_TICKERPROPERTIES_V1::SOURCE SOURCE(Model_Ticker::SOURCE_ENUM source, OP op)
{
    return DB_Table_TICKERPROPERTIES_V1::SOURCE(source, op);
}

Model_Ticker::TYPE_ENUM Model_Ticker::type(const Data* ticker)
{
    return static_cast<TYPE_ENUM>(ticker->TYPE);
}

Model_Ticker::TYPE_ENUM Model_Ticker::type(const Data& ticker)
{
    return type(&ticker);
}

Model_Ticker::Data* Model_Ticker::get(const wxString& symbol)
{
    Data* ticker = this->get_one(SYMBOL(symbol));
    if (ticker) return ticker;

    Data_Set items = this->find(SYMBOL(symbol));
    if (!items.empty()) ticker = this->get(items[0].TICKERID, this->db_);
    return ticker;
}

