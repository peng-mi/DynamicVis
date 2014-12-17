#include "LogFile.h"
#include <fstream>
namespace VIS
{
	CLogFile* CLogFile::s_logfile;
	CLogFile::CLogFile(void)
	{
		s_logfile = NULL;
	}

	CLogFile::CLogFile(char* _filename)
	{
		m_logfile = fopen(_filename,"w");
	}

	CLogFile::~CLogFile(void)
	{
	}

	void CLogFile::Create(char* _filename)
	{
		s_logfile = new CLogFile(_filename);
		s_logfile->NextLine();
	}

	CLogFile* CLogFile::GetLogFile()
	{
		if(s_logfile != NULL)
			return s_logfile;
		else
		{
			Create("logfile.csv");
			return s_logfile;
		}
	}

	void CLogFile::Add(char* _column, float _data)
	{
		unsigned int index = 0;
		for( index = 0; index < m_column.size(); index++)
		{
			if(strcmp(_column, m_column[index].c_str()) == 0)
				break;
		}
		if(index == m_column.size())
			m_column.push_back(_strdup(_column));
		
		if( m_data[m_data.size() -1].size() <  m_column.size())
		{
			m_data[m_data.size() -1].push_back(_data);
		}
		else
			m_data[m_data.size() -1][index] = _data;
	}

	void CLogFile::NextLine()
	{
		vector<float> tmp;
		tmp.resize(m_column.size());
		m_data.push_back(tmp);
	}

	void CLogFile::Write()
	{
		if(m_column.size() != 0)
		{
			for(unsigned int i =0; i < m_column.size() -1; i++)
			{
				fprintf(m_logfile, "%s,",m_column[i].c_str());
			}
		
			fprintf(m_logfile, "%s\n",m_column[m_column.size() -1].c_str());
	

			for(unsigned int i = 0; i < m_data.size(); i++)
			{
				if(m_data[i].size() < m_column.size())
					continue;
				for(unsigned int j = 0; j < m_data[i].size() -1; j++)
					fprintf(m_logfile, "%f,",m_data[i][j]);
				fprintf(m_logfile, "%f\n",m_data[i][m_column.size() -1]);
			}
		}
		fflush(m_logfile);
		fclose(m_logfile);
	}
}