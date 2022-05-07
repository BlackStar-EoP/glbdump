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