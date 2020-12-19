/*******************************************************
Copyright (C) 2006 Madhan Kanagavel
Copyright (C) 2011, 2012 Stefano Giorgio
Copyright (C) 2013, 2014, 2020 Nikolay

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

#ifndef MM_EX_MONEY_LIST_H_
#define MM_EX_MONEY_LIST_H_

#include "mmpanelbase.h"
#include "stockspanel.h"
#include "assetspanel.h"

class mmStocksPanel;
class mmAssetsPanel;

class MoneyListCtrl : public mmListCtrl
{
public:

    enum EIcons //m_imageList
    {
        ICON_RECONCILED,
        ICON_VOID,
        ICON_FOLLOWUP,
        ICON_NONE,
        ICON_DUPLICATE,
        ICON_DESC,
        ICON_ASC,
    };

    //MoneyListCtrl(mmAssetsPanel* ap, wxWindow* parent, wxWindowID winid = wxID_ANY);
    MoneyListCtrl(int account_id, mmPanelBase* sp, wxWindow* parent, wxWindowID id = wxID_ANY);

    ~MoneyListCtrl();

    Model_Checking::Full_Data_Set m_money;
    int initVirtualListControl(int trx_id = -1, int col = 0, bool asc = true);
    const wxString getMoneyInfo(int selectedIndex) const;

public:
    enum EColumn
    {
        COL_IMGSTATUS = 0,
        COL_TYPE,
        COL_DATE,
        COL_PAYEE_STR,
        COL_STATUS,
        COL_VALUE,
        COL_NOTES,
        COL_MAX, // number of columns
        COL_DEF_SORT = COL_DATE
    };

    inline EColumn toEColumn(long col)
    {
        EColumn res = COL_DEF_SORT;
        if (col >= 0 && col < COL_MAX) res = static_cast<EColumn>(col);

        return res;
    }
    void setColumnImage(EColumn col, int image);

public:
    EColumn g_sortcol; // index of column to sort
    EColumn m_prevSortCol;
    bool g_asc; // asc\desc sorting

    bool getSortOrder() const;
    EColumn getSortColumn() const { return m_sortCol; }
    long getSelectedID() const;

    void setSortOrder(bool asc);
    void setSortColumn(EColumn col);

public:
    void OnMenuHandler(wxCommandEvent& event);

    void OnDeleteTransaction(wxCommandEvent& event);
    void OnEditTransaction(wxCommandEvent& event);
    void OnNewTransaction(wxCommandEvent& event);
    void OnDuplicateTransaction();

    void doRefreshItems(int trans_id = -1, bool filter = true);
    long getSelectedForCopy() const;
    void setSelectedForCopy(long v);

protected:
    /* Sort Columns */
    virtual void OnColClick(wxListEvent& event);


private:
    DECLARE_NO_COPY_CLASS(MoneyListCtrl)
    wxDECLARE_EVENT_TABLE();

    mmStocksPanel* m_sp;

    wxSharedPtr<wxListItemAttr> m_attr1;  // style1
    wxSharedPtr<wxListItemAttr> m_attr2;  // style2
    wxSharedPtr<wxListItemAttr> m_attr3;  // style, for future dates
    wxSharedPtr<wxListItemAttr> m_attr4;  // style, for future dates

private:
    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual wxListItemAttr* OnGetItemAttr(long item) const;
    virtual int OnGetItemColumnImage(long item, long column) const;
    void OnMouseRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnMarkTransaction(wxCommandEvent& event);
    void OnMarkAllTransactions(wxCommandEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnCopy();
    void OnPaste();
    int OnPaste(Model_Checking::Data* tran);
    void do_sort_table();
    void do_update_extra_trx_data(int selIndex);
    bool do_delete_trxs_by_status(Model_Checking::STATUS_ENUM status);
    bool do_delete_viewed_trxs();
    bool do_delete_selected_trx();

    int m_account_id;
    int col_sort() const;
    long m_selectedID; //Selected transaction ID
    long m_selectedForCopy; //The transaction ID if selected for copy

    bool TransactionLocked(const wxString& transdate);
private:
    /* The topmost visible item - this will be used to set
    where to display the list again after refresh */
    long m_topItemIndex;
    EColumn m_sortCol;
    wxString m_today;
    wxSharedPtr<wxImageList> m_imageList;
    void createColumns(mmListCtrl &lst);
    void OnMarkTransaction(const wxString & status);

    enum
    {
        MENU_TREEPOPUP_MIN,
        MENU_TREEPOPUP_1,
        MENU_TREEPOPUP_2,
        MENU_TREEPOPUP_3,
        MENU_TREEPOPUP_4,
        MENU_ON_COPY_TRANSACTION,
        MENU_ON_PASTE_TRANSACTION,
        MENU_ON_NEW_TRANSACTION,
        MENU_ON_DUPLICATE_TRANSACTION,
        MENU_TREEPOPUP_NEW_WITHDRAWAL,
        MENU_TREEPOPUP_NEW_DEPOSIT,
        MENU_TREEPOPUP_NEW_TRANSFER,
        MENU_TREEPOPUP_MARKRECONCILED_ALL,
        MENU_TREEPOPUP_MARKVOID_ALL,
        MENU_TREEPOPUP_EDIT2,
        MENU_TREEPOPUP_DELETE2,
        MENU_TREEPOPUP_DELETE_VIEWED,
        MENU_TREEPOPUP_DELETE_FLAGGED,
        MENU_TREEPOPUP_DELETE_UNRECONCILED,
        MENU_TREEPOPUP_MARKRECONCILED,
        MENU_TREEPOPUP_MARKUNRECONCILED,
        MENU_TREEPOPUP_MARK_ADD_FLAG_FOLLOWUP,
        MENU_TREEPOPUP_MARKVOID,
        MENU_TREEPOPUP_MAX,
    };

};

inline int MoneyListCtrl::col_sort() const { return COL_DEF_SORT; }
inline void MoneyListCtrl::setSortColumn(EColumn col) { m_sortCol = col; }
inline void MoneyListCtrl::setSortOrder(bool asc) { m_asc = asc; }
inline bool MoneyListCtrl::getSortOrder() const { return m_asc; }
inline long MoneyListCtrl::getSelectedID() const { return m_selectedID; }
inline long MoneyListCtrl::getSelectedForCopy() const { return m_selectedForCopy; }
inline void MoneyListCtrl::setSelectedForCopy(long v) { m_selectedForCopy = v; }

#endif // MM_EX_MONEY_LIST_H_

