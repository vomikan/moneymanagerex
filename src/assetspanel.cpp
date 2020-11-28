/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel
 Copyright (C) 2015, 2020 Nikolay

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

#include "assetspanel.h"
#include "assetdialog.h"
#include "attachmentdialog.h"
#include "constants.h"
#include "images_list.h"

#include "model/allmodel.h"
#include <wx/srchctrl.h>

/*******************************************************/

wxBEGIN_EVENT_TABLE(mmAssetsListCtrl, mmListCtrl)
    EVT_LIST_ITEM_ACTIVATED(wxID_ANY,   mmAssetsListCtrl::OnListItemActivated)
    EVT_LIST_ITEM_SELECTED(wxID_ANY,    mmAssetsListCtrl::OnListItemSelected)
    EVT_LIST_END_LABEL_EDIT(wxID_ANY,   mmAssetsListCtrl::OnEndLabelEdit)
    EVT_RIGHT_DOWN(mmAssetsListCtrl::OnMouseRightClick)
    EVT_LEFT_DOWN(mmAssetsListCtrl::OnListLeftClick)

    EVT_MENU(MENU_TREEPOPUP_NEW,    mmAssetsListCtrl::OnNewAsset)
    EVT_MENU(MENU_TREEPOPUP_EDIT,   mmAssetsListCtrl::OnEditAsset)
    EVT_MENU(MENU_TREEPOPUP_DELETE, mmAssetsListCtrl::OnDeleteAsset)
    EVT_MENU(MENU_ON_DUPLICATE_TRANSACTION, mmAssetsListCtrl::OnDuplicateAsset)
    EVT_MENU(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, mmAssetsListCtrl::OnOrganizeAttachments)

    EVT_LIST_KEY_DOWN(wxID_ANY, mmAssetsListCtrl::OnListKeyDown)
wxEND_EVENT_TABLE()
/*******************************************************/

mmAssetsListCtrl::mmAssetsListCtrl(mmAssetsPanel* cp, wxWindow *parent, wxWindowID winid)
: mmListCtrl(parent, winid)
, m_panel(cp)
{
    int x = Option::instance().getIconSize();
    m_imageList.reset(new wxImageList(x, x));
    m_imageList->Add(mmBitmap(png::PROPERTY));
    m_imageList->Add(mmBitmap(png::CAR));
    m_imageList->Add(mmBitmap(png::HOUSEHOLD_OBJ));
    m_imageList->Add(mmBitmap(png::ART));
    m_imageList->Add(mmBitmap(png::JEWELLERY));
    m_imageList->Add(mmBitmap(png::CASH));
    m_imageList->Add(mmBitmap(png::OTHER));
    m_imageList->Add(mmBitmap(png::UPARROW));
    m_imageList->Add(mmBitmap(png::DOWNARROW));

    SetImageList(m_imageList.get(), wxIMAGE_LIST_SMALL);

    ToggleWindowStyle(wxLC_EDIT_LABELS);

    m_columns.push_back(PANEL_COLUMN(" ", 25, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("ID"), wxLIST_AUTOSIZE, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Name"), 150, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Date"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Type"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_LEFT));
    m_columns.push_back(PANEL_COLUMN(_("Initial Value"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Current Value"), wxLIST_AUTOSIZE_USEHEADER, wxLIST_FORMAT_RIGHT));
    m_columns.push_back(PANEL_COLUMN(_("Notes"), 450, wxLIST_FORMAT_LEFT));

    m_col_width = "ASSETS_COL%d_WIDTH";
    for (const auto& entry : m_columns)
    {
        int count = GetColumnCount();
        InsertColumn(count
            , entry.HEADER
            , entry.FORMAT
            , Model_Setting::instance().GetIntSetting(wxString::Format(m_col_width, count), entry.WIDTH));
    }

    // load the global variables
    m_default_sort_column = col_sort();
    m_selected_col = Model_Setting::instance().GetIntSetting("ASSETS_SORT_COL", m_default_sort_column);
    m_asc = Model_Setting::instance().GetBoolSetting("ASSETS_ASC", true);
}

void mmAssetsListCtrl::OnMouseRightClick(wxMouseEvent& event)
{
    if (m_selected_row > -1)
        SetItemState(m_selected_row, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);
    int Flags = wxLIST_HITTEST_ONITEM;
    m_selected_row = HitTest(wxPoint(event.m_x, event.m_y), Flags);

    if (m_selected_row >= 0)
    {
        SetItemState(m_selected_row, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(m_selected_row, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
    }
    m_panel->updateExtraAssetData(m_selected_row);
    wxMenu menu;
    menu.Append(MENU_TREEPOPUP_NEW, _("&New Asset"));
    menu.AppendSeparator();
    menu.Append(MENU_ON_DUPLICATE_TRANSACTION, _("D&uplicate Asset"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_EDIT, _("&Edit Asset"));
    menu.Append(MENU_TREEPOPUP_DELETE, _("&Delete Asset"));
    menu.AppendSeparator();
    menu.Append(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, _("&Organize Attachments"));
    if (m_selected_row < 0)
    {
        menu.Enable(MENU_ON_DUPLICATE_TRANSACTION, false);
        menu.Enable(MENU_TREEPOPUP_EDIT, false);
        menu.Enable(MENU_TREEPOPUP_DELETE, false);
        menu.Enable(MENU_TREEPOPUP_ORGANIZE_ATTACHMENTS, false);
    }
    PopupMenu(&menu, event.GetPosition());
}

void mmAssetsListCtrl::OnListLeftClick(wxMouseEvent& event)
{
    int Flags = wxLIST_HITTEST_ONITEM;
    long index = HitTest(wxPoint(event.m_x, event.m_y), Flags);
    if (index == -1)
    {
        m_selected_row = -1;
        m_panel->updateExtraAssetData(m_selected_row);
    }
    event.Skip();
}

wxString mmAssetsListCtrl::OnGetItemText(long item, long column) const
{
    return getItem(item, column);
}

void mmAssetsListCtrl::OnListItemSelected(wxListEvent& event)
{
    m_selected_row = event.GetIndex();
    m_panel->updateExtraAssetData(m_selected_row);
}

int mmAssetsListCtrl::OnGetItemImage(long item) const
{
    return Model_Asset::type(m_assets[item]);
}

void mmAssetsListCtrl::OnListKeyDown(wxListEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE)
    {
        wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, MENU_TREEPOPUP_DELETE);
        OnDeleteAsset(evt);
    }
    else
    {
        event.Skip();
    }
}

void mmAssetsListCtrl::OnNewAsset(wxCommandEvent& /*event*/)
{
    mmAssetDialog dlg(this, m_panel->m_frame, static_cast<Model_Asset::Data*>(nullptr));
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.m_asset->ASSETID);
        m_panel->m_frame->RefreshNavigationTree();
    }
}

void mmAssetsListCtrl::doRefreshItems(int trx_id)
{
    int selectedIndex = initVirtualListControl(trx_id, m_selected_col, m_asc);

    long cnt = static_cast<long>(m_assets.size());

    if (selectedIndex >= cnt || selectedIndex < 0)
        selectedIndex = m_asc ? cnt - 1 : 0;

    if (cnt>0)
        RefreshItems(0, cnt > 0 ? --cnt : 0);
    else
        selectedIndex = -1;

    if (selectedIndex >= 0 && cnt>0)
    {
        SetItemState(selectedIndex, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
        SetItemState(selectedIndex, wxLIST_STATE_FOCUSED, wxLIST_STATE_FOCUSED);
        EnsureVisible(selectedIndex);
    }
    m_selected_row = selectedIndex;
}

void mmAssetsListCtrl::OnDeleteAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)    return;

    wxMessageDialog msgDlg(this
        , _("Do you really want to delete the Asset?")
        , _("Confirm Asset Deletion")
        , wxYES_NO | wxNO_DEFAULT | wxICON_ERROR);

    if (msgDlg.ShowModal() == wxID_YES)
    {
        const Model_Asset::Data& asset = m_assets[m_selected_row];
        Model_Asset::instance().remove(asset.ASSETID);
        mmAttachmentManage::DeleteAllAttachments(Model_Attachment::reftype_desc(Model_Attachment::ASSET), asset.ASSETID);

        initVirtualListControl(m_selected_row, m_selected_col, m_asc);
        m_selected_row = -1;
        m_panel->updateExtraAssetData(m_selected_row);
    }
}

void mmAssetsListCtrl::OnEditAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)     return;

    wxListEvent evt(wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxID_ANY);
    AddPendingEvent(evt);
}

void mmAssetsListCtrl::OnDuplicateAsset(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0)     return;

    const Model_Asset::Data& asset = m_assets[m_selected_row];
    Model_Asset::Data* duplicate_asset = Model_Asset::instance().clone(&asset);

    if (EditAsset(duplicate_asset))
    {
        initVirtualListControl();
        doRefreshItems(duplicate_asset->ASSETID);
    }
}

void mmAssetsListCtrl::OnOrganizeAttachments(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
    int RefId = m_assets[m_selected_row].ASSETID;

    mmAttachmentDialog dlg(this, RefType, RefId);
    dlg.ShowModal();

    doRefreshItems(RefId);
}

void mmAssetsListCtrl::OnOpenAttachment(wxCommandEvent& /*event*/)
{
    if (m_selected_row < 0) return;

    wxString RefType = Model_Attachment::reftype_desc(Model_Attachment::ASSET);
    int RefId = m_assets[m_selected_row].ASSETID;

    mmAttachmentManage::OpenAttachmentFromPanelIcon(this, RefType, RefId);
    doRefreshItems(RefId);
}

void mmAssetsListCtrl::OnListItemActivated(wxListEvent& event)
{
    if (m_selected_row < 0)
    {
        m_selected_row = event.GetIndex();
    }
    EditAsset(&(m_assets[m_selected_row]));
}

bool mmAssetsListCtrl::EditAsset(Model_Asset::Data* pEntry)
{
    mmAssetDialog dlg(this, m_panel->m_frame, pEntry);
    bool edit = true;
    if (dlg.ShowModal() == wxID_OK)
    {
        doRefreshItems(dlg.m_asset->ASSETID);
        m_panel->updateExtraAssetData(m_selected_row);
    }
    else edit = false;

    return edit;
}

void mmAssetsListCtrl::OnColClick(wxListEvent& event)
{
    int ColumnNr;
    if (event.GetId() != MENU_HEADER_SORT)
         ColumnNr = event.GetColumn();
    else
         ColumnNr = m_ColumnHeaderNbr;
    if (0 > ColumnNr || ColumnNr >= col_max() || ColumnNr == 0) return;

    if (m_selected_col == ColumnNr && event.GetId() != MENU_HEADER_SORT) m_asc = !m_asc;

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(-1);
    SetColumn(m_selected_col, item);

    m_selected_col = ColumnNr;

    item.SetImage(m_asc ? 8 : 7);
    SetColumn(m_selected_col, item);

    Model_Setting::instance().Set("ASSETS_ASC", m_asc);
    Model_Setting::instance().Set("ASSETS_SORT_COL", m_selected_col);

    int trx_id = -1;
    if (m_selected_row>=0) trx_id = m_assets[m_selected_row].ASSETID;

    doRefreshItems(trx_id);
}

void mmAssetsListCtrl::OnEndLabelEdit(wxListEvent& event)
{
    if (event.IsEditCancelled()) return;
    Model_Asset::Data* asset = &m_assets[event.GetIndex()];
    asset->ASSETNAME = event.m_item.m_text;
    Model_Asset::instance().save(asset);
    RefreshItems(event.GetIndex(), event.GetIndex());
}

int mmAssetsListCtrl::initVirtualListControl(int id, int col, bool asc)
{
    /* Clear all the records */
    DeleteAllItems();

    wxListItem item;
    item.SetMask(wxLIST_MASK_IMAGE);
    item.SetImage(asc ? 8 : 7);
    SetColumn(col, item);

    if (m_panel->m_filter_type == Model_Asset::TYPE(-1)) // ALL
        this->m_assets = Model_Asset::instance().find(Model_Asset::ACCOUNTID(m_panel->get_account_id()));
    else
        this->m_assets = Model_Asset::instance().find(
            Model_Asset::ASSETTYPE(m_panel->m_filter_type)
            , Model_Asset::ACCOUNTID(m_panel->get_account_id()));
    this->sortTable();

    SetItemCount(this->m_assets.size());

    double balance = 0.0;
    for (const auto& asset : this->m_assets) balance += Model_Asset::value(asset);
    m_panel->header_text_->SetLabelText(wxString::Format(_("Total: %s"), Model_Currency::toCurrency(balance))); // balance

    int selected_item = 0;
    for (const auto& asset : this->m_assets)
    {
        if (asset.ASSETID == id) return selected_item;
        ++selected_item;
    }
    return -1;
}

void mmAssetsListCtrl::sortTable()
{
    std::sort(this->m_assets.begin(), this->m_assets.end());
    std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterBySTARTDATE());
    switch (m_selected_col)
    {
    case COL_ID:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByASSETID());
        break;
    case COL_NAME:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByASSETNAME());
        break;
    case COL_TYPE:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByASSETTYPE());
        break;
    case COL_VALUE_INITIAL:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByVALUE());
        break;
    case COL_VALUE_CURRENT:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end()
            , [](const Model_Asset::Data& x, const Model_Asset::Data& y)
        {
            return Model_Asset::value(x) < Model_Asset::value(y);
        });
        break;
    case COL_DATE:
        break;
    case COL_NOTES:
        std::stable_sort(this->m_assets.begin(), this->m_assets.end(), SorterByNOTES());
    default:
        break;
    }

    if (!m_asc) std::reverse(this->m_assets.begin(), this->m_assets.end());
}


wxString mmAssetsListCtrl::getItem(long item, long column) const
{
    const Model_Asset::Data& asset = this->m_assets[item];
    switch (column)
    {
    case COL_ICON:
        return " ";
    case COL_ID:
        return wxString::Format("%i", asset.ASSETID).Trim();
    case COL_NAME:
        return asset.ASSETNAME;
    case COL_TYPE:
        return wxGetTranslation(asset.ASSETTYPE);
    case COL_VALUE_INITIAL:
        return Model_Currency::toCurrency(asset.VALUE);
    case COL_VALUE_CURRENT:
        return Model_Currency::toCurrency(Model_Asset::value(asset));
    case COL_DATE:
        return mmGetDateForDisplay(asset.STARTDATE);
    case COL_NOTES:
    {
        wxString full_notes = asset.NOTES;
        if (Model_Attachment::NrAttachments(Model_Attachment::reftype_desc(Model_Attachment::ASSET), asset.ASSETID))
            full_notes = full_notes.Prepend(mmAttachmentManage::GetAttachmentNoteSign());
        return full_notes;
    }
    default:
        return "";
    }
}

void mmAssetsListCtrl::doSearchTxt(const wxString& search_string)
{
    if (search_string.IsEmpty()) return;

    long last = GetItemCount();
    long selectedItem = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selectedItem < 0) //nothing selected
        selectedItem = m_asc ? last - 1 : 0;

    while (selectedItem > 0 && selectedItem <= last)
    {
        m_asc ? selectedItem-- : selectedItem++;
        const wxString t = getItem(selectedItem, COL_NOTES).Lower();
        if (t.Matches(search_string + "*"))
        {
            //First of all any items should be unselected
            long cursel = GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
            if (cursel != wxNOT_FOUND)
                SetItemState(cursel, 0, wxLIST_STATE_SELECTED | wxLIST_STATE_FOCUSED);

            //Then finded item will be selected
            SetItemState(selectedItem, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
            EnsureVisible(selectedItem);
            break;
        }
    }

}

/*******************************************************/
BEGIN_EVENT_TABLE(mmAssetsPanel, wxPanel)
    EVT_BUTTON(wxID_NEW, mmAssetsPanel::OnNewAsset)
    EVT_BUTTON(wxID_EDIT, mmAssetsPanel::OnEditAsset)
    EVT_BUTTON(wxID_DELETE, mmAssetsPanel::OnDeleteAsset)
    EVT_BUTTON(wxID_FILE, mmAssetsPanel::OnOpenAttachment)
    EVT_BUTTON(wxID_FILE2, mmAssetsPanel::OnMouseLeftDown)
    EVT_MENU(wxID_ANY, mmAssetsPanel::OnViewPopupSelected)
    EVT_SEARCHCTRL_SEARCH_BTN(wxID_FIND, mmAssetsPanel::OnSearchTxtEntered)
    EVT_TEXT_ENTER(wxID_FIND, mmAssetsPanel::OnSearchTxtEntered)
    EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, mmAssetsPanel::OnNotebookPageChanged)
END_EVENT_TABLE()
/*******************************************************/

mmAssetsPanel::mmAssetsPanel(mmGUIFrame* frame
, wxWindow* parent
, int account_id
, wxWindowID winid)
    : m_filter_type(Model_Asset::TYPE(-1))
    , m_frame(frame)
    , m_account_id(account_id)
    , m_view_mode(0)
    , m_assets_list(nullptr)
    , m_bitmapTransFilter(nullptr)
    , header_text_(nullptr)
    , tips_()
{
    Create(parent, winid);
}

bool mmAssetsPanel::Create(wxWindow *parent
    , wxWindowID winid
    , const wxPoint &pos
    , const wxSize &size
    , long style
    , const wxString &name)
{
    SetExtraStyle(GetExtraStyle() | wxWS_EX_BLOCK_EVENTS);

    if (!wxPanel::Create(parent, winid, pos, size, style, name)) return false;

    this->windowsFreezeThaw();

    tips_ = _("MMEX allows you to track fixed assets like cars, houses, land and others. Each asset can have its value appreciate by a certain rate per year, depreciate by a certain rate per year, or not change in value. The total assets are added to your total financial worth.");
    CreateControls();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    dataToControls();

    this->windowsFreezeThaw();
    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    Model_Usage::instance().pageview(this);

    return true;
}

void mmAssetsPanel::CreateControls()
{
    wxBoxSizer* itemBoxSizer9 = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(itemBoxSizer9);

    wxPanel* headerPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition
        , wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);
    itemBoxSizer9->Add(headerPanel, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerVHeader = new wxBoxSizer(wxVERTICAL);
    headerPanel->SetSizer(itemBoxSizerVHeader);

    wxStaticText* itemStaticText9 = new wxStaticText(headerPanel, wxID_STATIC, _("Assets"));
    itemStaticText9->SetFont(this->GetFont().Larger().Bold());
    itemBoxSizerVHeader->Add(itemStaticText9, g_flagsBorder1V);

    wxBoxSizer* itemBoxSizerHHeader2 = new wxBoxSizer(wxHORIZONTAL);
    itemBoxSizerVHeader->Add(itemBoxSizerHHeader2);

    m_bitmapTransFilter = new wxButton(headerPanel, wxID_FILE2);
    m_bitmapTransFilter->SetBitmap(mmBitmap(png::RIGHTARROW));
    m_bitmapTransFilter->SetLabel(_("All"));
    m_bitmapTransFilter->SetMinSize(wxSize(150, -1));
    itemBoxSizerHHeader2->Add(m_bitmapTransFilter, g_flagsBorder1H);

    header_text_ = new wxStaticText(headerPanel, wxID_STATIC, "");
    itemBoxSizerVHeader->Add(header_text_, g_flagsBorder1V);

    /* ---------------------- */

    wxSplitterWindow* splitter_window = new wxSplitterWindow(this
        , wxID_STATIC, wxDefaultPosition, wxSize(200, 200)
        , wxSP_3DBORDER | wxSP_3DSASH | wxNO_BORDER);

    m_notebook = new wxNotebook(splitter_window, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_MULTILINE);
    wxPanel* assets_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(assets_tab, _("Assets"));
    wxBoxSizer *assets_sizer = new wxBoxSizer(wxVERTICAL);
    assets_tab->SetSizer(assets_sizer);

    wxPanel* money_tab = new wxPanel(m_notebook, wxID_ANY);
    m_notebook->AddPage(money_tab, _("Money"));

    m_assets_list = new mmAssetsListCtrl(this, assets_tab, wxID_ANY);
    assets_sizer->Add(m_assets_list, g_flagsExpand);

    wxBoxSizer *money_sizer = new wxBoxSizer(wxVERTICAL);
    money_tab->SetSizer(money_sizer);

    wxListCtrl* m_listCtrlMoney = new wxListCtrl(money_tab, wxID_ANY);
    m_listCtrlMoney->SetMinSize(wxSize(500, 150));

    money_sizer->Add(m_listCtrlMoney, g_flagsExpand);

    //
    wxPanel* bottom_panel = new wxPanel(splitter_window, wxID_ANY
        , wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTAB_TRAVERSAL);

    splitter_window->SplitHorizontally(m_notebook, bottom_panel);
    splitter_window->SetMinimumPaneSize(100);
    splitter_window->SetSashGravity(1.0);
    itemBoxSizer9->Add(splitter_window, g_flagsExpandBorder1);

    wxBoxSizer* BoxSizerVBottom = new wxBoxSizer(wxVERTICAL);
    bottom_panel->SetSizer(BoxSizerVBottom);

    wxBoxSizer* BoxSizerHBottom = new wxBoxSizer(wxHORIZONTAL);
    BoxSizerVBottom->Add(BoxSizerHBottom, g_flagsBorder1V);

    wxButton* itemButton6 = new wxButton(bottom_panel, wxID_NEW, _("&New "));
    itemButton6->SetToolTip(_("New Asset"));
    BoxSizerHBottom->Add(itemButton6, 0, wxRIGHT, 5);

    wxButton* itemButton81 = new wxButton(bottom_panel, wxID_EDIT, _("&Edit "));
    itemButton81->SetToolTip(_("Edit Asset"));
    BoxSizerHBottom->Add(itemButton81, 0, wxRIGHT, 5);
    itemButton81->Enable(false);

    wxButton* itemButton7 = new wxButton(bottom_panel, wxID_DELETE, _("&Delete "));
    itemButton7->SetToolTip(_("Delete Asset"));
    BoxSizerHBottom->Add(itemButton7, 0, wxRIGHT, 5);
    itemButton7->Enable(false);

    wxBitmapButton* attachment_button_ = new wxBitmapButton(bottom_panel
        , wxID_FILE, mmBitmap(png::CLIP), wxDefaultPosition,
        wxSize(30, itemButton7->GetSize().GetY()));
    attachment_button_->SetToolTip(_("Open attachments"));
    BoxSizerHBottom->Add(attachment_button_, 0, wxRIGHT, 5);
    attachment_button_->Enable(false);

    wxSearchCtrl* searchCtrl = new wxSearchCtrl(bottom_panel
        , wxID_FIND, wxEmptyString, wxDefaultPosition
        , wxSize(100, itemButton7->GetSize().GetHeight())
        , wxTE_PROCESS_ENTER, wxDefaultValidator, _("Search"));
    searchCtrl->SetHint(_("Search"));
    BoxSizerHBottom->Add(searchCtrl, 0, wxCENTER, 1);
    searchCtrl->SetToolTip(_("Enter any string to find related assets"));

    //Infobar-mini
    wxStaticText* itemStaticText44 = new wxStaticText(bottom_panel, IDC_PANEL_ASSET_STATIC_DETAILS_MINI, "");
    BoxSizerHBottom->Add(itemStaticText44, 1, wxGROW | wxTOP | wxLEFT, 5);

    //Infobar
    wxStaticText* itemStaticText33 = new wxStaticText(bottom_panel
        , IDC_PANEL_ASSET_STATIC_DETAILS, "", wxDefaultPosition, wxSize(200, -1), wxTE_MULTILINE | wxTE_WORDWRAP);
    BoxSizerHBottom->Add(itemStaticText33, g_flagsExpandBorder1);

    updateExtraAssetData(-1);
}

void mmAssetsPanel::OnDeleteAsset(wxCommandEvent& event)
{
    m_assets_list->OnDeleteAsset(event);
}

void mmAssetsPanel::OnNewAsset(wxCommandEvent& event)
{
    m_assets_list->OnNewAsset(event);
}

void mmAssetsPanel::OnEditAsset(wxCommandEvent& event)
{
    m_assets_list->OnEditAsset(event);
}

void mmAssetsPanel::OnOpenAttachment(wxCommandEvent& event)
{
    m_assets_list->OnOpenAttachment(event);
}

void mmAssetsPanel::updateExtraAssetData(int selIndex)
{
    wxStaticText* st = static_cast<wxStaticText*>(FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS));
    wxStaticText* stm = static_cast<wxStaticText*>(FindWindow(IDC_PANEL_ASSET_STATIC_DETAILS_MINI));
    if (selIndex > -1)
    {
        const Model_Asset::Data& asset = m_assets_list->get_m_assets()[selIndex];
        enableEditDeleteButtons(true);
        const auto& change_rate = (Model_Asset::rate(asset) == Model_Asset::RATE_PERCENTAGE)
            ? wxString::Format("%.2f %%", asset.VALUECHANGERATE)
            : wxString::Format("%.2f", asset.VALUECHANGERATE);
        const wxString& miniInfo = " " + wxString::Format(_("Change in Value: %s")
            , change_rate);

        st->SetLabelText(asset.NOTES);
        stm->SetLabelText(miniInfo);
    }
    else
    {
        stm->SetLabelText("");
        st->SetLabelText(this->tips_);
        enableEditDeleteButtons(false);
    }
}

void mmAssetsPanel::enableEditDeleteButtons(bool enable)
{
    wxButton* btn = static_cast<wxButton*>(FindWindow(wxID_EDIT));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_NEW));
    if (btn) btn->Enable(!enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_ADD));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_DELETE));
    if (btn) btn->Enable(enable);

    btn = static_cast<wxButton*>(FindWindow(wxID_FILE));
    if (btn) btn->Enable(enable);
}

void mmAssetsPanel::OnMouseLeftDown (wxCommandEvent& event)
{
    int i = 0;
    wxMenu menu;
    menu.Append(++i, wxGetTranslation(wxTRANSLATE("All")));

    for (const auto& type: Model_Asset::all_type())
    {
        menu.Append(++i, wxGetTranslation(type));
    }
    PopupMenu(&menu);

    event.Skip();
}

void mmAssetsPanel::OnViewPopupSelected(wxCommandEvent& event)
{
    int evt = std::max(event.GetId() - 1, 0);

    if (evt == 0)
    {
        m_bitmapTransFilter->SetLabel(_("All"));
        this->m_filter_type = Model_Asset::TYPE(-1);
    }
    else
    {
        this->m_filter_type = Model_Asset::TYPE(evt - 1);
        m_bitmapTransFilter->SetLabel(wxGetTranslation(Model_Asset::all_type()[evt - 1]));
    }

    int trx_id = -1;
    m_assets_list->doRefreshItems(trx_id);
    updateExtraAssetData(trx_id);
}

void mmAssetsPanel::OnSearchTxtEntered(wxCommandEvent& event)
{
    const wxString search_string = event.GetString().Lower();
    m_assets_list->doSearchTxt(search_string);
}

void mmAssetsPanel::AddAssetTrans(const int selected_index)
{
    Model_Asset::Data* asset = &m_assets_list->get_m_assets()[selected_index];
    mmAssetDialog asset_dialog(this, m_frame, asset);

    if (asset_dialog.ShowModal() == wxID_OK)
    {
        m_assets_list->doRefreshItems(selected_index);
        updateExtraAssetData(selected_index);
    }
}

void mmAssetsPanel::OnNotebookPageChanged(wxBookCtrlEvent & event)
{
    m_view_mode = event.GetSelection();
    wxLogDebug("%i Mode", m_view_mode);
}

void mmAssetsPanel::RefreshList(int transID)
{
    dataToControls(transID);
}

void mmAssetsPanel::dataToControls(int transID)
{
    m_assets_list->initVirtualListControl(transID);
    //m_listCtrlMoney->initVirtualListControl(transID);
}

void mmAssetsPanel::DisplayAccountDetails(int accountID)
{
    m_account_id = accountID;
    m_view_mode = 0;
    m_notebook->SetSelection(m_view_mode);
    RefreshList();
}

inline void mmAssetsPanel::sortTable() {}
