
/*
	File		: FileReader.h
	Author		: Mustafa K. GILOR
	Description : Provides cross-platform file reading functionality.
	Date		: 23.12.2015

	TO-DO List	:
	-	Add {CRC32,SHA1,SHA256,MD5} checksum algorithms for checksum generation and comparison	
*/

#pragma once

#pragma region	(Include)

	/* Interplatform */
	#include <stdio.h>
	#include <time.h>
	#include <string>


	/* Platform specific */
	#ifdef linux
		#include <stdlib.h>
		#include <sys/stat.h>
		#include <string.h>
	#elif _WIN32

	#else
		#error Platform is not supported by file library.
	#endif

#pragma endregion

#pragma region	(Define)
	#define FCOPY_BUFFER_SIZE 4096 // Buffer size for copy operation
	#define MAX_PATH 4096
	#define MAX_DRIVE_LETTER 3		
#pragma endregion


/// wow. such macro. much functionality.
#ifdef _WIN32
	#define open_file(fpointer,fn,mode) fopen_s(&fpointer,fn,mode)
	#define read_file(buf,elem_size,elem_count,fpointer) fread_s(buf,elem_size * elem_count,elem_size,elem_count,fpointer)
	#define write_file(buf,elem_size,elem_count,fpointer) fwrite(buf,elem_size,elem_count,fpointer)
#else
	#define open_file(fpointer,fn,mode) fpointer = fopen(fn,mode)
	#define read_file(buf,elem_size,elem_count,fpointer) fread(buf,elem_size,elem_count,fpointer)
	#define write_file(buf,elem_size,elem_count,fpointer) fwrite(buf,elem_size,elem_count,fpointer)
#endif

namespace mkemal
{
	/* File library */
	namespace filetools
	{
		
		class FileReader final
		{
		public:

			/*
				@func		FileReader
				@purpose	File reader class constructor
				@params		szFilename		A (constant) char pointer to the file name
				@returns	-
				@created	27th of December, 2015
			*/
			explicit FileReader(char * szFileName): m_fp(NULL),m_fd(-1),m_cszFileName(szFileName) {
				memset(&m_fileStat, 0, sizeof(struct stat));
				memset(&m_szFullPath, 0, MAX_PATH);
				memset(&m_szDriveLetter, 0, MAX_DRIVE_LETTER);
			}

			/*
				@func		FileReader
				@purpose	File reader class constructor
				@params		szFilename		A standart string containing file path and the name
				@returns	-
				@created	27th of December, 2015
			*/
			explicit FileReader(const std::string & strFileName) : m_fp(NULL), m_cszFileName((char*)strFileName.c_str()) {
				memset(&m_fileStat, 0, sizeof(struct stat));
				memset(&m_szFullPath, 0, MAX_PATH);
				memset(&m_szDriveLetter, 0, MAX_DRIVE_LETTER);
			}

			/*
				@func		~FileReader
				@purpose	File reader class destructor
				@params		none
				@returns	-
				@created	27th of December, 2015
			*/
			~FileReader()
			{
				if (m_fp)
					Close();
			}

			bool Open()
			{
			
				if (m_fp)
				{
					/* 
						If a file is already open, we should close it first.
					*/
					std::string cszFileName = std::string(m_cszFileName);
					Close();
					m_cszFileName = (char*)(cszFileName.c_str());
				}

				open_file(m_fp, m_cszFileName, "rb");

				/* File open failed */
				if (NULL == m_fp)
					return false;

				m_fd = fileno(m_fp);
				char dir [MAX_PATH];
				char fname [MAX_PATH];
				char extension [MAX_PATH];
				#ifdef linux
					realpath(m_cszFileName, m_szFullPath);
					if (strlen(m_szFullPath) > 0)
					{
						strcpy(dir, m_szFullPath);
						strcpy(fname, m_szFullPath);
						strcpy(dir, dirname(dir));
						strcpy(fname, basename(fname));
						strcpy(extension, fname);
						strcpy(fname, strtok(fname, "."));
						strtok(extension, ".");
						strcpy(extension, strtok(NULL, "."));
						
						std::string ext_tmp = std::string(extension);
						sprintf(extension, ".%s", ext_tmp.c_str());

						m_strDirectory = std::string(dir);
						m_strFilename = std::string(fname);
						m_strExtension = std::string(extension);
					}
					
				#elif _WIN32
				
					_fullpath(m_szFullPath, m_cszFileName, MAX_PATH);
					_splitpath(m_szFullPath,m_szDriveLetter,dir,fname,extension);
					
					m_strDirectory	= std::string(dir);
					m_strFilename	= std::string(fname);
					m_strExtension	= std::string(extension);
					//m_s
				#endif

				if (stat(m_szFullPath, &m_fileStat) == -1)
				{
					printf("stat failed\n");
					memset(&m_fileStat, 0, sizeof(struct stat));
				}
				return true;
			}

			bool Move(const char * szDestination){

				if (NULL == m_fp)
					return false;
				long _curpos = ftell(m_fp);
				/// Close the file for moving
				fclose(m_fp);
				/// "If oldname and newname specify different paths and this is supported by the system, 
				///  the file is moved to the new location."
				if (rename(m_cszFileName, szDestination) > -1)
				{
					m_cszFileName = (char*)szDestination;
					bool open = Open();
					// Reseek after
					fseek(m_fp, _curpos, SEEK_SET);
					return open;

				}
				else
				{
					Open();
					// Reseek after
					fseek(m_fp, _curpos, SEEK_SET);
					return false;
				}
				
			}

			bool Copy(const char * szDestFilename){
				if (NULL == m_fp)
					return false;
				FILE * fCopy = fopen(szDestFilename, "wb");
				if (fCopy)
				{
					
					long _curpos = ftell(m_fp);
					// Reset to the beginning
					fseek(m_fp, 0, SEEK_SET);
					/// Buffered copy operation
					unsigned char buf [FCOPY_BUFFER_SIZE];
					int rsize = 0;
					while ((rsize = ReadBytes(buf, FCOPY_BUFFER_SIZE)) > 0)
						write_file(buf, 1, rsize, fCopy);
					fclose(fCopy);

					// Reseek after
					fseek(m_fp, _curpos, SEEK_SET);
					return true;
				}
				return false;
			}


			void Reset(){
				fseek(m_fp, 0, SEEK_SET);
			}

			void Close()
			{
				if (m_fp == NULL)
					return;
				fclose(m_fp);
				m_fp = NULL; m_fd = -1;
				memset(&m_fileStat, 0, sizeof(struct stat));
				memset(&m_szFullPath, 0, MAX_PATH);
				memset(&m_szDriveLetter, 0, 3);
			}

			/*
				@func		GetDriveLetter
				@purpose	Retrieving the logical drive letter of file path.
				@params		none
				@returns	std::string containing the drive letter.
				@created	23nd of January, 2016
			*/
			inline const std::string	GetDriveLetter()				{ return std::string(m_szDriveLetter);}
			/*
				@func		GetExtension
				@purpose	Retrieving the file extension of the file.
				@params		none
				@returns	std::string containing the drive letter.
				@created	23nd of January, 2016
			*/
			inline const std::string	GetExtension()					{ return std::string(m_strExtension); }
			/*
				@func		GetDirectory
				@purpose	Retrieving the folder name of the file.
				@params		none
				@returns	std::string containing the directory name of the file.
				@created	23nd of January, 2016
			*/
			inline const std::string	GetDirectory()					{ return m_strDirectory; }
			/*
				@func		GetFileName
				@purpose	Retrieving the name of the file.
				@params		none
				@returns	std::string containing filename, without extension.
				@created	23nd of January, 2016
			*/
			inline const std::string	GetFileName()					{ return m_strFilename; }
			/*
				@func		GetFileName
				@purpose	Retrieving the absolute path of the file.
				@params		none
				@returns	std::string containing absolute path containing everything.
				@created	23nd of January, 2016
			*/
			inline const std::string	GetPath()						{ return std::string(m_szFullPath); }
			inline const std::string	GetLastModifyTimeString()		{ return UnixTimeToReadableString(m_fileStat.st_mtime); }
			inline const std::string	GetCreationTimeString()			{ return UnixTimeToReadableString(m_fileStat.st_ctime); }
			inline const std::string	GetLastAccessTimeString()		{ return UnixTimeToReadableString(m_fileStat.st_atime); }

			/*
				@func		GetLastModifyTime
				@purpose	Retrieving the last modification time of the file.
				@params		none
				@returns	time_t, representing amount of seconds passed since EPOCH, from last modification date of the file.
				@created	22nd of January, 2016
			*/
			inline time_t GetLastModifyTime()				const		{ return m_fileStat.st_mtime;}
			/*
				@func		GetCreationTime
				@purpose	Retrieving the creation time of the file.
				@params		none
				@returns	time_t, representing amount of seconds passed since EPOCH, from creation date of the file.
				@created	22nd of January, 2016
			*/
			inline time_t GetCreationTime()					const		{ return m_fileStat.st_ctime;}
			/*
				@func		GetLastAccessTime
				@purpose	Retrieving the last access time of the file.
				@params		none
				@returns	time_t, representing amount of seconds passed since EPOCH, from last access date of the file.
				@created	22nd of January, 2016
			*/
			inline time_t GetLastAccessTime()				const		{ return m_fileStat.st_atime;}
			inline size_t GetSize()							const		{ return m_fileStat.st_size;}
			inline short  GetOwnerUserID()					const		{ return m_fileStat.st_uid; }
			inline short  GetOwnerGroupID()					const		{ return m_fileStat.st_gid; }

			inline int GetFileDescriptor()					const		{ return m_fd; }
			inline FILE * GetFilePointer()					const		{ return m_fp; }

			void PrintFileInformation()
			{
				const float byte = static_cast<float>(GetSize()), kbyte = byte / 1024.0f, mbyte = kbyte / 1024.0f, gbyte = mbyte / 1024.0f;
				printf("Absolute path of file\n\t -- %s\n", GetPath().c_str());
				printf("Drive letter and Directory\n\t -- %s,%s\n",GetDriveLetter().c_str(), GetDirectory().c_str());
				printf("Filename and Extension\n\t -- %s,%s\n", GetFileName().c_str(), GetExtension().c_str());
				printf("File size\n\t -- byte(s) : %llu byte(s)\n\t -- kilobyte(s) : %g KB\n\t -- megabyte(s) : %g MB\n\t -- gigabyte(s) : %g GB\n", static_cast<unsigned long long>(byte), kbyte, mbyte, gbyte);
				printf("Created on\n\t -- unix_timestamp(%llu)\t%s\n", GetCreationTime(), GetCreationTimeString().c_str());
				printf("Modified on\n\t -- unix_timestamp(%llu)\t%s\n", GetLastModifyTime(), GetLastModifyTimeString().c_str());
				printf("Accessed on\n\t -- unix_timestamp(%llu)\t%s\n", GetLastAccessTime(), GetLastAccessTimeString().c_str());
				printf("File is owned by UID(%u), GROUP(%u)\n", GetOwnerUserID(), GetOwnerGroupID());
			}

		/*	std::string CalculateSHA1()
			{
				if (m_fp == NULL)
					throw std::exception("File is not open.");
				long _curpos = ftell(m_fp);

				// Reset to the beginning
				fseek(m_fp, 0, SEEK_SET);
				

				SHA1 sha1;
				unsigned char buf;
				while (Read<unsigned char>(&buf))
					sha1.add(&buf, 1);

				// Reseek after
				fseek(m_fp, _curpos, SEEK_SET);

				return sha1.getHash();
			}*/

			

			


			template <typename T>
			const size_t Read(T* val){
				return read_file(val, sizeof(T), 1, m_fp);	
			}

			template <typename T>
			const T Read(){
				T val;
				
				if (read_file(&val, sizeof(T), 1, m_fp) == 0)
				{
					throw std::exception("End of file reached.");
				}
				return val;
			}

			const size_t ReadBytes(void * buf, size_t len){
				return read_file(buf, 1, len, m_fp);
			}

		private:
			std::string UnixTimeToReadableString(time_t unix_timestamp) const 
			{
				char arcString [20];
				// add start-date/start-time
				if (strftime(&(arcString [0]), 20, "%d.%m.%Y %H:%M:%S", (const tm*)(gmtime((const time_t*)&(unix_timestamp)))) != 0)
					return std::string((char*)&(arcString [0]));
				return std::string("1970-01-01 00:00:00");
			}
			std::string m_strExtension, m_strFilename, m_strDirectory;
			struct stat m_fileStat;
			char * m_cszFileName;
			char m_szDriveLetter	[MAX_DRIVE_LETTER];
			char m_szFullPath		[MAX_PATH];
			FILE *  m_fp; 
			int		m_fd; 

		};
	}
}
