/*************************************************************************
    > File Name: callPython.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月17日 星期二 09时24分09秒
 ************************************************************************/

#include <python2.6/Python.h>
#include <string>

bool InitPythonEnv()
{
	Py_Initialize();
	if(!Py_IsInitialized())
		return false;
	return true;
}
void UnInitPythonEnv()
{
	Py_Finalize();
}
bool RunScript(char *script)
{
	int ret = PyRun_SimpleString(script);
	return ret == 0;
}

class CallPython
{
public:
	CallPython();
public:
	~CallPython();
	bool Invoke();
	void BuildArgs(int argc,std::string *argv);
	bool LoadFile(char *fileName);
	bool GetFunc(char *funName);
private:
	void DeleteArgs();
private:
	PyObject *pName,*pModule,*pFunc,*pArgs,*pValue;
};
