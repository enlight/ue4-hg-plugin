//-------------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2014 Vadim Macagon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-------------------------------------------------------------------------------

#include "MercurialSourceControlPrivatePCH.h"
#include "MercurialSourceControlProviderSettings.h"
#include "SourceControlHelpers.h"

namespace MercurialSourceControl {
	
namespace Settings {
	const TCHAR* Section = TEXT("MercurialSourceControl.ProviderSettings");
	const TCHAR* MercurialPath = TEXT("MercurialPath");
} // unnamed namespace


const FString& FProviderSettings::GetMercurialPath() const
{
	FScopeLock ScopeLock(&CriticalSection);
	return MercurialPath;
}

void FProviderSettings::SetMercurialPath(const FString& InMercurialPath)
{
	FScopeLock ScopeLock(&CriticalSection);
	MercurialPath = InMercurialPath;
}

void FProviderSettings::Save()
{
	FScopeLock ScopeLock(&CriticalSection);
	const FString& SettingsFile = SourceControlHelpers::GetSettingsIni();
	if (GConfig)
	{
		GConfig->SetString(Settings::Section, Settings::MercurialPath, *MercurialPath, SettingsFile);
	}
}

void FProviderSettings::Load()
{
	FScopeLock ScopeLock(&CriticalSection);
	const FString& SettingsFile = SourceControlHelpers::GetSettingsIni();
	if (GConfig)
	{
		GConfig->GetString(Settings::Section, Settings::MercurialPath, MercurialPath, SettingsFile);
	}
}

} // namespace MercurialSourceControl
