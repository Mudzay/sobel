#include <iostream>
#include <fstream>

#pragma pack(1) //wyrownanie pol struktur do 2 bajtow

typedef unsigned char byte;

using namespace std;



struct BMPheader { //naglowek pliku
	uint16_t file_type = 0x4D42; //typ pliku, dla BMP zawsze 0x4D42
	uint32_t file_size = 0; //rozmiar pliku w bajtach
	uint16_t reserved1 = 0; //Pole zarezerwowane (zwykle zero)
	uint16_t reserved2 = 0; //Pole zarezerwowane (zwykle zero)
	uint32_t offset_data = 0; //pozycja danych obrazowych w pliku
};

struct BMPinfo { //naglowek obrazu
	uint32_t size = 0; //rozmiar naglowka informacyjnego
	int32_t width = 0; //szerokosc w pikselach
	int32_t height = 0; //wysokosc w pikselach
	uint16_t planes = 1; //liczba platow, musi byc 1
	uint16_t bit_count = 24; //liczba bitow na piksel (1/4/8/16/24/32)
	uint32_t compression = 0; //algorytm kompresji
	uint32_t size_image = 0; //rozmiar rysunku
	int32_t xppm = 0; //rozdzielczosc pozioma
	int32_t yppm = 0; //rozdzielczosc pionowa
	uint32_t colors_used = 0; //liczba uzytych kolorow
	uint32_t colors_important = 0; //liczba waznych kolorow
};

struct pixel {
	byte B = 0;
	byte G = 0;
	byte R = 0;
};

struct BMP {
	BMPheader header;
	BMPinfo info;
	pixel* piksele;
};


const int s1[3][3] = {
	{-1, 0, 1},
	{-2, 0, 2},
	{-1, 0, 1}
};

const int s2[3][3] = {
	
	{1, 0, -1},
	{2, 0, -2},
	{1, 0, -1}
};

const int s3[3][3] = {
	{1,   2,  1},
	{0,   0,  0},
	{-1, -2, -1}
};

const int s4[3][3] = {
	{-1, -2, -1},
	{0,   0,  0},
	{1,   2,  1}
};

const int s5[3][3] = {
	{0,   1, 2},
	{-1,  0, 1},
	{-2, -1, 0}
};

const int s6[3][3] = {
	{0, -1, -2},
	{1,  0, -1},
	{2,  1,  0}
};

const int s7[3][3] = {
	{2,  1,  0},
	{1,  0, -1},
	{0, -1, -2}
};

const int s8[3][3] = {
	{-2, -1, 0},
	{-1,  0, 1},
	{0,   1, 2}
};

void readheader(ifstream& ifs, BMPheader& bfh) {
	//wczytywanie danych naglowka pliku
	ifs.read(reinterpret_cast<char*>(&bfh.file_type), 2);
	ifs.read(reinterpret_cast<char*>(&bfh.file_size), 4);
	ifs.read(reinterpret_cast<char*>(&bfh.reserved1), 2);
	ifs.read(reinterpret_cast<char*>(&bfh.reserved2), 2);
	ifs.read(reinterpret_cast<char*>(&bfh.offset_data), 4);
}

void readinfo(ifstream& ifs, BMPinfo& bi) {
	//wczytywanie danych naglowka obrazu
	ifs.read(reinterpret_cast<char*>(&bi.size), 4);
	ifs.read(reinterpret_cast<char*>(&bi.width), 4);
	ifs.read(reinterpret_cast<char*>(&bi.height), 4);
	ifs.read(reinterpret_cast<char*>(&bi.planes), 2);
	ifs.read(reinterpret_cast<char*>(&bi.bit_count), 2);
	ifs.read(reinterpret_cast<char*>(&bi.compression), 4);
	ifs.read(reinterpret_cast<char*>(&bi.size_image), 4);
	if (bi.size_image == 0)
		bi.size_image = bi.height * (bi.width + (bi.width % 4));
	/*
		w przypadku niepoprawnej wartosci size_image automatycznie ja wyliczam
		zdarzalo sie, ze obrazy z internetu mialy ta wartosc ustawiona na 0
	*/ 
	ifs.read(reinterpret_cast<char*>(&bi.xppm), 4);
	ifs.read(reinterpret_cast<char*>(&bi.yppm), 4);
	ifs.read(reinterpret_cast<char*>(&bi.colors_used), 4);
	ifs.read(reinterpret_cast<char*>(&bi.colors_important), 4);
}





void readpixel(ifstream& ifs, BMP& plik) {
	byte smietnik;
	ifs.seekg(plik.header.offset_data, ios::beg);
	plik.piksele = new pixel[plik.info.size_image];
	for (int i = 0; i < plik.info.height; i++) {
		for (int j = 0; j < plik.info.width; j++)
			ifs.read(reinterpret_cast<char*>(&plik.piksele[i * plik.info.width + j]), 3);
		for (int j = 0; j < plik.info.width % 4; j++)
			ifs.read(reinterpret_cast<char*>(&smietnik), 1);

	}
}

void readBMP(BMP& plik, string path) {
	ifstream ifs;
	ifs.open(path, ios::binary);
	readheader(ifs, plik.header);
	readinfo(ifs, plik.info);
	readpixel(ifs, plik);
	ifs.close();
}

void printinfo(BMP plik) {
	cout << "File type: " << plik.header.file_type << endl;
	cout << "File size: " << plik.header.file_size << endl;
	cout << "Reserved 1: " << plik.header.reserved1 << endl;
	cout << "Reserved 2: " << plik.header.reserved2 << endl;
	cout << "Offset: " << plik.header.offset_data << endl << endl;
	cout << "Info header size: " << plik.info.size << endl;
	cout << "Image width: " << plik.info.width << endl;
	cout << "Image height: " << plik.info.height << endl;
	cout << "Planes: " << plik.info.planes << endl;
	cout << "Bit count: " << plik.info.bit_count << " (" << plik.info.bit_count / 8 << " bytes per pixel)" << endl;
	cout << "Compression: " << plik.info.compression << endl;
	cout << "Image size: " << plik.info.size_image << endl;
	cout << "X resolution: " << plik.info.xppm << endl;
	cout << "Y resolution: " << plik.info.yppm << endl;
	cout << "Colors used: " << plik.info.colors_used << endl;
	cout << "Important colors: " << plik.info.colors_important << endl;
	cout << "sizeof(plik.header): " << sizeof(plik.header) << endl;
	cout << "sizeof(plik.info): " << sizeof(plik.info) << endl;
}

void zerujkrawedzie(pixel* temp, uint32_t szerokosc, uint32_t wysokosc) {
	pixel zero;
	for (int i = 0; i < szerokosc; i++) {
		temp[i] = zero;
		temp[szerokosc * (wysokosc - 1) + i] = zero;
	}
	for (int i = 1; i < wysokosc - 1; i++) {
		temp[i * szerokosc] = zero;
		temp[(i + 1) * szerokosc - 1] = zero;
	}

}

void mapuj(int& tR, int& tG, int& tB) {
	if (tR > 255)
		tR = 255;
	if (tG > 255)
		tG = 255;
	if (tB > 255)
		tB = 255;
	if (tR < 0)
		tR = 0;
	if (tG < 0)
		tG = 0;
	if (tB < 0)
		tB = 0;
	
}

void maska(pixel zr[9], pixel& p, const int m[3][3]) {
	int tR = 0;
	int tG = 0;
	int tB = 0;
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++) {
			tR += (int)zr[i * 3 + j].R * (int)m[i][j];
			tG += (int)zr[i * 3 + j].G * (int)m[i][j];
			tB += (int)zr[i * 3 + j].B * (int)m[i][j];
		}
	
	mapuj(tR, tG, tB);
	if (tR >= p.R)
		p.R = tR;
	if (tG >= p.G)
		p.G = tG;
	if (tB >= p.B)
		p.B = tB;
}


void wykrywanie(BMP plik, const int m[3][3]) {
	int width = plik.info.width;
	int height = plik.info.height;
	pixel* temp = new pixel[plik.info.size_image];
	for (int i = 1; i < height - 1; i++)
		for (int j = 1; j < width - 1; j++) {
			pixel zr[9];
			zr[0] = plik.piksele[(i - 1) * width + j - 1];
			zr[1] = plik.piksele[(i - 1) * width + j];
			zr[2] = plik.piksele[(i - 1) * width + j + 1];
			zr[3] = plik.piksele[i * width + j - 1];
			zr[4] = plik.piksele[i * width + j];
			zr[5] = plik.piksele[i * width + j + 1];
			zr[6] = plik.piksele[(i + 1) * width + j - 1];
			zr[7] = plik.piksele[(i + 1) * width + j];
			zr[8] = plik.piksele[(i + 1) * width + j + 1];
			maska(zr, temp[i * width + j], s1);
			maska(zr, temp[i * width + j], s2);
			maska(zr, temp[i * width + j], s3);
			maska(zr, temp[i * width + j], s4);
			maska(zr, temp[i * width + j], s5);
			maska(zr, temp[i * width + j], s6);
			maska(zr, temp[i * width + j], s7);
			maska(zr, temp[i * width + j], s8);
		}
	zerujkrawedzie(temp, width, height);
	copy(temp, temp + plik.info.size_image, plik.piksele);
	delete[] temp;
}

void saveBMP(BMP plik, string path) {
	ofstream ofs;
	ofs.open(path, ios::binary);
	byte z = 0;
	ofs.write((char*)&plik.header, sizeof(plik.header));
	ofs.write((char*)&plik.info, sizeof(plik.info));
	ofs.seekp(plik.header.offset_data, ios::beg);
	int szerokosc = plik.info.width - plik.info.width % 4;
	for (int i = 0; i < plik.info.height; i++) {
		for (int j = 0; j < plik.info.width; j++)
			ofs.write((char*)&plik.piksele[i * plik.info.width + j], 3);
		for (int j = 0; j < plik.info.width % 4; j++)
			ofs.write((char*)&z, 1);
	}
	ofs.close();
}

int main()
{

	cout << "Wpisz nazwe pliku: ";
	string path;
	cin >> path;

	BMP plik;
	readBMP(plik, path);
	printinfo(plik);
	wykrywanie(plik, s1);

	cout << "Wpisz nazwe pliku wyjsciowego: ";
	string path_out;
	cin >> path_out;
	
	saveBMP(plik, path_out);
	
	
}
