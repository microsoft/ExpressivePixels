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
            Reset();
	}
	
	
	
	EPXString(const EPXString &__str)
	{
                Reset();
		assign(__str);
	}
	
	
	
	
	EPXString(const char* __s)
	{
                Reset();
		assign(__s);
	}
	

	
	EPXString(char* psz)
	{
                Reset();
		assign(psz);
	}

	
	
	EPXString(int val)
	{
                Reset();

		char szFmt[16];		
		itoa(val, szFmt, 10);
		assign(szFmt);
	}



	
	~EPXString()
	{
              free();
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

	

        void Reset()
        {
              m_len = 0;
              m_psz = NULL;
        }


        void free()
        {
            if (m_psz != NULL)
            {			
                    TFREE(m_psz);
                    m_psz = NULL;
            }
        }

	
	EPXString& append(const char *psz)
	{	
		char *pszNew;
		int __len = strlen(psz) + this->m_len;
	
		pszNew = (char *) TMALLOC(__len + 1);
		*pszNew = 0x00;
		
		if (m_psz != NULL)
			strcpy(pszNew, m_psz);
		strcat(pszNew, psz);
		
		if (m_psz != NULL)
			TFREE(m_psz);
		m_psz = pszNew;	
		m_len = __len;		
		return *this;
	}
	
	
	
private:

	EPXString& assign(const EPXString& __str)
	{
                free();

		m_len = __str.m_len;
		m_psz = (char *) TMALLOC(m_len + 1);
		if (m_psz != NULL)
			strcpy(m_psz, __str.m_psz);
		return *this;
	}

	
	
	EPXString& assign(const char *psz)
	{
                free();

		m_len = strlen(psz);
		m_psz = (char *) TMALLOC(m_len + 1);
		if (m_psz != NULL)
			strcpy(m_psz, psz);
		return *this;
	}

	
	
	char *m_psz;
	int m_len;
};

