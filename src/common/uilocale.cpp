///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/uilocale.cpp
// Purpose:     wxUILocale implementation
// Author:      Vadim Zeitlin
// Created:     2021-07-31
// Copyright:   (c) 2021 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

// for compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#if wxUSE_INTL

#include "wx/uilocale.h"

#include "wx/private/uilocale.h"

// ----------------------------------------------------------------------------
// global variables
// ----------------------------------------------------------------------------

// This static global variable doesn't need to be protected from concurrent
// access as it's only supposed to be used from the UI thread.
/* static */
wxUILocale wxUILocale::ms_current;

// ============================================================================
// implementation
// ============================================================================

#ifndef __WINDOWS__

/* static */
wxUILocaleImpl* wxUILocaleImpl::CreateForLanguage(const wxLanguageInfo& info)
{
    wxLocaleIdent locId;

    // Strings in our language database are of the form "lang[_region[@mod]]".
    wxString rest;
    locId.Language(info.CanonicalName.BeforeFirst('_', &rest));

    if ( !rest.empty() )
    {
        wxString mod;
        locId.Region(rest.BeforeFirst('@', &mod));

        if ( !mod.empty() )
            locId.Modifier(mod);
    }

    return CreateForLocale(locId);
}

#endif // !__WINDOWS__

/* static */
bool wxUILocale::UseDefault()
{
    // We don't attempt to optimize this function by checking whether
    // ms_current is already set to the user default locale, as we're
    // supposed to be called just once during the program lifetime anyhow.

    wxUILocaleImpl* const impl = wxUILocaleImpl::CreateUserDefault();
    if ( !impl )
        return false;

    impl->Use();
    ms_current.SetImpl(impl);

    return true;
}

/* static */
bool wxUILocale::UseLanguage(const wxLanguageInfo& info)
{
    wxUILocaleImpl* const impl = wxUILocaleImpl::CreateForLanguage(info);
    if ( !impl )
        return false;

    impl->Use();
    ms_current.SetImpl(impl);

    return true;
}

/* static */
const wxUILocale& wxUILocale::GetCurrent()
{
    // We initialize it on demand.
    if ( !ms_current.m_impl )
    {
        ms_current.SetImpl(wxUILocaleImpl::CreateStdC());
    }

    return ms_current;
}

wxUILocale::wxUILocale(const wxLocaleIdent& localeId)
{
    m_impl = wxUILocaleImpl::CreateForLocale(localeId);
}

void wxUILocale::SetImpl(wxUILocaleImpl* impl)
{
    delete m_impl;

    m_impl = impl;
}

wxString wxUILocale::GetName() const
{
    if ( !m_impl )
        return wxString();

    return m_impl->GetName();
}

wxString wxUILocale::GetInfo(wxLocaleInfo index, wxLocaleCategory cat) const
{
    if ( !m_impl )
        return wxGetStdCLocaleInfo(index, cat);

    return m_impl->GetInfo(index, cat);
}

int
wxUILocale::CompareStrings(const wxString& lhs,
                           const wxString& rhs,
                           int flags) const
{
    if ( !m_impl )
    {
        const int rc = flags & wxCompare_CaseInsensitive ? lhs.CmpNoCase(rhs)
                                                         : lhs.Cmp(rhs);
        if ( rc < 0 )
            return -1;
        if ( rc > 0 )
            return 1;
        return 0;
    }

    return m_impl->CompareStrings(lhs, rhs, flags);
}

wxUILocale::~wxUILocale()
{
    delete m_impl;
}

#endif // wxUSE_INTL
