// -*- C++ -*-
//=============================================================================
/**
 *      Copyright: (c) 2013 - 2020 Guan Lisheng (guanlisheng@gmail.com)
 *      Copyright: (c) 2017 - 2018 Stefano Giorgio (stef145g)
 *
 *      @file
 *
 *      @author [sqlite2cpp.py]
 *
 *      @brief
 *
 *      Revision History:
 *          AUTO GENERATED at 2020-11-04 23:26:12.891000.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_TICKERPROPERTIES_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_TICKERPROPERTIES_V1 Self;

    /** A container to hold list of Data records for the table*/
    struct Data_Set : public std::vector<Self::Data>
    {
        /**Return the data records as a json array string */
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

            json_writer.StartArray();
            for (const auto & item: *this)
            {
                json_writer.StartObject();
                item.as_json(json_writer);
                json_writer.EndObject();
            }
            json_writer.EndArray();

            return json_buffer.GetString();
        }
    };

    /** A container to hold a list of Data record pointers for the table in memory*/
    typedef std::vector<Self::Data*> Cache;
    typedef std::map<int, Self::Data*> Index_By_Id;
    Cache cache_;
    Index_By_Id index_by_id_;
    Data* fake_; // in case the entity not found

    /** Destructor: clears any data records stored in memory */
    ~DB_Table_TICKERPROPERTIES_V1() 
    {
        delete this->fake_;
        destroy_cache();
    }
     
    /** Removes all records stored in memory (cache) for the table*/ 
    void destroy_cache()
    {
        std::for_each(cache_.begin(), cache_.end(), std::mem_fun(&Data::destroy));
        cache_.clear();
        index_by_id_.clear(); // no memory release since it just stores pointer and the according objects are in cache
    }

    /** Creates the database table if the table does not exist*/
    bool ensure(wxSQLite3Database* db)
    {
        if (!exists(db))
        {
            try
            {
                db->ExecuteUpdate(R"(CREATE TABLE TICKERPROPERTIES_V1 (TICKERID INTEGER PRIMARY KEY,UNIQUENAME TEXT UNIQUE COLLATE NOCASE NOT NULL,SOURCE INTEGER, /* Yahoo, MorningStar, MOEX */ SYMBOL TEXT COLLATE NOCASE NOT NULL,SOURCENAME TEXT,MARKET TEXT, TYPE INTEGER DEFAULT 0, /* Share, Fund, Bond, Corp. Bond */ COUNTRY TEXT,SECTOR TEXT, /*Basic Materials, Consumer Cyclical, Financial Services, Real Estate, Consumer Defensive, Healthcare, Utilities, Communication Services, Energy, Industrials, Technology, Other */ INDUSTRY TEXT,WEBPAGE TEXT,NOTES TEXT,PRECISION INTEGER,CURRENCY_SYMBOL TEXT))");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("TICKERPROPERTIES_V1: Exception %s", e.GetMessage().utf8_str());
                return false;
            }
        }

        this->ensure_index(db);

        return true;
    }

    bool ensure_index(wxSQLite3Database* db)
    {
        try
        {
            db->ExecuteUpdate(R"(CREATE INDEX IF NOT EXISTS IDX_TICKER ON TICKERPROPERTIES_V1 (SYMBOL, TICKERID))");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("TICKERPROPERTIES_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct TICKERID : public DB_Column<int>
    { 
        static wxString name() { return "TICKERID"; } 
        explicit TICKERID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct UNIQUENAME : public DB_Column<wxString>
    { 
        static wxString name() { return "UNIQUENAME"; } 
        explicit UNIQUENAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct SOURCE : public DB_Column<int>
    { 
        static wxString name() { return "SOURCE"; } 
        explicit SOURCE(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct SYMBOL : public DB_Column<wxString>
    { 
        static wxString name() { return "SYMBOL"; } 
        explicit SYMBOL(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct SOURCENAME : public DB_Column<wxString>
    { 
        static wxString name() { return "SOURCENAME"; } 
        explicit SOURCENAME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct MARKET : public DB_Column<wxString>
    { 
        static wxString name() { return "MARKET"; } 
        explicit MARKET(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct TYPE : public DB_Column<int>
    { 
        static wxString name() { return "TYPE"; } 
        explicit TYPE(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct COUNTRY : public DB_Column<wxString>
    { 
        static wxString name() { return "COUNTRY"; } 
        explicit COUNTRY(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct SECTOR : public DB_Column<wxString>
    { 
        static wxString name() { return "SECTOR"; } 
        explicit SECTOR(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct INDUSTRY : public DB_Column<wxString>
    { 
        static wxString name() { return "INDUSTRY"; } 
        explicit INDUSTRY(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct WEBPAGE : public DB_Column<wxString>
    { 
        static wxString name() { return "WEBPAGE"; } 
        explicit WEBPAGE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        explicit NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct PRECISION : public DB_Column<int>
    { 
        static wxString name() { return "PRECISION"; } 
        explicit PRECISION(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct CURRENCY_SYMBOL : public DB_Column<wxString>
    { 
        static wxString name() { return "CURRENCY_SYMBOL"; } 
        explicit CURRENCY_SYMBOL(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    typedef TICKERID PRIMARY;
    enum COLUMN
    {
        COL_TICKERID = 0
        , COL_UNIQUENAME = 1
        , COL_SOURCE = 2
        , COL_SYMBOL = 3
        , COL_SOURCENAME = 4
        , COL_MARKET = 5
        , COL_TYPE = 6
        , COL_COUNTRY = 7
        , COL_SECTOR = 8
        , COL_INDUSTRY = 9
        , COL_WEBPAGE = 10
        , COL_NOTES = 11
        , COL_PRECISION = 12
        , COL_CURRENCY_SYMBOL = 13
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_TICKERID: return "TICKERID";
            case COL_UNIQUENAME: return "UNIQUENAME";
            case COL_SOURCE: return "SOURCE";
            case COL_SYMBOL: return "SYMBOL";
            case COL_SOURCENAME: return "SOURCENAME";
            case COL_MARKET: return "MARKET";
            case COL_TYPE: return "TYPE";
            case COL_COUNTRY: return "COUNTRY";
            case COL_SECTOR: return "SECTOR";
            case COL_INDUSTRY: return "INDUSTRY";
            case COL_WEBPAGE: return "WEBPAGE";
            case COL_NOTES: return "NOTES";
            case COL_PRECISION: return "PRECISION";
            case COL_CURRENCY_SYMBOL: return "CURRENCY_SYMBOL";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("TICKERID" == name) return COL_TICKERID;
        else if ("UNIQUENAME" == name) return COL_UNIQUENAME;
        else if ("SOURCE" == name) return COL_SOURCE;
        else if ("SYMBOL" == name) return COL_SYMBOL;
        else if ("SOURCENAME" == name) return COL_SOURCENAME;
        else if ("MARKET" == name) return COL_MARKET;
        else if ("TYPE" == name) return COL_TYPE;
        else if ("COUNTRY" == name) return COL_COUNTRY;
        else if ("SECTOR" == name) return COL_SECTOR;
        else if ("INDUSTRY" == name) return COL_INDUSTRY;
        else if ("WEBPAGE" == name) return COL_WEBPAGE;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("PRECISION" == name) return COL_PRECISION;
        else if ("CURRENCY_SYMBOL" == name) return COL_CURRENCY_SYMBOL;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_TICKERPROPERTIES_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int TICKERID;//  primary key
        wxString UNIQUENAME;
        int SOURCE;
        wxString SYMBOL;
        wxString SOURCENAME;
        wxString MARKET;
        int TYPE;
        wxString COUNTRY;
        wxString SECTOR;
        wxString INDUSTRY;
        wxString WEBPAGE;
        wxString NOTES;
        int PRECISION;
        wxString CURRENCY_SYMBOL;

        int id() const
        {
            return TICKERID;
        }

        void id(int id)
        {
            TICKERID = id;
        }

        bool operator < (const Data& r) const
        {
            return this->id() < r.id();
        }
        
        bool operator < (const Data* r) const
        {
            return this->id() < r->id();
        }

        explicit Data(Self* table = 0) 
        {
            table_ = table;
        
            TICKERID = -1;
            SOURCE = -1;
            TYPE = -1;
            PRECISION = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            TICKERID = q.GetInt(0); // TICKERID
            UNIQUENAME = q.GetString(1); // UNIQUENAME
            SOURCE = q.GetInt(2); // SOURCE
            SYMBOL = q.GetString(3); // SYMBOL
            SOURCENAME = q.GetString(4); // SOURCENAME
            MARKET = q.GetString(5); // MARKET
            TYPE = q.GetInt(6); // TYPE
            COUNTRY = q.GetString(7); // COUNTRY
            SECTOR = q.GetString(8); // SECTOR
            INDUSTRY = q.GetString(9); // INDUSTRY
            WEBPAGE = q.GetString(10); // WEBPAGE
            NOTES = q.GetString(11); // NOTES
            PRECISION = q.GetInt(12); // PRECISION
            CURRENCY_SYMBOL = q.GetString(13); // CURRENCY_SYMBOL
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            TICKERID = other.TICKERID;
            UNIQUENAME = other.UNIQUENAME;
            SOURCE = other.SOURCE;
            SYMBOL = other.SYMBOL;
            SOURCENAME = other.SOURCENAME;
            MARKET = other.MARKET;
            TYPE = other.TYPE;
            COUNTRY = other.COUNTRY;
            SECTOR = other.SECTOR;
            INDUSTRY = other.INDUSTRY;
            WEBPAGE = other.WEBPAGE;
            NOTES = other.NOTES;
            PRECISION = other.PRECISION;
            CURRENCY_SYMBOL = other.CURRENCY_SYMBOL;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }

        bool match(const Self::TICKERID &in) const
        {
            return this->TICKERID == in.v_;
        }

        bool match(const Self::UNIQUENAME &in) const
        {
            return this->UNIQUENAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::SOURCE &in) const
        {
            return this->SOURCE == in.v_;
        }

        bool match(const Self::SYMBOL &in) const
        {
            return this->SYMBOL.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::SOURCENAME &in) const
        {
            return this->SOURCENAME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::MARKET &in) const
        {
            return this->MARKET.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::TYPE &in) const
        {
            return this->TYPE == in.v_;
        }

        bool match(const Self::COUNTRY &in) const
        {
            return this->COUNTRY.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::SECTOR &in) const
        {
            return this->SECTOR.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::INDUSTRY &in) const
        {
            return this->INDUSTRY.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::WEBPAGE &in) const
        {
            return this->WEBPAGE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::NOTES &in) const
        {
            return this->NOTES.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::PRECISION &in) const
        {
            return this->PRECISION == in.v_;
        }

        bool match(const Self::CURRENCY_SYMBOL &in) const
        {
            return this->CURRENCY_SYMBOL.CmpNoCase(in.v_) == 0;
        }

        // Return the data record as a json string
        wxString to_json() const
        {
            StringBuffer json_buffer;
            PrettyWriter<StringBuffer> json_writer(json_buffer);

			json_writer.StartObject();			
			this->as_json(json_writer);
            json_writer.EndObject();

            return json_buffer.GetString();
        }

        // Add the field data as json key:value pairs
        void as_json(PrettyWriter<StringBuffer>& json_writer) const
        {
            json_writer.Key("TICKERID");
            json_writer.Int(this->TICKERID);
            json_writer.Key("UNIQUENAME");
            json_writer.String(this->UNIQUENAME.utf8_str());
            json_writer.Key("SOURCE");
            json_writer.Int(this->SOURCE);
            json_writer.Key("SYMBOL");
            json_writer.String(this->SYMBOL.utf8_str());
            json_writer.Key("SOURCENAME");
            json_writer.String(this->SOURCENAME.utf8_str());
            json_writer.Key("MARKET");
            json_writer.String(this->MARKET.utf8_str());
            json_writer.Key("TYPE");
            json_writer.Int(this->TYPE);
            json_writer.Key("COUNTRY");
            json_writer.String(this->COUNTRY.utf8_str());
            json_writer.Key("SECTOR");
            json_writer.String(this->SECTOR.utf8_str());
            json_writer.Key("INDUSTRY");
            json_writer.String(this->INDUSTRY.utf8_str());
            json_writer.Key("WEBPAGE");
            json_writer.String(this->WEBPAGE.utf8_str());
            json_writer.Key("NOTES");
            json_writer.String(this->NOTES.utf8_str());
            json_writer.Key("PRECISION");
            json_writer.Int(this->PRECISION);
            json_writer.Key("CURRENCY_SYMBOL");
            json_writer.String(this->CURRENCY_SYMBOL.utf8_str());
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"TICKERID") = TICKERID;
            row(L"UNIQUENAME") = UNIQUENAME;
            row(L"SOURCE") = SOURCE;
            row(L"SYMBOL") = SYMBOL;
            row(L"SOURCENAME") = SOURCENAME;
            row(L"MARKET") = MARKET;
            row(L"TYPE") = TYPE;
            row(L"COUNTRY") = COUNTRY;
            row(L"SECTOR") = SECTOR;
            row(L"INDUSTRY") = INDUSTRY;
            row(L"WEBPAGE") = WEBPAGE;
            row(L"NOTES") = NOTES;
            row(L"PRECISION") = PRECISION;
            row(L"CURRENCY_SYMBOL") = CURRENCY_SYMBOL;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"TICKERID") = TICKERID;
            t(L"UNIQUENAME") = UNIQUENAME;
            t(L"SOURCE") = SOURCE;
            t(L"SYMBOL") = SYMBOL;
            t(L"SOURCENAME") = SOURCENAME;
            t(L"MARKET") = MARKET;
            t(L"TYPE") = TYPE;
            t(L"COUNTRY") = COUNTRY;
            t(L"SECTOR") = SECTOR;
            t(L"INDUSTRY") = INDUSTRY;
            t(L"WEBPAGE") = WEBPAGE;
            t(L"NOTES") = NOTES;
            t(L"PRECISION") = PRECISION;
            t(L"CURRENCY_SYMBOL") = CURRENCY_SYMBOL;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save TICKERPROPERTIES_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove TICKERPROPERTIES_V1");
                return false;
            }
            
            return table_->remove(this, db);
        }

        void destroy()
        {
            delete this;
        }
    };

    enum
    {
        NUM_COLUMNS = 14
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "TICKERPROPERTIES_V1"; }

    DB_Table_TICKERPROPERTIES_V1() : fake_(new Data())
    {
        query_ = "SELECT TICKERID, UNIQUENAME, SOURCE, SYMBOL, SOURCENAME, MARKET, TYPE, COUNTRY, SECTOR, INDUSTRY, WEBPAGE, NOTES, PRECISION, CURRENCY_SYMBOL FROM TICKERPROPERTIES_V1 ";
    }

    /** Create a new Data record and add to memory table (cache)*/
    Self::Data* create()
    {
        Self::Data* entity = new Self::Data(this);
        cache_.push_back(entity);
        return entity;
    }
    
    /** Create a copy of the Data record and add to memory table (cache)*/
    Self::Data* clone(const Data* e)
    {
        Self::Data* entity = create();
        *entity = *e;
        entity->id(-1);
        return entity;
    }

    /**
    * Saves the Data record to the database table.
    * Either create a new record or update the existing record.
    * Remove old record from the memory table (cache)
    */
    bool save(Self::Data* entity, wxSQLite3Database* db)
    {
        wxString sql = wxEmptyString;
        if (entity->id() <= 0) //  new & insert
        {
            sql = "INSERT INTO TICKERPROPERTIES_V1(UNIQUENAME, SOURCE, SYMBOL, SOURCENAME, MARKET, TYPE, COUNTRY, SECTOR, INDUSTRY, WEBPAGE, NOTES, PRECISION, CURRENCY_SYMBOL) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE TICKERPROPERTIES_V1 SET UNIQUENAME = ?, SOURCE = ?, SYMBOL = ?, SOURCENAME = ?, MARKET = ?, TYPE = ?, COUNTRY = ?, SECTOR = ?, INDUSTRY = ?, WEBPAGE = ?, NOTES = ?, PRECISION = ?, CURRENCY_SYMBOL = ? WHERE TICKERID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->UNIQUENAME);
            stmt.Bind(2, entity->SOURCE);
            stmt.Bind(3, entity->SYMBOL);
            stmt.Bind(4, entity->SOURCENAME);
            stmt.Bind(5, entity->MARKET);
            stmt.Bind(6, entity->TYPE);
            stmt.Bind(7, entity->COUNTRY);
            stmt.Bind(8, entity->SECTOR);
            stmt.Bind(9, entity->INDUSTRY);
            stmt.Bind(10, entity->WEBPAGE);
            stmt.Bind(11, entity->NOTES);
            stmt.Bind(12, entity->PRECISION);
            stmt.Bind(13, entity->CURRENCY_SYMBOL);
            if (entity->id() > 0)
                stmt.Bind(14, entity->TICKERID);

            stmt.ExecuteUpdate();
            stmt.Finalize();

            if (entity->id() > 0) // existent
            {
                for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
                {
                    Self::Data* e = *it;
                    if (e->id() == entity->id()) 
                        *e = *entity;  // in-place update
                }
            }
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("TICKERPROPERTIES_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
            return false;
        }

        if (entity->id() <= 0)
        {
            entity->id((db->GetLastRowId()).ToLong());
            index_by_id_.insert(std::make_pair(entity->id(), entity));
        }
        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(int id, wxSQLite3Database* db)
    {
        if (id <= 0) return false;
        try
        {
            wxString sql = "DELETE FROM TICKERPROPERTIES_V1 WHERE TICKERID = ?";
            wxSQLite3Statement stmt = db->PrepareStatement(sql);
            stmt.Bind(1, id);
            stmt.ExecuteUpdate();
            stmt.Finalize();

            Cache c;
            for(Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it)
            {
                Self::Data* entity = *it;
                if (entity->id() == id) 
                {
                    index_by_id_.erase(entity->id());
                    delete entity;
                }
                else 
                {
                    c.push_back(entity);
                }
            }
            cache_.clear();
            cache_.swap(c);
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("TICKERPROPERTIES_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    /** Remove the Data record from the database and the memory table (cache) */
    bool remove(Self::Data* entity, wxSQLite3Database* db)
    {
        if (remove(entity->id(), db))
        {
            entity->id(-1);
            return true;
        }

        return false;
    }

    template<typename... Args>
    Self::Data* get_one(const Args& ... args)
    {
        for (Index_By_Id::iterator it = index_by_id_.begin(); it != index_by_id_.end(); ++ it)
        {
            Self::Data* item = it->second;
            if (item->id() > 0 && match(item, args...)) 
            {
                ++ hit_;
                return item;
            }
        }

        ++ miss_;

        return 0;
    }
    
    /**
    * Search the memory table (Cache) for the data record.
    * If not found in memory, search the database and update the cache.
    */
    Self::Data* get(int id, wxSQLite3Database* db)
    {
        if (id <= 0) 
        {
            ++ skip_;
            return 0;
        }

        Index_By_Id::iterator it = index_by_id_.find(id);
        if (it != index_by_id_.end())
        {
            ++ hit_;
            return it->second;
        }
        
        ++ miss_;
        Self::Data* entity = 0;
        wxString where = wxString::Format(" WHERE %s = ?", PRIMARY::name().utf8_str());
        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(this->query() + where);
            stmt.Bind(1, id);

            wxSQLite3ResultSet q = stmt.ExecuteQuery();
            if(q.NextRow())
            {
                entity = new Self::Data(q, this);
                cache_.push_back(entity);
                index_by_id_.insert(std::make_pair(id, entity));
            }
            stmt.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }
        
        if (!entity) 
        {
            entity = this->fake_;
            // wxLogError("%s: %d not found", this->name().utf8_str(), id);
        }
 
        return entity;
    }

    /**
    * Return a list of Data records (Data_Set) derived directly from the database.
    * The Data_Set is sorted based on the column number.
    */
    const Data_Set all(wxSQLite3Database* db, COLUMN col = COLUMN(0), bool asc = true)
    {
        Data_Set result;
        try
        {
            wxSQLite3ResultSet q = db->ExecuteQuery(col == COLUMN(0) ? this->query() : this->query() + " ORDER BY " + column_to_name(col) + " COLLATE NOCASE " + (asc ? " ASC " : " DESC "));

            while(q.NextRow())
            {
                Self::Data entity(q, this);
                result.push_back(std::move(entity));
            }

            q.Finalize();
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("%s: Exception %s", this->name().utf8_str(), e.GetMessage().utf8_str());
        }

        return result;
    }
};

