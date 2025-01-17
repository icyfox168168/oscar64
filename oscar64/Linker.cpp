#include "Linker.h"
#include <string.h>
#include <stdio.h>


LinkerRegion::LinkerRegion(void)
	: mSections(nullptr)
{}

LinkerSection::LinkerSection(void)
	: mObjects(nullptr)
{}

LinkerObject::LinkerObject(void)
	: mReferences(nullptr), mNumTemporaries(0)
{}

LinkerObject::~LinkerObject(void)
{

}

void LinkerObject::AddReference(const LinkerReference& ref)
{
	LinkerReference* nref = new LinkerReference(ref);
	mReferences.Push(nref);
}


void LinkerObject::AddData(const uint8* data, int size)
{
	mSize = size;
	mData = new uint8[size];
	memcpy(mData, data, size);
}

uint8* LinkerObject::AddSpace(int size)
{
	mSize = size;
	mData = new uint8[size];
	memset(mData, 0, size);
	return mData;
}

Linker::Linker(Errors* errors)
	: mErrors(errors), mSections(nullptr), mReferences(nullptr), mObjects(nullptr), mRegions(nullptr)
{
	for (int i = 0; i < 64; i++)
	{
		mCartridgeBankUsed[i] = 0;
		memset(mCartridge[i], 0, 0x4000);
	}
	memset(mMemory, 0, 0x10000);
}

Linker::~Linker(void)
{

}


LinkerRegion* Linker::AddRegion(const Ident* region, int start, int end)
{
	LinkerRegion* lrgn = new LinkerRegion();
	lrgn->mIdent = region;
	lrgn->mStart = start;
	lrgn->mEnd = end;
	lrgn->mUsed = 0;
	lrgn->mNonzero = 0;
	lrgn->mCartridge = -1;
	lrgn->mFlags = 0;
	mRegions.Push(lrgn);
	return lrgn;
}

LinkerRegion* Linker::FindRegion(const Ident* region)
{
	for (int i = 0; i < mRegions.Size(); i++)
	{
		if (mRegions[i]->mIdent == region)
			return mRegions[i];
	}

	return nullptr;
}

LinkerSection* Linker::AddSection(const Ident* section, LinkerSectionType type)
{
	LinkerSection* lsec = new LinkerSection;
	lsec->mIdent = section;
	lsec->mType = type;
	mSections.Push(lsec);
	return lsec;

}

LinkerSection* Linker::FindSection(const Ident* section)
{
	for (int i = 0; i < mSections.Size(); i++)
	{
		if (mSections[i]->mIdent == section)
			return mSections[i];
	}

	return nullptr;
}

bool Linker::IsSectionPlaced(LinkerSection* section)
{
	for (int i = 0; i < mRegions.Size(); i++)
	{
		LinkerRegion* rgn = mRegions[i];
		for (int j = 0; j < rgn->mSections.Size(); j++)
			if (section == rgn->mSections[j])
				return true;
	}

	return false;
}

LinkerObject* Linker::FindObjectByAddr(int addr)
{
	for (int i = 0; i < mObjects.Size(); i++)
	{
		LinkerObject* lobj = mObjects[i];
		if (lobj->mFlags & LOBJF_PLACED)
		{
			if (addr >= lobj->mAddress && addr < lobj->mAddress + lobj->mSize)
				return lobj;
		}
	}

	return nullptr;
}

LinkerObject * Linker::AddObject(const Location& location, const Ident* ident, LinkerSection * section, LinkerObjectType type)
{
	LinkerObject* obj = new LinkerObject;
	obj->mLocation = location;
	obj->mID = mObjects.Size();
	obj->mType = type;
	obj->mData = nullptr;
	obj->mSize = 0;
	obj->mIdent = ident;
	obj->mSection = section;
	obj->mRegion = nullptr;
	obj->mProc = nullptr;
	obj->mFlags = 0;
	section->mObjects.Push(obj);
	mObjects.Push(obj);
	return obj;
}

void Linker::CollectReferences(void) 
{
	for (int i = 0; i < mObjects.Size(); i++)
	{
		LinkerObject* lobj(mObjects[i]);
		for (int j = 0; j < lobj->mReferences.Size(); j++)
			mReferences.Push(lobj->mReferences[j]);
	}
}

void Linker::ReferenceObject(LinkerObject* obj)
{
	if (!(obj->mFlags & LOBJF_REFERENCED))
	{
		obj->mFlags |= LOBJF_REFERENCED;
		for (int i = 0; i < mReferences.Size(); i++)
		{
			LinkerReference* ref = mReferences[i];
			if (ref->mObject == obj)
				ReferenceObject(ref->mRefObject);
		}
	}
}

void Linker::Link(void)
{
	if (mErrors->mErrorCount == 0)
	{

		for (int i = 0; i < mSections.Size(); i++)
		{
			LinkerSection* lsec = mSections[i];
			lsec->mStart = 0x10000;
			lsec->mEnd = 0x0000;
		}

		// Move objects into regions

		for (int i = 0; i < mRegions.Size(); i++)
		{
			LinkerRegion* lrgn = mRegions[i];
			for (int j = 0; j < lrgn->mSections.Size(); j++)
			{
				LinkerSection* lsec = lrgn->mSections[j];
				for (int k = 0; k < lsec->mObjects.Size(); k++)
				{
					LinkerObject* lobj = lsec->mObjects[k];
					if ((lobj->mFlags & LOBJF_REFERENCED) && !(lobj->mFlags & LOBJF_PLACED) && lrgn->mStart + lrgn->mUsed + lobj->mSize <= lrgn->mEnd)
					{
						lobj->mFlags |= LOBJF_PLACED;
						lobj->mAddress = lrgn->mStart + lrgn->mUsed;
						lrgn->mUsed += lobj->mSize;
						lobj->mRegion = lrgn;

						if (lsec->mType == LST_DATA)
							lrgn->mNonzero = lrgn->mUsed;

						if (lobj->mAddress < lsec->mStart)
							lsec->mStart = lobj->mAddress;
						if (lobj->mAddress + lobj->mSize > lsec->mEnd)
							lsec->mEnd = lobj->mAddress + lobj->mSize;
					}
				}
			}
		}

		mProgramStart = 0x0801;
		mProgramEnd = 0x0801;

		int	address = 0;

		for (int i = 0; i < mRegions.Size(); i++)
		{
			LinkerRegion* lrgn = mRegions[i];
			address = lrgn->mStart + lrgn->mNonzero;

			if (lrgn->mNonzero && address > mProgramEnd)
				mProgramEnd = address;
		}

		// Place stack segment
		
		for (int i = 0; i < mRegions.Size(); i++)
		{
			LinkerRegion* lrgn = mRegions[i];
			for (int j = 0; j < lrgn->mSections.Size(); j++)
			{
				LinkerSection* lsec = lrgn->mSections[j];

				if (lsec->mType == LST_STACK)
				{
					lsec->mStart = lrgn->mEnd - lsec->mSize;
					lsec->mEnd = lrgn->mEnd;
					lrgn->mEnd = lsec->mStart;
				}
			}
		}

		// Now expand the heap section to cover the remainder of the region

		for (int i = 0; i < mRegions.Size(); i++)
		{
			LinkerRegion* lrgn = mRegions[i];
			for (int j = 0; j < lrgn->mSections.Size(); j++)
			{
				LinkerSection* lsec = lrgn->mSections[j];

				if (lsec->mType == LST_HEAP)
				{
					lsec->mStart = lrgn->mStart + lrgn->mUsed;
					lsec->mEnd = lrgn->mEnd;
				}
			}
		}

		for (int i = 0; i < mObjects.Size(); i++)
		{
			LinkerObject* obj = mObjects[i];
			if (obj->mType == LOT_SECTION_START)
				obj->mAddress = obj->mSection->mStart;
			else if (obj->mType == LOT_SECTION_END)
				obj->mAddress = obj->mSection->mEnd;
			else if (obj->mFlags & LOBJF_REFERENCED)
			{
				if (obj->mRegion->mCartridge >= 0)
				{
					mCartridgeBankUsed[obj->mRegion->mCartridge] = true;
					memcpy(mCartridge[obj->mRegion->mCartridge] + obj->mAddress - obj->mRegion->mStart, obj->mData, obj->mSize);
				}
				else
				{
					memcpy(mMemory + obj->mAddress, obj->mData, obj->mSize);
				}
			}
		}

		for (int i = 0; i < mReferences.Size(); i++)
		{
			LinkerReference* ref = mReferences[i];
			LinkerObject* obj = ref->mObject;
			if (obj->mFlags & LOBJF_REFERENCED)
			{
				LinkerObject* robj = ref->mRefObject;

				int			raddr = robj->mAddress + ref->mRefOffset;
				uint8* dp;
				
				if (obj->mRegion->mCartridge < 0)
					dp = mMemory + obj->mAddress + ref->mOffset;
				else
					dp = mCartridge[obj->mRegion->mCartridge] + obj->mAddress - obj->mRegion->mStart + ref->mOffset;

				if (ref->mFlags & LREF_LOWBYTE)
					*dp++ = raddr & 0xff;
				if (ref->mFlags & LREF_HIGHBYTE)
					*dp++ = (raddr >> 8) & 0xff;
				if (ref->mFlags & LREF_TEMPORARY)
					*dp += obj->mTemporaries[ref->mRefOffset];
			}
		}
	}
}

static const char * LinkerObjectTypeNames[] = 
{
	"NONE",
	"PAD",
	"BASIC",
	"BYTE_CODE",
	"NATIVE_CODE",
	"RUNTIME",
	"DATA",
	"BSS",
	"HEAP",
	"STACK",
	"START",
	"END"
};

static const char* LinkerSectionTypeNames[] = {
	"NONE",
	"DATA",
	"BSS",
	"HEAP",
	"STACK"
};

bool Linker::WritePrgFile(const char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "wb");
	if (file)
	{
		mMemory[mProgramStart - 2] = mProgramStart & 0xff;
		mMemory[mProgramStart - 1] = mProgramStart >> 8;

		int	done = fwrite(mMemory + mProgramStart - 2, 1, mProgramEnd - mProgramStart + 2, file);
		fclose(file);
		return done == mProgramEnd - mProgramStart + 2;
	}
	else
		return false;
}

bool Linker::WriteCrtFile(const char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "wb");
	if (file)
	{
		struct CRIHeader
		{
			char	mSignature[16];
			uint32	mHeaderLength;
			uint16	mVersion, mHardware;
			uint8	mExrom, mGameLine;
			uint8	mPad[6];
			char	mName[32];
		}	criHeader = { 0 };

		memcpy(criHeader.mSignature, "C64 CARTRIDGE   ", 16);
		criHeader.mHeaderLength = 0x40000000;
		criHeader.mVersion = 0x0001;
		criHeader.mHardware = 0x2000;
		criHeader.mExrom = 0;
		criHeader.mGameLine = 0;
		memset(criHeader.mName, 0, 32);
		strcpy_s(criHeader.mName, "OSCAR");

		fwrite(&criHeader, sizeof(CRIHeader), 1, file);

		struct CHIPHeader
		{
			char	mSignature[4];
			uint32	mPacketLength;
			uint16	mChipType, mBankNumber, mLoadAddress, mImageSize;
		}	chipHeader = { 0 };

		memcpy(chipHeader.mSignature, "CHIP", 4);
		chipHeader.mPacketLength = 0x10200000;
		chipHeader.mChipType = 0;
		chipHeader.mBankNumber = 0;
		chipHeader.mImageSize = 0x0020;

		uint8 bootmem[8192];

		memset(bootmem, 0, 0x2000);

		chipHeader.mLoadAddress = 0x0080;
		fwrite(&chipHeader, sizeof(chipHeader), 1, file);
		fwrite(mMemory + 0x0800, 1, 0x2000, file);

		memcpy(bootmem, mMemory + 0x2800, 0x1800);

		bootmem[0x1ffc] = 0x00;
		bootmem[0x1ffd] = 0xff;

		uint8	bootcode[] = {
			0xa9, 0x87,
			0x8d, 0x02, 0xde,
			0xa9, 0x00,
			0x8d, 0x00, 0xde,
			0x6c, 0xfc, 0xff
		};

		int j = 0x1f00;
		for (int i = 0; i < sizeof(bootcode); i++)
		{
			bootmem[j++] = 0xa9;
			bootmem[j++] = bootcode[i];
			bootmem[j++] = 0x8d;
			bootmem[j++] = i;
			bootmem[j++] = 0x04;
		}
		bootmem[j++] = 0x4c;
		bootmem[j++] = 0x00;
		bootmem[j++] = 0x04;

		chipHeader.mLoadAddress = 0x00e0;
		fwrite(&chipHeader, sizeof(chipHeader), 1, file);
		fwrite(bootmem, 1, 0x2000, file);

		for (int i = 1; i < 64; i++)
		{
			if (mCartridgeBankUsed[i])
			{
				chipHeader.mBankNumber = i << 8;

				chipHeader.mLoadAddress = 0x0080;
				fwrite(&chipHeader, sizeof(chipHeader), 1, file);
				fwrite(mCartridge[i] + 0x0000, 1, 0x2000, file);

				chipHeader.mLoadAddress = 0x00a0;
				fwrite(&chipHeader, sizeof(chipHeader), 1, file);
				fwrite(mCartridge[i] + 0x2000, 1, 0x2000, file);
			}
		}

		fclose(file);
		return true;
	}
	else
		return false;
}


bool Linker::WriteMapFile(const char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "wb");
	if (file)
	{
		fprintf(file, "sections\n");
		for (int i = 0; i <  mSections.Size(); i++)
		{
			LinkerSection* lsec = mSections[i];

			fprintf(file, "%04x - %04x : %s, %s\n", lsec->mStart, lsec->mEnd, LinkerSectionTypeNames[lsec->mType], lsec->mIdent->mString);
		}

		fprintf(file, "\nregions\n");

		for (int i = 0; i < mRegions.Size(); i++)
		{
			LinkerRegion	* lrgn = mRegions[i];

			fprintf(file, "%04x - %04x : %04x, %04x, %s\n", lrgn->mStart, lrgn->mEnd, lrgn->mNonzero, lrgn->mUsed, lrgn->mIdent->mString);
		}

		fprintf(file, "\nobjects\n");

		for (int i = 0; i < mObjects.Size(); i++)
		{
			LinkerObject* obj = mObjects[i];

			if (obj->mFlags & LOBJF_REFERENCED)
			{
				if (obj->mIdent)
					fprintf(file, "%04x - %04x : %s, %s:%s\n", obj->mAddress, obj->mAddress + obj->mSize, obj->mIdent->mString, LinkerObjectTypeNames[obj->mType], obj->mSection->mIdent->mString);
				else
					fprintf(file, "%04x - %04x : *, %s:%s\n", obj->mAddress, obj->mAddress + obj->mSize, LinkerObjectTypeNames[obj->mType], obj->mSection->mIdent->mString);
			}
		}

		fclose(file);

		return true;
	}
	else
		return false;
}

bool Linker::WriteLblFile(const char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "wb");
	if (file)
	{
		for (int i = 0; i < mObjects.Size(); i++)
		{
			LinkerObject* obj = mObjects[i];

			if (obj->mFlags & LOBJF_REFERENCED)
			{
				if (obj->mIdent)
					fprintf(file, "al %04x .%s\n", obj->mAddress, obj->mIdent->mString);
			}
		}

		fclose(file);

		return true;
	}
	else
		return false;
}

bool Linker::WriteAsmFile(const char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "wb");
	if (file)
	{
		for (int i = 0; i < mObjects.Size(); i++)
		{
			LinkerObject* obj = mObjects[i];

			if (obj->mFlags & LOBJF_REFERENCED)
			{
				switch (obj->mType)
				{
				case LOT_BYTE_CODE:
					mByteCodeDisassembler.Disassemble(file, mMemory, obj->mAddress, obj->mSize, obj->mProc, obj->mIdent, this);
					break;
				case LOT_NATIVE_CODE:
					if (obj->mRegion->mCartridge < 0)
						mNativeDisassembler.Disassemble(file, mMemory, obj->mAddress, obj->mSize, obj->mProc, obj->mIdent, this);
					else
						mNativeDisassembler.Disassemble(file, mCartridge[obj->mRegion->mCartridge] - obj->mRegion->mStart, obj->mAddress, obj->mSize, obj->mProc, obj->mIdent, this);
					break;
				}
			}
		}

		fclose(file);

		return true;
	}
	else
		return false;
}

