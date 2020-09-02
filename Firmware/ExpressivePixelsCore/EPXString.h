// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.
#pragma once
#include <string.h>
#include <stdlib.h>
#include <EPXPlatform_Runtime.h>



class EPXString
{
public:
	EPXString()
	{
		m_len = 0;
		m_psz = NULL;
	}
	
	
	
	EPXString(const EPXString &__str)
	{
		assign(__str);
	}
	
	
	
	~EPXString()
	{
		if (m_psz != NULL)
		{			
			delete m_psz;
			m_psz = NULL;
		}
	}

	
	
	EPXString(const char* __s)
	{
		assign(__s);
	}
	

	
	EPXString(char* psz)
	{
		assign(psz);
	}

	
	
	EPXString(int val)
	{
		char szFmt[16];		
		itoa(val, szFmt, 10);
		assign(szFmt);
	}

	
	
	
	const char* c_str() { return m_psz; }
	
	
	const int length() { return m_len; }
	
	
	EPXString& append(const EPXString& __str)
	{		
		return append(__str.m_psz);
	}
		
	
		
	EPXString& operator+=(const EPXString& __str)
	{ 
		return this->append(__str); 
	}

	
	
	EPXString& operator+(const EPXString& __str)
	{ 
		return this->append(__str); 
	}


	
	EPXString& operator+=(const char *__str)
	{ 
		return this->append(__str); 
	}


	
	EPXString& operator+=(char *psz)
	{ 
		return this->append(psz); 
	}

	
	
	EPXString&	operator=(const EPXString& __s)		
	{ 
		return assign(__s); 
	}
	
	
	
	EPXString&	operator=(char *psz)		
	{ 
		return assign(psz); 
	}

	
	
	EPXString& append(const char *psz)
	{	
		char *pszNew;
		int __len = strlen(psz) + this->m_len;
	
		pszNew = (char *) new char[__len + 1];
		*pszNew = 0x00;
		
		if (m_psz != NULL)
			strcpy(pszNew, m_psz);
		strcat(pszNew, psz);
		
		if (m_psz != NULL)
			delete m_psz;
		m_psz = pszNew;	
		m_len = __len;		
		return *this;
	}
	
	
	
private:

	EPXString& assign(const EPXString& __str)
	{
		m_len = __str.m_len;
		m_psz = (char *) new char[m_len + 1];
		if (m_psz != NULL)
			strcpy(m_psz, __str.m_psz);
		return *this;
	}

	
	
	EPXString& assign(const char *psz)
	{
		m_len = strlen(psz);
		m_psz = (char *) new char[m_len + 1];
		if (m_psz != NULL)
			strcpy(m_psz, psz);
		return *this;
	}

	
	
	char *m_psz;
	int m_len;
};

