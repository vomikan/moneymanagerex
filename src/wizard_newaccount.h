/*******************************************************
 Copyright (C) 2006 Madhan Kanagavel

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

#ifndef MM_EX_WIZARD_NEWACCOUNT_H_
#define MM_EX_WIZARD_NEWACCOUNT_H_

#include <wx/wizard.h>
#include <wx/frame.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
//----------------------------------------------------------------------------

class mmAddAccountWizard : public wxWizard
{
public:
    mmAddAccountWizard(wxFrame *frame);
    void RunIt();
    const wxString get_account_name() const;
    void set_account_name(const wxString & v);
    int get_currency_id() const;
    void set_currency_id(int v);
    int get_account_type() const;
    void set_account_type(int v);
    int get_account_id() const;
    void set_account_id(int v);

private:
    wxWizardPageSimple* page1;

    wxString accountName_;
    int currencyID_;
    int accountType_;
    int acctID_;

};
//----------------------------------------------------------------------------

class mmAddAccountNamePage : public wxWizardPageSimple
{
public:
    mmAddAccountNamePage(mmAddAccountWizard* parent);
    virtual bool TransferDataFromWindow();

private:
    mmAddAccountWizard* parent_;
    wxTextCtrl* textAccountName_;
};
//----------------------------------------------------------------------------

class mmAddAccountTypePage : public wxWizardPageSimple
{
public:
    mmAddAccountTypePage(mmAddAccountWizard *parent);
    virtual bool TransferDataFromWindow();

private:
    wxChoice* itemChoiceType_;
    mmAddAccountWizard* parent_;
};

inline const wxString mmAddAccountWizard::get_account_name() const { return accountName_; }
inline void mmAddAccountWizard::set_account_name(const wxString& v) { accountName_ = v; }
inline int mmAddAccountWizard::get_currency_id() const { return currencyID_; }
inline void mmAddAccountWizard::set_currency_id(int v) { currencyID_ = v; }
inline int mmAddAccountWizard::get_account_type() const { return accountType_; }
inline void mmAddAccountWizard::set_account_type(int v) { accountType_ = v; }
inline int mmAddAccountWizard::get_account_id() const { return acctID_; }
inline void mmAddAccountWizard::set_account_id(int v) { acctID_ = v; }

#endif // MM_EX_WIZARD_NEWACCOUNT_H_

