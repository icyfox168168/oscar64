#include "Preprocessor.h"
#include <string.h>
#include <stdlib.h>

SourcePath::SourcePath(const char* path)
{
	strcpy_s(mPathName, path);
	char* p = mPathName;
	while (*p)
	{
		if (*p == '\\')
			*p = '/';
		p++;
	}
}

SourcePath::~SourcePath(void)
{

}

bool SourceFile::ReadLine(char* line)
{
	if (mFile)
	{
		if (mBinary)
		{
			if (mLimit)
				mLimit--;

			int c = fgetc(mFile);
			if (c >= 0)
			{
				sprintf_s(line, 1024, "0x%02x, ", c);
				return true;
			}
		}
		else
		{
			if (fgets(line, 1024, mFile))
				return true;
		}

		fclose(mFile);
		mFile = nullptr;
	}

	return false;
}

void SourceFile::Limit(int skip, int limit)
{
	mLimit = limit;
	if (mFile)
		fseek(mFile, skip, SEEK_SET);
}

SourceFile::SourceFile(void) 
	: mFile(nullptr), mFileName{ 0 }, mStack(nullptr)
{

}

SourceFile::~SourceFile(void)
{
	if (mFile)
	{
		fclose(mFile);
		mFile = nullptr;
	}
}

bool SourceFile::Open(const char* name, const char* path, bool binary)
{
	char	fname[220];

	if (strlen(name) + strlen(path) > 200)
		return false;

	strcpy_s(fname, path);
	int	n = strlen(fname);

	if (n > 0 && fname[n - 1] != '/')
	{
		fname[n++] = '/';
		fname[n] = 0;
	}

	strcat_s(fname + n, sizeof(fname) - n, name);

	if (!fopen_s(&mFile, fname, binary ? "rb" : "r"))
	{
		_fullpath(mFileName, fname, sizeof(mFileName));
		char* p = mFileName;
		while (*p)
		{
			if (*p == '\\')
				*p = '/';
			p++;
		}
		mBinary = binary;
		mLimit = 0x10000;

		return true;
	}

	return false;
}

void SourceFile::Close(void)
{
	if (mFile)
	{
		fclose(mFile);
		mFile = nullptr;
	}
}

bool SourceFile::PushSource(void)
{
	SourceStack* stack = new SourceStack();
	stack->mUp = mStack;
	mStack = stack;
	stack->mFilePos = ftell(mFile);
	return true;
}

bool SourceFile::PopSource(void)
{
	SourceStack* stack = mStack;
	if (stack)
	{
		fseek(mFile, stack->mFilePos, SEEK_SET);
		mStack = mStack->mUp;
		return true;
	}
	else
		return false;
}

bool SourceFile::DropSource(void)
{
	SourceStack* stack = mStack;
	if (stack)
	{
		mStack = mStack->mUp;
		return true;
	}
	else
		return false;
}

bool Preprocessor::NextLine(void)
{
	int	s = 0;
	while (mSource->ReadLine(mLine + s))
	{
		mLocation.mLine++;

		s = strlen(mLine);
		while (s > 0 && mLine[s - 1] == '\n')
			s--;
		if (s == 0 || mLine[s - 1] != '\\')
			return true;
		s--;
	}

	return false;
}

bool Preprocessor::EmbedData(const char* reason, const char* name, bool local, int skip, int limit)
{
	if (strlen(name) > 200)
	{
		mErrors->Error(mLocation, EERR_FILE_NOT_FOUND, "Binary file path exceeds max path length");
		return false;
	}

	if (mSource)
		mSource->mLocation = mLocation;

	SourceFile* source = new SourceFile();

	bool	ok = false;

	if (source->Open(name, "", true))
		ok = true;

	if (!ok && local && mSource)
	{
		char	lpath[220];
		strcpy_s(lpath, mSource->mFileName);
		int	i = strlen(lpath);
		while (i > 0 && lpath[i - 1] != '/')
			i--;
		lpath[i] = 0;

		if (source->Open(name, lpath, true))
			ok = true;
	}

	SourcePath* p = mPaths;
	while (!ok && p)
	{
		if (source->Open(name, p->mPathName, true))
			ok = true;
		else
			p = p->mNext;
	}

	if (ok)
	{
		printf("%s \"%s\"\n", reason, source->mFileName);

		source->Limit(skip, limit);

		source->mUp = mSource;
		mSource = source;
		mLocation.mFileName = mSource->mFileName;
		mLocation.mLine = 0;
		mLine[0] = 0;

		return true;
	}
	else
	{
		delete source;
		return false;
	}
}

bool Preprocessor::OpenSource(const char * reason, const char* name, bool local)
{
	if (strlen(name) > 200) 
	{
		mErrors->Error(mLocation, EERR_FILE_NOT_FOUND, "Source file path exceeds max path length");
		return false;
	}

	if (mSource)
		mSource->mLocation = mLocation;

	SourceFile	*	source = new SourceFile();

	bool	ok = false;

	if (source->Open(name, ""))
		ok = true;

	if (!ok && local && mSource)
	{
		char	lpath[220];
		strcpy_s(lpath, mSource->mFileName);
		int	i = strlen(lpath);
		while (i > 0 && lpath[i - 1] != '/')
			i--;
		lpath[i] = 0;

		if (source->Open(name, lpath))
			ok = true;
	}

	SourcePath* p = mPaths;
	while (!ok && p)
	{
		if (source->Open(name, p->mPathName))
			ok = true;
		else
			p = p->mNext;
	}
	
	if (ok)
	{
		printf("%s \"%s\"\n", reason, source->mFileName);
		source->mUp = mSource;
		mSource = source;
		mLocation.mFileName = mSource->mFileName;
		mLocation.mLine = 0;
		mLine[0] = 0;

		return true;
	}
	else
		return false;
}

bool Preprocessor::CloseSource(void)
{
	if (mSource)
	{
		mSource = mSource->mUp;
		if (mSource)
		{
			mLocation = mSource->mLocation;
			mLine[0] = 0;
			return true;
		}
	}
	
	return false;
}

bool Preprocessor::PushSource(void)
{
	mSource->mLocation = mLocation;
	return mSource->PushSource();
}

bool Preprocessor::PopSource(void)
{
	mLocation = mSource->mLocation;
	return mSource->PopSource();
}

bool Preprocessor::DropSource(void)
{
	return mSource->DropSource();
}

Preprocessor::Preprocessor(Errors* errors)
	: mSource(nullptr), mSourceList(nullptr), mPaths(nullptr), mErrors(errors)
{

}

Preprocessor::~Preprocessor(void)
{
}

void Preprocessor::AddPath(const char* path)
{
	SourcePath* sp = new SourcePath(path);
	sp->mNext = mPaths;
	mPaths = sp;
}

