#pragma once
#include <stdio.h>
#include <vector>
using namespace std;
namespace VIS
{
	class CLogFile
	{
	public:
		static CLogFile* GetLogFile();
		CLogFile(void);
		CLogFile(char *_filename);
		~CLogFile(void);
		static void Create(char* _filename);
		void Add(char* _column, float _data);
		void NextLine();
		void Write();
	private:
		static CLogFile* s_logfile;
		FILE* m_logfile;
		vector<string> m_column;
		vector<vector<float>> m_data;
	};
}
