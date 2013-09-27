/*************************************************************************
    > File Name: CallPython.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月17日 星期二 09时30分50秒
 ************************************************************************/

#include "callPython.h"

CallPython::CallPython()
{
	pName = NULL;
	pModule = NULL;
	pFunc = NULL;
	pArgs = NULL;
}

CallPython::~CallPython()
{
	if(!pModule)
		Py_DECREF(pModule);
	if(!pName)
		Py_DECREF(pName);
}

bool CallPython::LoadFile(char *fileName)
{
	pName = PyString_FromString(fileName);
	pModule = PyImport_Import(pName);
	if(!pModule)
		return false;
	return true;
}

bool CallPython::GetFunc(char *funName)
{
	if(!pModule)
		return false;
//	PyObject *pDict = PyModule_GetDict(pModule);
//	if(!pDict)
//		return false;
//	pFunc = PyDict_GetItemString(pDict,funName);
	pFunc = PyObject_GetAttrString(pModule,funName);
//	Py_DECREF(pDict);
	if(pFunc && PyCallable_Check(pFunc))
		return true;
	return false;
}

bool CallPython::Invoke()
{
	if(pFunc && pArgs)
		pValue = PyObject_CallObject(pFunc,pArgs);
	if(pValue != Py_True)
		return false;
	DeleteArgs();
	return true;
}


void CallPython::BuildArgs(int argc,std::string *argv)
{
	pArgs = PyTuple_New(argc);
	int i;
	for(i = 0; i < argc; i++)
	{
		PyObject *pyValue = PyString_FromString(argv[i].c_str());
		PyTuple_SetItem(pArgs,i,pyValue);
	}
}

void CallPython::DeleteArgs()
{
	if(!pFunc)
		Py_DECREF(pFunc);
	if(!pArgs)
		Py_DECREF(pArgs);
/*	if(!pModule)
		Py_DECREF(pModule);
	if(!pName)
		Py_DECREF(pName);*/
	if(!pValue)
		Py_DECREF(pValue);
}
