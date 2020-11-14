/*******************************************************
 Copyright (C) 2013,2014 Guan Lisheng (guanlisheng@gmail.com)
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

#include "Model_Stock.h"
#include "Model_StockHistory.h"
#include "Model_Ticker.h"

Model_Stock::Model_Stock()
: Model<DB_Table_STOCK_V1>()
{
}

Model_Stock::~Model_Stock()
{
}

/**
* Initialize the global Model_Stock table.
* Reset the Model_Stock table or create the table if it does not exist.
*/
Model_Stock& Model_Stock::instance(wxSQLite3Database* db)
{
    Model_Stock& ins = Singleton<Model_Stock>::instance();
    ins.db_ = db;
    ins.destroy_cache();
    ins.ensure(db);

    return ins;
}

/** Return the static instance of Model_Stock table */
Model_Stock& Model_Stock::instance()
{
    return Singleton<Model_Stock>::instance();
}

wxDate Model_Stock::PURCHASEDATE(const Data* stock)
{
    return Model::to_date(stock->PURCHASEDATE);
}

wxDate Model_Stock::PURCHASEDATE(const Data& stock)
{
    return Model::to_date(stock.PURCHASEDATE);
}

/** Original value of Stocks */
double Model_Stock::InvestmentValue(const Data* r)
{
    return 0.0; //TODO: //r->VALUE;
}

/** Original value of Stocks */
double Model_Stock::InvestmentValue(const Data& r)
{
    return InvestmentValue(&r);
}

double Model_Stock::CurrentValue(const Data* r)
{
    return r->NUMSHARES * 1.0; //TODO: // r->CURRENTPRICE;
}

double Model_Stock::CurrentValue(const Data& r)
{
    return CurrentValue(&r);
}

wxString Model_Stock::get_symbol()
{
    return "TBD";
}

/**
* Remove the Data record from memory and the database.
* Delete also all stock history
*/
bool Model_Stock::remove(int id)
{
    Model_Stock::Data *data = this->get(id);
    const auto &stocks = Model_Stock::instance().find(Model_Stock::TICKERID(data->TICKERID));
    if (stocks.size() == 1)
    {
        this->Savepoint(); //TODO:
        //for (const auto& r : Model_StockHistory::instance().find(Model_StockHistory::TICKERID(data->SYMBOL)))
        //    Model_StockHistory::instance().remove(r.id());
        this->ReleaseSavepoint();
    }

    return this->remove(id, db_);
}

// ------------------------------------------

Model_StockStat::~Model_StockStat() {}

Model_StockStat::Model_StockStat(int ticker_id, int accountID, double current_price)
{

    Model_Account::Data* a = Model_Account::instance().get(accountID);
    Model_Ticker::Data* t = Model_Ticker::instance().get(ticker_id);
    Model_Stock::Data_Set s = Model_Stock::instance().find(Model_Stock::HELDAT(a ? a->ACCOUNTID : -1)
        , Model_Stock::TICKERID(t ? t->TICKERID : -1));

    bool marginal = false;
    m_purchase_total = 0.0;
    m_money_total = 0.0;
    m_everage_price = 0.0;
    m_commission = 0.0;
    m_init_date = wxDate::Today().FormatISODate();

    std::vector<double> shares;

    for (const auto& item : s)
    {
        if (m_init_date > item.PURCHASEDATE) {
            m_init_date = item.PURCHASEDATE;
        }
        for (int i = 0; i < abs(item.NUMSHARES); i++)
        {
            marginal = !shares.empty() ? shares[0] < 0.0 : item.NUMSHARES < 0;
            if (!marginal && item.NUMSHARES > 0) {
                shares.push_back(item.PURCHASEPRICE);
                m_purchase_total += item.PURCHASEPRICE;
            }
            else if (marginal && item.NUMSHARES < 0) {
                shares.push_back(-item.PURCHASEPRICE);
                m_purchase_total += -item.PURCHASEPRICE;
            }
            else if (!marginal && item.NUMSHARES < 0)
            {
                m_purchase_total += -item.PURCHASEPRICE; //z тут добавил минус для SBER
                m_money_total += item.PURCHASEPRICE;
                shares.erase(shares.begin());
            }
            else if (marginal && item.NUMSHARES > 0)
            {
                m_purchase_total += item.PURCHASEPRICE;
                m_money_total += item.PURCHASEPRICE;
                shares.erase(shares.begin());
            }
        }
        m_commission += item.COMMISSION;
    }

    m_count = static_cast<double>(shares.size());

    if (m_count == 0.0) {
        m_gain_loss = -(m_purchase_total + m_commission);
    }
    else if (marginal) {
        m_purchase_total -= m_commission;
        m_everage_price = m_purchase_total / m_count;
        m_gain_loss = (current_price - m_everage_price) * m_count;
        m_count = -m_count;
    }
    else {
        m_purchase_total += m_commission;
        m_everage_price = m_purchase_total / m_count;
        m_gain_loss = (current_price - m_everage_price) * m_count;
    }

    //wxLogDebug(" Stock total: %.2f  Money total: %.2f  Count: %f Gain_Loss: %f"
    //    , m_purchase_total, m_money_total, m_count, m_gain_loss);

    return;

}