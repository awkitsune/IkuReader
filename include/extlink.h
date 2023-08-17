#include <string>

#define ExtLinkBody_MaxLength (256)
#define ExtLinkBody_ID (0x30545845) // EXT0

/* //extlink.dat structure (LIES!)
typedef u16 UnicodeChar;
typedef struct {
  u32 ID,dummy1,dummy2,dummy3; // dummy is ZERO.
  char DataFullPathFilenameAlias[ExtLinkBody_MaxLength];	//0
  char DataPathAlias[ExtLinkBody_MaxLength];				//1
  char DataFilenameAlias[ExtLinkBody_MaxLength];			//2
  char NDSFullPathFilenameAlias[ExtLinkBody_MaxLength];		//3
  char NDSPathAlias[ExtLinkBody_MaxLength];					//4
  char NDSFilenameAlias[ExtLinkBody_MaxLength];				//5
  UnicodeChar DataFullPathFilenameUnicode[ExtLinkBody_MaxLength];	//6
  UnicodeChar DataPathUnicode[ExtLinkBody_MaxLength];				//7
  UnicodeChar DataFilenameUnicode[ExtLinkBody_MaxLength];			//8
  UnicodeChar NDSFullPathFilenameUnicode[ExtLinkBody_MaxLength];	//9
  UnicodeChar NDSPathUnicode[ExtLinkBody_MaxLength];
  UnicodeChar NDSFilenameUnicode[ExtLinkBody_MaxLength];
} TExtLinkBody;
*/

std::string GetFileBody_From_MoonShell2_ExtLink()
{
	char FullAlias[ExtLinkBody_MaxLength];
	const char *pfn="/moonshl2/extlink.dat"; // This full path file name is fixation. It is never moved.
	FILE *pf = fopen(pfn,"rb");
	if(pf == NULL) return "";
	u32 ID,IDSize;
	IDSize = fread(&ID,1,4,pf);
	if(IDSize == 0 || ID == 0) {
	  fclose(pf); pf=NULL;
	  return "";
	}
	if(ExtLinkBody_ID!=ID){
	  fclose(pf); pf=NULL;
	  return "";
	}
	u32 Dummy[3];
	fread(Dummy,1,4*3,pf);

	long int tell = ftell(pf);
	fseek(pf, 0, SEEK_END);
	int size = ftell(pf) - tell;
	fseek(pf, tell,SEEK_SET);
	
	fread(FullAlias,1,ExtLinkBody_MaxLength,pf);
	string out = FullAlias;
	
	/*
	for(int i = 0; i < 9; i++)
		fread(FullAlias,1,ExtLinkBody_MaxLength,pf);
	string out;
	for(int i = 0; i < ExtLinkBody_MaxLength; i++)
		if(FullAlias[i]) out += FullAlias[i];
	fread(FullAlias,1,ExtLinkBody_MaxLength,pf);
	fread(FullAlias,1,ExtLinkBody_MaxLength,pf);
	out += '/';
	for(int i = 0; i < ExtLinkBody_MaxLength; i++)
		if(FullAlias[i]) out += FullAlias[i];
	*/
	
	fclose(pf); pf=NULL;
	
	pf=fopen(pfn,"wb");
	fwrite(&ID,1,4,pf);
	fwrite(Dummy,1,4*3,pf);
	unsigned char a = 0;
	for(int i = 0; i < size; i++) fwrite(&a,1,1,pf);
	fclose(pf); pf=NULL;
	
	return out;
}