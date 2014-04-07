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
#include "MercurialSourceControlModule.h"
#include "Features/IModularFeatures.h"
#include "MercurialSourceControlOperationNames.h"
#include "MercurialSourceControlWorkers.h"

namespace MercurialSourceControl {

namespace 
{
	const char* SourceControl = "SourceControl";

	template<typename T>
	FWorkerRef CreateWorker()
	{
		return MakeShareable(new T());
	}	
} // unnamed namespace

void FModule::StartupModule()
{
	Provider.RegisterWorkerCreator(
		OperationNames::Connect, 
		FCreateWorkerDelegate::CreateStatic(&CreateWorker<FConnectWorker>)
	);
	Provider.RegisterWorkerCreator(
		OperationNames::UpdateStatus, 
		FCreateWorkerDelegate::CreateStatic(&CreateWorker<FUpdateStatusWorker>)
	);
	Provider.RegisterWorkerCreator(
		OperationNames::Revert, 
		FCreateWorkerDelegate::CreateStatic(&CreateWorker<FRevertWorker>)
	);
	Provider.RegisterWorkerCreator(
		OperationNames::Delete, 
		FCreateWorkerDelegate::CreateStatic(&CreateWorker<FDeleteWorker>)
	);
	Provider.RegisterWorkerCreator(
		OperationNames::MarkForAdd, 
		FCreateWorkerDelegate::CreateStatic(&CreateWorker<FMarkForAddWorker>)
	);
	
	IModularFeatures::Get().RegisterModularFeature(SourceControl, &Provider);
}

void FModule::ShutdownModule()
{
	Provider.Close();
	IModularFeatures::Get().UnregisterModularFeature(SourceControl, &Provider);
}

bool FModule::IsGameModule() const
{
	// no gameplay code in this module
	return false;
}

FProvider& FModule::GetProvider()
{
	FModule& MercurialModule = FModuleManager::LoadModuleChecked<FModule>("MercurialSourceControl");
	return MercurialModule.Provider;
}

} // namespace MercurialSourceControl

IMPLEMENT_MODULE(MercurialSourceControl::FModule, MercurialSourceControl);