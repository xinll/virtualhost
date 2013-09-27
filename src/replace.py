#!/usr/bin/python
from string import Template

class MyTemplate(Template):
	"""docstring for MyTemplate"""
	delimiter ='?'

def doReplaceHostFile(oldFile,newFile,ftpUser,defaultDomain,max_trans,max_conn):
	dict = {'ftpuser':ftpUser,'defaultdomain':defaultDomain,'max_trans':max_trans,'max_conn':max_conn}
	return doReplace(oldFile,newFile,dict)


def doReplaceCGIFile(oldFile,newFile,ftpUser):
	dict = {'ftpuser':ftpUser}
	return doReplace(oldFile,newFile,dict)

def doReplacePhpFile(oldFile,newFile,ftpUser):
	dict = {'ftpuser':ftpuser}
	return doReplace(oldFile,newFile,dict)

def doReplace(oldFile,newFile,dict):
	with open(oldFile,'r') as old:
		str = old.read()
	
	if(str.count == 0):
		return False

	with open(newFile,'w') as new:
		new.write(str)
	
	return True

if __name__ == '__main__':
	doReplaceCGIFile("php-cgi.sample",'php-cgi','willkyd')
