#include <stdint.h>
#include <stdio.h>
#include <vector>

typedef unsigned char byte;

/*

Data type	 Name	 Description
UINT32LE	 flags	 0=normal, 1=encrypted
UINT32LE	 offset	 Offset of the file data, relative to the start of the archive
UINT32LE	 length	 Size of the file data, in bytes
char	 filename[16]	 Null-terminated filename (15 chars + terminating null)

*/
struct FileEntry
{
	uint32_t flags;
	uint32_t offset;
	uint32_t length;
	char     filename[16];
};

void decode_entry(byte* buffer, FileEntry* entry);
void dump_files(std::vector<FileEntry>& file_entries, byte* glb_file);

int main(int argc, char* argv[])
{
/*
The very first entry is not a real file, but is a header. The offset field of this entry contains the number of files in the FAT (not including this header entry itself.) 
The rest of the fields are unused and should be set to zero.
The first file's data begins directly after the FAT, which will also be the offset value of the second FAT entry.
Encryption algorithm

The encryption key is the string "32768GLB".
For each byte in the file:
Subtract the character value from the current position in the encryption key (i.e. if the current position is 0, subtract 0x33, the character code for "3")
Advance the position in the encryption key by one (i.e. go to the next letter)
If the end of the encryption key has been reached, go back to the first character
Subtract the value of the previous byte read (note the previous byte *read*, not the decrypted version of that byte)
Logical AND with 0xFF to limit the result to 0-255
This byte is now decoded, move on to the next
Before performing the above loop, the initial state is as such:
The position in the encryption key (step 1) does not start at 0. Instead, it starts at 25 % <length of key>. 
In the case of the "32768GLB" key, the length is 8, so 25 mod 8 == 1, so the encryption key starts at position 1 (i.e. the first character of the key is skipped on the first run.) 
After the end of the encryption key is reached, it loops back to position 0 again.
The very first "previous byte read" value (step 4) is the actual key character at the initial position.
As per the previous item in this list, if the initial key position is 1, this is key character "2", so the first "previous byte read" is the character "2" (hex 0x32).
*/

	FILE* fp = fopen("d:/glbdump/file0004.glb", "rb");
	if (fp)
	{
		//byte header[4];
		//fread(header, sizeof(byte), 4, fp);

		byte buffer[28];
		
		fread(buffer, sizeof(byte), 28, fp);
		FileEntry file_header;
		decode_entry(buffer, &file_header);

		std::vector<FileEntry> file_entries;

		for (uint32_t i = 0; i < file_header.offset; ++i)
		{
			fread(buffer, sizeof(byte), 28, fp);
			FileEntry entry;
			decode_entry(buffer, &entry);
			file_entries.push_back(entry);
		}

		fseek(fp, 0L, SEEK_END);
		int size = ftell(fp);
		rewind(fp);

		byte* glb_file = new byte[size];
		fread(glb_file, sizeof(byte), size, fp);
		fclose(fp);

		dump_files(file_entries, glb_file);
		delete glb_file;
	}

	return 0;
}

void dump_files(std::vector<FileEntry>& file_entries, byte* glb_file)
{
	for (uint32_t i = 0; i < file_entries.size(); ++i)
	{
		std::string path = "d:/glbdump/";
		path += file_entries[i].filename;

		FILE* fp = fopen(path.c_str(), "wb");
		if (fp)
		{
			fwrite(glb_file + file_entries[i].offset, sizeof(byte), file_entries[i].length, fp);
			fclose(fp);
		}
	}
}

void decode_entry(byte* buffer, FileEntry* entry)
{

		const char* decrypt_key = "32768GLB";
		unsigned int key_position = 1; // Start at 1

		byte* decoded_buffer = (byte*) entry;
		//byte decoded_buffer[28];
		byte previous_byte_read = decrypt_key[key_position];
		for (unsigned int i = 0; i < 28; ++i)
		{
//The encryption key is the string "32768GLB".
//For each byte in the file:
//Subtract the character value from the current position in the encryption key (i.e. if the current position is 0, subtract 0x33, the character code for "3")
//Advance the position in the encryption key by one (i.e. go to the next letter)
//If the end of the encryption key has been reached, go back to the first character
//Subtract the value of the previous byte read (note the previous byte *read*, not the decrypted version of that byte)
//Logical AND with 0xFF to limit the result to 0-255
//This byte is now decoded, move on to the next
			byte current_byte = buffer[i];
			current_byte -= decrypt_key[key_position];
			current_byte -= previous_byte_read;
			current_byte &= 0xFF;
			decoded_buffer[i] = current_byte;

			++key_position;
			key_position %= 8;
			previous_byte_read = buffer[i];
		}
}
