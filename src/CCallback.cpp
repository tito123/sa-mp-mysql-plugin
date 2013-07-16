#pragma once

#include "main.h"
#include "CCallback.h"
#include "CMySQLHandle.h"
#include "CMySQLQuery.h"
#include "CMySQLResult.h"
#include "CLog.h"


boost::lockfree::queue<
		CMySQLQuery*, 
		boost::lockfree::fixed_sized<true>,
		boost::lockfree::capacity<10000>
	> CCallback::CallbackQueue;

list<AMX *> CCallback::AmxList;


void CCallback::ProcessCallbacks() {
	CMySQLQuery *Query = NULL;
	while( (Query = GetNextQuery()) != NULL) {
		CCallback *Callback = Query->Callback;
		 
		if(Callback != NULL && Callback->Name.length() > 0) { 

			bool PassByReference = Query->Callback->IsInline;

			for (list<AMX *>::iterator a = AmxList.begin(), end = AmxList.end(); a != end; ++a) { 
				cell amx_Ret;
				int amx_Index;
				cell amx_MemoryAddress = -1;

				if (amx_FindPublic( (*a), Callback->Name.c_str(), &amx_Index) == AMX_ERR_NONE) { 
						
					CLog::Get()->StartCallback(Callback->Name.c_str());

					int StringIndex = Callback->ParamFormat.length()-1; 
					while(!Callback->Parameters.empty() && StringIndex >= 0) {
						switch(Callback->ParamFormat.at(StringIndex)) {
							case 'i':
							case 'd': {
								int val = atoi(Callback->Parameters.top().c_str());

								if(PassByReference == false)
									amx_Push( (*a), (cell)val);
								else {
									cell tmpAddress;
									amx_PushArray( (*a), &tmpAddress, NULL, (cell*)&val, 1);
									if(amx_MemoryAddress < NULL)
										amx_MemoryAddress = tmpAddress;
								}
							} break;

							case 'f': { 
								float FParam = (float)atof(Callback->Parameters.top().c_str());
								if(PassByReference == false)
									amx_Push( (*a), amx_ftoc(FParam));
								else {
									cell tmpAddress;
									amx_PushArray( (*a), &tmpAddress, NULL, (cell*)&amx_ftoc(FParam), 1);
									if(amx_MemoryAddress < NULL)
										amx_MemoryAddress = tmpAddress;
								}

							} break;
							
							default: {
								cell tmpAddress;
								amx_PushString( (*a), &tmpAddress, NULL, Callback->Parameters.top().c_str(), 0, 0);
								if(amx_MemoryAddress < NULL)
										amx_MemoryAddress = tmpAddress;
							}
						}


						StringIndex--;
						Callback->Parameters.pop();
					}

					CMySQLResult *Result = Query->Result;
					Query->ConnHandle->SetNewResult(Result);

					amx_Exec( (*a), &amx_Ret, amx_Index);
					if (amx_MemoryAddress >= NULL)
						amx_Release( (*a), amx_MemoryAddress);


					if(Query->ConnHandle->IsActiveResultSaved() == true)
						Query->ConnHandle->SetNewResult(NULL);

					if(Query->ConnHandle->GetResult() != NULL) 
						delete Result;

					Query->ConnHandle->SetNewResult(NULL);
					
					CLog::Get()->EndCallback();

				}
			}
		}
		delete Callback;
		delete Query;
	}
}



void CCallback::AddQueryToQueue( CMySQLQuery *cb )
{
	CallbackQueue.push(cb);
}

CMySQLQuery * CCallback::GetNextQuery()
{
	CMySQLQuery *NextQuery = NULL;
	CallbackQueue.pop(NextQuery);
	return NextQuery;
}

void CCallback::AddAmx( AMX *amx )
{
	AmxList.push_back(amx);
}

void CCallback::EraseAmx( AMX *amx )
{
	for (list<AMX *>::iterator a = AmxList.begin(); a != AmxList.end(); ++a) {
		if (( *a) == amx) {
			AmxList.erase(a);
			break;
		}
	}
}

void CCallback::ClearAll()
{
	CMySQLQuery *tmpQuery = NULL;
	while(CallbackQueue.pop(tmpQuery)) {
		delete tmpQuery->Callback;
		delete tmpQuery->Result;
		delete tmpQuery;
		tmpQuery = NULL;
	}
}


CCallback::CCallback( const CCallback &rhs )
{
	Parameters = rhs.Parameters;
	Name = rhs.Name;
	ParamFormat = rhs.ParamFormat;
	IsInline = rhs.IsInline;
}

void CCallback::operator=( const CCallback &rhs )
{
	Parameters = rhs.Parameters;
	Name = rhs.Name;
	ParamFormat = rhs.ParamFormat;
	IsInline = rhs.IsInline;
}
