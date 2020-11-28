/*******************************************************
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
#include "mmpanelbase.h"
#include "model/Model_Asset.h"
#include "model/Model_Account.h"
#include "mmframe.h"
//#include "money_list.h"

class wxListEvent;
class mmAssetsPanel;
class wxButton;

/* Custom ListCtrl class that implements virtual LC style */
class mmAssetsListCtrl: public mmListCtrl
{
    DECLARE_NO_COPY_CLASS(mmAssetsListCtrl)
    wxDECLARE_EVENT_TABLE();

public:
    mmAssetsListCtrl(mmAssetsPanel* cp, wxWindow *parent, wxWindowID winid = wxID_ANY);

    void OnNewAsset(wxCommandEvent& event);
    void OnEditAsset(wxCommandEvent& event);
    void OnDeleteAsset(wxCommandEvent& event);
    void OnDuplicateAsset(wxCommandEvent& event);
    void OnOrganizeAttachments(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);

    int initVirtualListControl(int trx_id = -1, int col = 0, bool asc = true);

    void doSearchTxt(const wxString& txt);
    void doRefreshItems(int trx_id = -1);
    void sortTable();
    wxString getItem(long item, long column) const;
    Model_Asset::Data_Set get_m_assets();

protected:
    virtual void OnColClick(wxListEvent& event);

private:
    mmAssetsPanel* m_panel;
    wxScopedPtr<wxImageList> m_imageList;
    Model_Asset::Data_Set m_assets;

    /* required overrides for virtual style list control */
    virtual wxString OnGetItemText(long item, long column) const;
    virtual int OnGetItemImage(long item) const;

    void OnMouseRightClick(wxMouseEvent& event);
    void OnListLeftClick(wxMouseEvent& event);
    void OnListItemActivated(wxListEvent& event);
    void OnListKeyDown(wxListEvent& event);
    void OnListItemSelected(wxListEvent& event);
    void OnEndLabelEdit(wxListEvent& event);
    bool EditAsset(Model_Asset::Data* pEntry);
    int col_max() { return COL_MAX; }
    int col_sort() { return COL_DATE; }

    enum {
        MENU_TREEPOPUP_NEW = wxID_HIGHEST + 1200,
        MENU_TREEPOPUP_EDIT,
        MENU_TREEPOPUP_DELETE,
        MENU_ON_DUPLICATE_TRANSACTION,
        MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS,
    };
    enum EColumn
    {
        COL_ICON = 0,
        COL_ID,
        COL_NAME,
        COL_DATE,
        COL_TYPE,
        COL_VALUE_INITIAL,
        COL_VALUE_CURRENT,
        COL_NOTES,
        COL_MAX, // number of columns
    };
};

class mmAssetsPanel : public mmPanelBase
{
    wxDECLARE_EVENT_TABLE();

public:
    mmAssetsPanel(mmGUIFrame* frame, wxWindow* parent, int account_id, wxWindowID winid);
    mmGUIFrame* m_frame;

    void updateExtraAssetData(int selIndex);

    Model_Asset::TYPE m_filter_type;

    wxString BuildPage() const { return m_assets_list->BuildPage(_("Assets")); }

    void DisplayAccountDetails(int accountID);
    void AddAssetTrans(const int selected_index);
    void RefreshList(int transID = -1);
    void dataToControls(int transID = -1);
    wxStaticText* header_text_;

private:
    void enableEditDeleteButtons(bool enable);
    void OnSearchTxtEntered(wxCommandEvent& event);
    void OnNotebookPageChanged(wxBookCtrlEvent& event);

    mmAssetsListCtrl* m_assets_list;
    wxButton* m_bitmapTransFilter;
    wxNotebook* m_notebook;
    //MoneyListCtrl* m_listCtrlMoney;
    int m_account_id;
    Model_Currency::Data * m_currency;

    bool Create(wxWindow* parent
        , wxWindowID winid
        , const wxPoint& pos = wxDefaultPosition
        , const wxSize& size =wxDefaultSize
        , long style = wxTAB_TRAVERSAL
        , const wxString &name = "mmAssetsPanel");
    void CreateControls();

    /* Event handlers for Buttons */
    void OnNewAsset(wxCommandEvent& event);
    void OnDeleteAsset(wxCommandEvent& event);
    void OnEditAsset(wxCommandEvent& event);
    void OnOpenAttachment(wxCommandEvent& event);
    void OnMouseLeftDown(wxCommandEvent& event);

    void OnViewPopupSelected(wxCommandEvent& event);

private:
    wxString tips_;
    int m_view_mode;
    void sortTable();

    enum {
        IDC_PANEL_ASSET_STATIC_DETAILS = wxID_HIGHEST + 1220,
        IDC_PANEL_ASSET_STATIC_DETAILS_MINI,
    };

};

inline Model_Asset::Data_Set mmAssetsListCtrl::get_m_assets() { return m_assets; }
