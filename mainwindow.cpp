/*
MIT License

Copyright (c) 2020 BlackStar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "mainwindow.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QMimeData>
#include <QDebug>
#include <QFileInfo>
#include <QFile>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

MainWindow::MainWindow()
{
	initUI();
}

void MainWindow::readPalette(const QString& palette)
{
	QFile pal(palette);
	pal.open(QIODevice::ReadOnly);
	if (pal.isOpen())
	{
		QByteArray data = pal.readAll();
		uint8_t* cdata = (uint8_t*) data.constData();
		
		uint32_t col = 0;
		uint8_t brighten = 2;
		for (uint32_t i = 0; i < 768; i += 3)
		{
			uint8_t r = cdata[i] << brighten;
			uint8_t g = cdata[i+1] << brighten;
			uint8_t b = cdata[i+2] << brighten;

			raptorPalette[col++] = qRgb(r, g, b);
		}


		pal.close();
	}
	const uint32_t WIDTH = 256 * 4;
	const uint32_t HEIGHT = 20;
	paletteLabel->setGeometry(10, 150, WIDTH, HEIGHT);
	QImage image(256, 1, QImage::Format_RGBA8888);

	for (uint32_t y = 0; y < 1; ++y)
	{
		for (uint32_t x = 0; x < 256; ++x)
		{
			image.setPixel(x, y, raptorPalette[x]);
		}
	}

	paletteLabel->setPixmap(QPixmap::fromImage(image).scaled(QSize(WIDTH, HEIGHT), Qt::IgnoreAspectRatio, Qt::FastTransformation));
}

void MainWindow::initUI()
{
	filenameLabel = new QLabel("Filename", this);
	filenameLabel->setGeometry(10, 10, 100, 20);
	filenameValueLabel = new QLabel("---", this);
	filenameValueLabel->setGeometry(110, 10, 400, 20);
	QPushButton* openFile = new QPushButton("...", this);
	openFile->setGeometry(520, 10, 40, 20);
	connect(openFile, SIGNAL(clicked()), this, SLOT(openFilePressed()));

	QPushButton* dumpFile = new QPushButton("DMP", this);
	dumpFile->setGeometry(570, 10, 40, 20);
	connect(dumpFile, SIGNAL(clicked()), this, SLOT(dumpPressed()));


	u32label1 = new QLabel("U32 #1", this);
	u32label1->setGeometry(10, 30, 100, 20);
	u32label1Value = new QLabel("", this);
	u32label1Value->setGeometry(110, 30, 200, 20);
	
	u32label2 = new QLabel("U32 #2", this);
	u32label2->setGeometry(10, 50, 100, 20);
	u32label2Value = new QLabel("", this);
	u32label2Value->setGeometry(110, 50, 200, 20);
	
	u32label3 = new QLabel("U32 #3", this);
	u32label3->setGeometry(10, 70, 100, 20);
	u32label3Value = new QLabel("", this);
	u32label3Value->setGeometry(110, 70, 200, 20);
	
	u32label4 = new QLabel("U32 #4", this);
	u32label4->setGeometry(10, 90, 100, 20);
	u32label4Value = new QLabel("", this);
	u32label4Value->setGeometry(110, 90, 200, 20);
	
	u32label5 = new QLabel("U32 #5", this);
	u32label5->setGeometry(10, 110, 100, 20);
	u32label5Value = new QLabel("", this);
	u32label5Value->setGeometry(110, 110, 200, 20);

	palLabel = new QLabel("Palette", this);
	palLabel->setGeometry(10, 130, 100, 20);
	palLabelValue = new QLabel("", this);
	palLabelValue->setGeometry(110, 130, 400, 20);

	QPushButton* openPal = new QPushButton("...", this);
	openPal->setGeometry(520, 130, 40, 20);
	connect(openPal, SIGNAL(clicked()), this, SLOT(openPalPressed()));

	imageLabel = new QLabel("", this);
	paletteLabel = new QLabel("", this);
}

void MainWindow::openFile(const QString& filename)
{
	QFile datafile(filename);
	if (datafile.open(QIODevice::ReadOnly))
	{
		QByteArray data = datafile.readAll();

		// read 5 uint32_t
		uint32_t pos = 0;
		QLabel* labels[5] = { u32label1Value, u32label2Value, u32label3Value, u32label4Value, u32label5Value };
		uint32_t header[5];
		uint8_t* cdata = (uint8_t*)data.constData();
		for (uint32_t i = 0; i < 5; ++i)
		{
			uint32_t val = 0u;
			val |= ((uint32_t)(cdata[pos++]));
			val |= ((uint32_t)(cdata[pos++] << 8));
			val |= ((uint32_t)(cdata[pos++] << 16));
			val |= ((uint32_t)(cdata[pos++] << 24));
			labels[i]->setText(QString("%1").arg(val));
			header[i] = val;
		}

		// read image
		datafile.close();

		uint32_t width = header[3];
		uint32_t height = header[4];

		imageLabel->setGeometry(10, 190, width, height);
		QImage image(width, height, QImage::Format_RGBA8888);

		for (uint32_t y = 0; y < height; ++y)
		{
			for (uint32_t x = 0; x < width; ++x)
			{
				QColor c = raptorPalette[cdata[pos++]];
				image.setPixel(x, y, c.rgba());
			}
		}

		imageLabel->setPixmap(QPixmap::fromImage(image));
	}
}

void MainWindow::openFilePressed()
{
	QString filename = QFileDialog::getOpenFileName();
	if (filename != "")
	{
		openFile(filename);
	}
}

void MainWindow::openPalPressed()
{
	QString filename = QFileDialog::getOpenFileName();
	if (filename != "")
	{
		readPalette(filename);
	}
}

/* DUMP*/

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

void dump_files(std::vector<FileEntry>& file_entries, byte* glb_file)
{
	for (uint32_t i = 0; i < file_entries.size(); ++i)
	{
		std::string path = "d:/glb/dmp/";
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

	byte* decoded_buffer = (byte*)entry;
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


void MainWindow::dumpPressed()
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
	const char* files[5] = { "d:/glb/file0000.glb", "d:/glb/file0001.glb", "d:/glb/file0002.glb", "d:/glb/file0003.glb", "d:/glb/file0003.glb" };
	
	for (uint32_t i = 0; i < 5; ++i)
	{
		FILE* fp = fopen(files[i], "rb");
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
	}
}