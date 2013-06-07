#pragma once

#include "main.h"
#include "CCallback.h"
#include "CMySQLHandler.h"

CMutex CCallback::CallbackMutex;
queue<CMySQLQuery*> CCallback::CallbackQueue;

void CCallback::ProcessCallbacks() {
	CMySQLQuery *Query = NULL;
	while( (Query = GetNextQuery()) != NULL) {
		CCallback *Callback = Query->Callback;
		 
		if(Callback != NULL && Callback->Name.length() > 0) { 
			for (list<AMX *>::iterator a = p_Amx.begin(), end = p_Amx.end(); a != end; ++a) { 
				cell amx_Ret;
				int amx_Index;
				cell amx_Address = -1;
				if (amx_FindPublic( (*a), Callback->Name.c_str(), &amx_Index) == AMX_ERR_NONE) { 
						
					Native::Log(LOG_DEBUG, "%s(%s) - Callback is being called..", Callback->Name.c_str(), Callback->ParamFormat.c_str());
					
					int StringIndex = Callback->ParamFormat.length()-1; 
					while(!Callback->Parameters.empty() && StringIndex >= 0) {
						switch(Callback->ParamFormat.at(StringIndex)) {
							case 'i':
							case 'd': {
								int val = atoi(Callback->Parameters.top().c_str());
								amx_Push( (*a), (cell)val);
							} break;

							case 'f': { 
								float FParam = (float)atof(Callback->Parameters.top().c_str());
								amx_Push( (*a), amx_ftoc(FParam));

							} break;
							
							default: {
								cell TmpAddress;
								amx_PushString( (*a), &TmpAddress, NULL, Callback->Parameters.top().c_str(), 0, 0);
								if(amx_Address < NULL)
									amx_Address = TmpAddress;
							}
						}


						StringIndex--;
						Callback->Parameters.pop();
					}

					CMySQLResult *Result = Query->Result;
					Query->ConnHandle->SetNewResult(Result);

					amx_Exec(* a, &amx_Ret, amx_Index);
					if (amx_Address >= NULL)
						amx_Release( (*a), amx_Address);
					
					if(!Query->ConnHandle->IsActiveResultSaved())
						delete Result;
					Query->ConnHandle->SetNewResult(NULL);
					
					Native::Log(LOG_DEBUG, "ProcessCallbacks() - The result has been cleared.");

				}
			}
		}
		delete Callback;
		delete Query;
	}
}
