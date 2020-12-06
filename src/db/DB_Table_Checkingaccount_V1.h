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
 *          AUTO GENERATED at 2020-12-06 16:34:20.027000.
 *          DO NOT EDIT!
 */
//=============================================================================
#pragma once

#include "DB_Table.h"

struct DB_Table_CHECKINGACCOUNT_V1 : public DB_Table
{
    struct Data;
    typedef DB_Table_CHECKINGACCOUNT_V1 Self;

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
    ~DB_Table_CHECKINGACCOUNT_V1() 
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
                db->ExecuteUpdate(R"(CREATE TABLE CHECKINGACCOUNT_V1(
TRANSID integer primary key
, ACCOUNTID integer NOT NULL
, TRANSDATE TEXT
, TRANSTIME TEXT
, TRANSCODE TEXT NOT NULL /* Withdrawal, Deposit, Transfer */
, TRANSAMOUNT numeric NOT NULL
, PAYEEID integer NOT NULL
, CATEGID integer
, SUBCATEGID integer
, STATUS TEXT /* None, Reconciled, Void, Follow up, Duplicate */
, NOTES TEXT
, TRANSACTIONNUMBER TEXT
, TOACCOUNTID integer
, TOTRANSAMOUNT numeric
, CURRENCYID integer
, COLOURID integer
, FOLLOWUPID integer
))");
                this->ensure_data(db);
            }
            catch(const wxSQLite3Exception &e) 
            { 
                wxLogError("CHECKINGACCOUNT_V1: Exception %s", e.GetMessage().utf8_str());
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
            db->ExecuteUpdate(R"(CREATE INDEX IF NOT EXISTS IDX_CHECKINGACCOUNT_ACCOUNT ON CHECKINGACCOUNT_V1 (ACCOUNTID, TOACCOUNTID))");
            db->ExecuteUpdate(R"(CREATE INDEX IF NOT EXISTS IDX_CHECKINGACCOUNT_TRANSDATE ON CHECKINGACCOUNT_V1 (TRANSDATE))");
        }
        catch(const wxSQLite3Exception &e) 
        { 
            wxLogError("CHECKINGACCOUNT_V1: Exception %s", e.GetMessage().utf8_str());
            return false;
        }

        return true;
    }

    void ensure_data(wxSQLite3Database* db)
    {
        db->Begin();
        db->Commit();
    }
    
    struct TRANSID : public DB_Column<int>
    { 
        static wxString name() { return "TRANSID"; } 
        explicit TRANSID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct ACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "ACCOUNTID"; } 
        explicit ACCOUNTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct TRANSDATE : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSDATE"; } 
        explicit TRANSDATE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct TRANSTIME : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSTIME"; } 
        explicit TRANSTIME(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct TRANSCODE : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSCODE"; } 
        explicit TRANSCODE(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct TRANSAMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "TRANSAMOUNT"; } 
        explicit TRANSAMOUNT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct PAYEEID : public DB_Column<int>
    { 
        static wxString name() { return "PAYEEID"; } 
        explicit PAYEEID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct CATEGID : public DB_Column<int>
    { 
        static wxString name() { return "CATEGID"; } 
        explicit CATEGID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct SUBCATEGID : public DB_Column<int>
    { 
        static wxString name() { return "SUBCATEGID"; } 
        explicit SUBCATEGID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct STATUS : public DB_Column<wxString>
    { 
        static wxString name() { return "STATUS"; } 
        explicit STATUS(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct NOTES : public DB_Column<wxString>
    { 
        static wxString name() { return "NOTES"; } 
        explicit NOTES(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct TRANSACTIONNUMBER : public DB_Column<wxString>
    { 
        static wxString name() { return "TRANSACTIONNUMBER"; } 
        explicit TRANSACTIONNUMBER(const wxString &v, OP op = EQUAL): DB_Column<wxString>(v, op) {}
    };
    
    struct TOACCOUNTID : public DB_Column<int>
    { 
        static wxString name() { return "TOACCOUNTID"; } 
        explicit TOACCOUNTID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct TOTRANSAMOUNT : public DB_Column<double>
    { 
        static wxString name() { return "TOTRANSAMOUNT"; } 
        explicit TOTRANSAMOUNT(const double &v, OP op = EQUAL): DB_Column<double>(v, op) {}
    };
    
    struct CURRENCYID : public DB_Column<int>
    { 
        static wxString name() { return "CURRENCYID"; } 
        explicit CURRENCYID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct COLOURID : public DB_Column<int>
    { 
        static wxString name() { return "COLOURID"; } 
        explicit COLOURID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    struct FOLLOWUPID : public DB_Column<int>
    { 
        static wxString name() { return "FOLLOWUPID"; } 
        explicit FOLLOWUPID(const int &v, OP op = EQUAL): DB_Column<int>(v, op) {}
    };
    
    typedef TRANSID PRIMARY;
    enum COLUMN
    {
        COL_TRANSID = 0
        , COL_ACCOUNTID = 1
        , COL_TRANSDATE = 2
        , COL_TRANSTIME = 3
        , COL_TRANSCODE = 4
        , COL_TRANSAMOUNT = 5
        , COL_PAYEEID = 6
        , COL_CATEGID = 7
        , COL_SUBCATEGID = 8
        , COL_STATUS = 9
        , COL_NOTES = 10
        , COL_TRANSACTIONNUMBER = 11
        , COL_TOACCOUNTID = 12
        , COL_TOTRANSAMOUNT = 13
        , COL_CURRENCYID = 14
        , COL_COLOURID = 15
        , COL_FOLLOWUPID = 16
    };

    /** Returns the column name as a string*/
    static wxString column_to_name(COLUMN col)
    {
        switch(col)
        {
            case COL_TRANSID: return "TRANSID";
            case COL_ACCOUNTID: return "ACCOUNTID";
            case COL_TRANSDATE: return "TRANSDATE";
            case COL_TRANSTIME: return "TRANSTIME";
            case COL_TRANSCODE: return "TRANSCODE";
            case COL_TRANSAMOUNT: return "TRANSAMOUNT";
            case COL_PAYEEID: return "PAYEEID";
            case COL_CATEGID: return "CATEGID";
            case COL_SUBCATEGID: return "SUBCATEGID";
            case COL_STATUS: return "STATUS";
            case COL_NOTES: return "NOTES";
            case COL_TRANSACTIONNUMBER: return "TRANSACTIONNUMBER";
            case COL_TOACCOUNTID: return "TOACCOUNTID";
            case COL_TOTRANSAMOUNT: return "TOTRANSAMOUNT";
            case COL_CURRENCYID: return "CURRENCYID";
            case COL_COLOURID: return "COLOURID";
            case COL_FOLLOWUPID: return "FOLLOWUPID";
            default: break;
        }
        
        return "UNKNOWN";
    }

    /** Returns the column number from the given column name*/
    static COLUMN name_to_column(const wxString& name)
    {
        if ("TRANSID" == name) return COL_TRANSID;
        else if ("ACCOUNTID" == name) return COL_ACCOUNTID;
        else if ("TRANSDATE" == name) return COL_TRANSDATE;
        else if ("TRANSTIME" == name) return COL_TRANSTIME;
        else if ("TRANSCODE" == name) return COL_TRANSCODE;
        else if ("TRANSAMOUNT" == name) return COL_TRANSAMOUNT;
        else if ("PAYEEID" == name) return COL_PAYEEID;
        else if ("CATEGID" == name) return COL_CATEGID;
        else if ("SUBCATEGID" == name) return COL_SUBCATEGID;
        else if ("STATUS" == name) return COL_STATUS;
        else if ("NOTES" == name) return COL_NOTES;
        else if ("TRANSACTIONNUMBER" == name) return COL_TRANSACTIONNUMBER;
        else if ("TOACCOUNTID" == name) return COL_TOACCOUNTID;
        else if ("TOTRANSAMOUNT" == name) return COL_TOTRANSAMOUNT;
        else if ("CURRENCYID" == name) return COL_CURRENCYID;
        else if ("COLOURID" == name) return COL_COLOURID;
        else if ("FOLLOWUPID" == name) return COL_FOLLOWUPID;

        return COLUMN(-1);
    }
    
    /** Data is a single record in the database table*/
    struct Data
    {
        friend struct DB_Table_CHECKINGACCOUNT_V1;
        /** This is a instance pointer to itself in memory. */
        Self* table_;
    
        int TRANSID;//  primary key
        int ACCOUNTID;
        wxString TRANSDATE;
        wxString TRANSTIME;
        wxString TRANSCODE;
        double TRANSAMOUNT;
        int PAYEEID;
        int CATEGID;
        int SUBCATEGID;
        wxString STATUS;
        wxString NOTES;
        wxString TRANSACTIONNUMBER;
        int TOACCOUNTID;
        double TOTRANSAMOUNT;
        int CURRENCYID;
        int COLOURID;
        int FOLLOWUPID;

        int id() const
        {
            return TRANSID;
        }

        void id(int id)
        {
            TRANSID = id;
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
        
            TRANSID = -1;
            ACCOUNTID = -1;
            TRANSAMOUNT = 0.0;
            PAYEEID = -1;
            CATEGID = -1;
            SUBCATEGID = -1;
            TOACCOUNTID = -1;
            TOTRANSAMOUNT = 0.0;
            CURRENCYID = -1;
            COLOURID = -1;
            FOLLOWUPID = -1;
        }

        explicit Data(wxSQLite3ResultSet& q, Self* table = 0)
        {
            table_ = table;
        
            TRANSID = q.GetInt(0); // TRANSID
            ACCOUNTID = q.GetInt(1); // ACCOUNTID
            TRANSDATE = q.GetString(2); // TRANSDATE
            TRANSTIME = q.GetString(3); // TRANSTIME
            TRANSCODE = q.GetString(4); // TRANSCODE
            TRANSAMOUNT = q.GetDouble(5); // TRANSAMOUNT
            PAYEEID = q.GetInt(6); // PAYEEID
            CATEGID = q.GetInt(7); // CATEGID
            SUBCATEGID = q.GetInt(8); // SUBCATEGID
            STATUS = q.GetString(9); // STATUS
            NOTES = q.GetString(10); // NOTES
            TRANSACTIONNUMBER = q.GetString(11); // TRANSACTIONNUMBER
            TOACCOUNTID = q.GetInt(12); // TOACCOUNTID
            TOTRANSAMOUNT = q.GetDouble(13); // TOTRANSAMOUNT
            CURRENCYID = q.GetInt(14); // CURRENCYID
            COLOURID = q.GetInt(15); // COLOURID
            FOLLOWUPID = q.GetInt(16); // FOLLOWUPID
        }

        Data& operator=(const Data& other)
        {
            if (this == &other) return *this;

            TRANSID = other.TRANSID;
            ACCOUNTID = other.ACCOUNTID;
            TRANSDATE = other.TRANSDATE;
            TRANSTIME = other.TRANSTIME;
            TRANSCODE = other.TRANSCODE;
            TRANSAMOUNT = other.TRANSAMOUNT;
            PAYEEID = other.PAYEEID;
            CATEGID = other.CATEGID;
            SUBCATEGID = other.SUBCATEGID;
            STATUS = other.STATUS;
            NOTES = other.NOTES;
            TRANSACTIONNUMBER = other.TRANSACTIONNUMBER;
            TOACCOUNTID = other.TOACCOUNTID;
            TOTRANSAMOUNT = other.TOTRANSAMOUNT;
            CURRENCYID = other.CURRENCYID;
            COLOURID = other.COLOURID;
            FOLLOWUPID = other.FOLLOWUPID;
            return *this;
        }

        template<typename C>
        bool match(const C &c) const
        {
            return false;
        }

        bool match(const Self::TRANSID &in) const
        {
            return this->TRANSID == in.v_;
        }

        bool match(const Self::ACCOUNTID &in) const
        {
            return this->ACCOUNTID == in.v_;
        }

        bool match(const Self::TRANSDATE &in) const
        {
            return this->TRANSDATE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::TRANSTIME &in) const
        {
            return this->TRANSTIME.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::TRANSCODE &in) const
        {
            return this->TRANSCODE.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::TRANSAMOUNT &in) const
        {
            return this->TRANSAMOUNT == in.v_;
        }

        bool match(const Self::PAYEEID &in) const
        {
            return this->PAYEEID == in.v_;
        }

        bool match(const Self::CATEGID &in) const
        {
            return this->CATEGID == in.v_;
        }

        bool match(const Self::SUBCATEGID &in) const
        {
            return this->SUBCATEGID == in.v_;
        }

        bool match(const Self::STATUS &in) const
        {
            return this->STATUS.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::NOTES &in) const
        {
            return this->NOTES.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::TRANSACTIONNUMBER &in) const
        {
            return this->TRANSACTIONNUMBER.CmpNoCase(in.v_) == 0;
        }

        bool match(const Self::TOACCOUNTID &in) const
        {
            return this->TOACCOUNTID == in.v_;
        }

        bool match(const Self::TOTRANSAMOUNT &in) const
        {
            return this->TOTRANSAMOUNT == in.v_;
        }

        bool match(const Self::CURRENCYID &in) const
        {
            return this->CURRENCYID == in.v_;
        }

        bool match(const Self::COLOURID &in) const
        {
            return this->COLOURID == in.v_;
        }

        bool match(const Self::FOLLOWUPID &in) const
        {
            return this->FOLLOWUPID == in.v_;
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
            json_writer.Key("TRANSID");
            json_writer.Int(this->TRANSID);
            json_writer.Key("ACCOUNTID");
            json_writer.Int(this->ACCOUNTID);
            json_writer.Key("TRANSDATE");
            json_writer.String(this->TRANSDATE.utf8_str());
            json_writer.Key("TRANSTIME");
            json_writer.String(this->TRANSTIME.utf8_str());
            json_writer.Key("TRANSCODE");
            json_writer.String(this->TRANSCODE.utf8_str());
            json_writer.Key("TRANSAMOUNT");
            json_writer.Double(this->TRANSAMOUNT);
            json_writer.Key("PAYEEID");
            json_writer.Int(this->PAYEEID);
            json_writer.Key("CATEGID");
            json_writer.Int(this->CATEGID);
            json_writer.Key("SUBCATEGID");
            json_writer.Int(this->SUBCATEGID);
            json_writer.Key("STATUS");
            json_writer.String(this->STATUS.utf8_str());
            json_writer.Key("NOTES");
            json_writer.String(this->NOTES.utf8_str());
            json_writer.Key("TRANSACTIONNUMBER");
            json_writer.String(this->TRANSACTIONNUMBER.utf8_str());
            json_writer.Key("TOACCOUNTID");
            json_writer.Int(this->TOACCOUNTID);
            json_writer.Key("TOTRANSAMOUNT");
            json_writer.Double(this->TOTRANSAMOUNT);
            json_writer.Key("CURRENCYID");
            json_writer.Int(this->CURRENCYID);
            json_writer.Key("COLOURID");
            json_writer.Int(this->COLOURID);
            json_writer.Key("FOLLOWUPID");
            json_writer.Int(this->FOLLOWUPID);
        }

        row_t to_row_t() const
        {
            row_t row;
            row(L"TRANSID") = TRANSID;
            row(L"ACCOUNTID") = ACCOUNTID;
            row(L"TRANSDATE") = TRANSDATE;
            row(L"TRANSTIME") = TRANSTIME;
            row(L"TRANSCODE") = TRANSCODE;
            row(L"TRANSAMOUNT") = TRANSAMOUNT;
            row(L"PAYEEID") = PAYEEID;
            row(L"CATEGID") = CATEGID;
            row(L"SUBCATEGID") = SUBCATEGID;
            row(L"STATUS") = STATUS;
            row(L"NOTES") = NOTES;
            row(L"TRANSACTIONNUMBER") = TRANSACTIONNUMBER;
            row(L"TOACCOUNTID") = TOACCOUNTID;
            row(L"TOTRANSAMOUNT") = TOTRANSAMOUNT;
            row(L"CURRENCYID") = CURRENCYID;
            row(L"COLOURID") = COLOURID;
            row(L"FOLLOWUPID") = FOLLOWUPID;
            return row;
        }

        void to_template(html_template& t) const
        {
            t(L"TRANSID") = TRANSID;
            t(L"ACCOUNTID") = ACCOUNTID;
            t(L"TRANSDATE") = TRANSDATE;
            t(L"TRANSTIME") = TRANSTIME;
            t(L"TRANSCODE") = TRANSCODE;
            t(L"TRANSAMOUNT") = TRANSAMOUNT;
            t(L"PAYEEID") = PAYEEID;
            t(L"CATEGID") = CATEGID;
            t(L"SUBCATEGID") = SUBCATEGID;
            t(L"STATUS") = STATUS;
            t(L"NOTES") = NOTES;
            t(L"TRANSACTIONNUMBER") = TRANSACTIONNUMBER;
            t(L"TOACCOUNTID") = TOACCOUNTID;
            t(L"TOTRANSAMOUNT") = TOTRANSAMOUNT;
            t(L"CURRENCYID") = CURRENCYID;
            t(L"COLOURID") = COLOURID;
            t(L"FOLLOWUPID") = FOLLOWUPID;
        }

        /** Save the record instance in memory to the database. */
        bool save(wxSQLite3Database* db)
        {
            if (db && db->IsReadOnly()) return false;
            if (!table_ || !db) 
            {
                wxLogError("can not save CHECKINGACCOUNT_V1");
                return false;
            }

            return table_->save(this, db);
        }

        /** Remove the record instance from memory and the database. */
        bool remove(wxSQLite3Database* db)
        {
            if (!table_ || !db) 
            {
                wxLogError("can not remove CHECKINGACCOUNT_V1");
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
        NUM_COLUMNS = 17
    };

    size_t num_columns() const { return NUM_COLUMNS; }

    /** Name of the table*/    
    wxString name() const { return "CHECKINGACCOUNT_V1"; }

    DB_Table_CHECKINGACCOUNT_V1() : fake_(new Data())
    {
        query_ = "SELECT TRANSID, ACCOUNTID, TRANSDATE, TRANSTIME, TRANSCODE, TRANSAMOUNT, PAYEEID, CATEGID, SUBCATEGID, STATUS, NOTES, TRANSACTIONNUMBER, TOACCOUNTID, TOTRANSAMOUNT, CURRENCYID, COLOURID, FOLLOWUPID FROM CHECKINGACCOUNT_V1 ";
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
            sql = "INSERT INTO CHECKINGACCOUNT_V1(ACCOUNTID, TRANSDATE, TRANSTIME, TRANSCODE, TRANSAMOUNT, PAYEEID, CATEGID, SUBCATEGID, STATUS, NOTES, TRANSACTIONNUMBER, TOACCOUNTID, TOTRANSAMOUNT, CURRENCYID, COLOURID, FOLLOWUPID) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
        }
        else
        {
            sql = "UPDATE CHECKINGACCOUNT_V1 SET ACCOUNTID = ?, TRANSDATE = ?, TRANSTIME = ?, TRANSCODE = ?, TRANSAMOUNT = ?, PAYEEID = ?, CATEGID = ?, SUBCATEGID = ?, STATUS = ?, NOTES = ?, TRANSACTIONNUMBER = ?, TOACCOUNTID = ?, TOTRANSAMOUNT = ?, CURRENCYID = ?, COLOURID = ?, FOLLOWUPID = ? WHERE TRANSID = ?";
        }

        try
        {
            wxSQLite3Statement stmt = db->PrepareStatement(sql);

            stmt.Bind(1, entity->ACCOUNTID);
            stmt.Bind(2, entity->TRANSDATE);
            stmt.Bind(3, entity->TRANSTIME);
            stmt.Bind(4, entity->TRANSCODE);
            stmt.Bind(5, entity->TRANSAMOUNT);
            stmt.Bind(6, entity->PAYEEID);
            stmt.Bind(7, entity->CATEGID);
            stmt.Bind(8, entity->SUBCATEGID);
            stmt.Bind(9, entity->STATUS);
            stmt.Bind(10, entity->NOTES);
            stmt.Bind(11, entity->TRANSACTIONNUMBER);
            stmt.Bind(12, entity->TOACCOUNTID);
            stmt.Bind(13, entity->TOTRANSAMOUNT);
            stmt.Bind(14, entity->CURRENCYID);
            stmt.Bind(15, entity->COLOURID);
            stmt.Bind(16, entity->FOLLOWUPID);
            if (entity->id() > 0)
                stmt.Bind(17, entity->TRANSID);

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
            wxLogError("CHECKINGACCOUNT_V1: Exception %s, %s", e.GetMessage().utf8_str(), entity->to_json());
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
            wxString sql = "DELETE FROM CHECKINGACCOUNT_V1 WHERE TRANSID = ?";
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
            wxLogError("CHECKINGACCOUNT_V1: Exception %s", e.GetMessage().utf8_str());
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

