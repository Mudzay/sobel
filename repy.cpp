#include <iostream>
#include <fstream>
#include <string>
#pragma pack(1) //wyrownanie pol struktur do 1 bajta zeby wyeliminowac wyrownywanie pol struktur do 4 bajtow

using namespace std;

typedef unsigned char byte;

const bool srednia = false;

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

const int s[8][3][3] = {
	{
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	},
	{
		{1, 0, -1},
		{2, 0, -2},
		{1, 0, -1}
	},
	{
		{1,   2,  1},
		{0,   0,  0},
		{-1, -2, -1}
	},
	{
		{-1, -2, -1},
		{0,   0,  0},
		{1,   2,  1}
	},
	{
		{0,   1, 2},
		{-1,  0, 1},
		{-2, -1, 0}
	},
	{
		{0, -1, -2},
		{1,  0, -1},
		{2,  1,  0}
	},
	{
		{2,  1,  0},
		{1,  0, -1},
		{0, -1, -2}
	},
	{
		{-2, -1, 0},
		{-1,  0, 1},
		{0,   1, 2}
	}
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
	ifs.read(reinterpret_cast<char*>(&bi.xppm), 4);
	ifs.read(reinterpret_cast<char*>(&bi.yppm), 4);
	ifs.read(reinterpret_cast<char*>(&bi.colors_used), 4);
	ifs.read(reinterpret_cast<char*>(&bi.colors_important), 4);
}

void readpixel(ifstream& ifs, BMP& plik) {
	ifs.seekg(plik.header.offset_data, ios::beg);
	plik.piksele = new pixel[plik.info.width * plik.info.height];
	for (int i = 0; i < plik.info.height; i++) {
		for (int j = 0; j < plik.info.width; j++)
			ifs.read(reinterpret_cast<char*>(&plik.piksele[i * plik.info.width + j]), 3);
		ifs.seekg(plik.info.width % 4, ios::cur);
		/*
			padding:
			ilosc bitow paddingu jest rowna szerokosc % 4, poniewaz:
			p = szerokosc % 4
			(szerokosc * 3 + p) % 4 jest rowna 0 dla kazdej liczby naturalnej (a w pliku szerokosc moze byc tylko liczba naturalna)
		*/
	}
}

int sprawdz(BMP plik) {
	if (plik.header.file_type != 0x4D42) {
		cout << "Niepoprawny typ pliku." << endl;
		return -1;
	}
	if (plik.info.height < 3 || plik.info.width < 3) {
		cout << "Plik jest za maly.";
		return -1;
	}
	if (plik.info.bit_count != 24) {
		cout << "Nieobslugiwana bitmapa";
		return -1;
	}
}

int readBMP(BMP& plik, string path) {
	ifstream ifs;
	ifs.open(path, ios::binary);
	if (!ifs.is_open()) {
		cout << "Plik nie istnieje." << endl;
		return -1;
	}
	readheader(ifs, plik.header);
	readinfo(ifs, plik.info);
	readpixel(ifs, plik);
	ifs.close();
	return 0;
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
	if (srednia) {
		tR /= 8;
		tG /= 8;
		tB /= 8;
	}
	if (tR > p.R)
		p.R = tR;
	if (tG > p.G)
		p.G = tG;
	if (tB > p.B)
		p.B = tB;
}

void wykrywanie(BMP plik) {
	int width = plik.info.width;
	pixel* temp = new pixel[plik.info.height * width];
	for (int i = 1; i < plik.info.height - 1; i++)
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
			for (int m = 0; m < 8; m++)
				maska(zr, temp[i * width + j], s[m]);
		}
	copy(temp, temp + plik.info.height * width, plik.piksele);
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

void draw_logo() {
	cout << "--------------------------------------------------------------------" << endl;
	string logo[8] = {
		"| ad88888ba     ,ad8888ba,    88888888ba   88888888888  88         |",
		"|d8\"     \"8b   d8\"'    `\"8b   88      \"8b  88           88         |",
		"|Y8,          d8'        `8b  88      ,8P  88           88         |",
		"|`Y8aaaaa,    88          88  88aaaaaa8P'  88aaaaa      88         |",
		"|  `\"\"\"\"\"8b,  88          88  88\"\"\"\"\"\"8b,  88\"\"\"\"\"      88         |",
		"|        `8b  Y8,        ,8P  88      `8b  88           88         |",
		"|Y8a     a8P   Y8a.    .a8P   88      a8P  88           88         |",
		"| \"Y88888P\"     `\"Y8888Y\"'    88888888P\"   88888888888  88888888888|"
	};
	for (string l : logo)
		cout << l << endl;
	cout << "--------------------------------------------------------------------" << endl << endl;
}

void czarnalinia(ofstream& ofs, int width)
{
	pixel z;
	for (int i = 0; i < width; i++)
		ofs.write((char*)&z, 3);
	ofs.seekp(width % 4, ios::cur);
}

void czesci(BMP& plik, string path, string path_out) {
	ifstream ifs;
	ifs.open(path, ios::binary);
	readheader(ifs, plik.header);
	readinfo(ifs, plik.info);
	ifs.seekg(plik.header.offset_data, ios::beg);

	ofstream ofs;
	ofs.open(path_out, ios::binary);
	ofs.write((char*)&plik.header, sizeof(plik.header));
	ofs.write((char*)&plik.info, sizeof(plik.info));
	ofs.seekp(plik.header.offset_data + plik.info.width*3 + (plik.info.width%4), ios::beg);

	pixel zr[9];

	plik.piksele = new pixel[3 * plik.info.width];
	pixel* wynik = new pixel[plik.info.width];
	pixel zero;

	//czarnalinia(ofs, plik.info.width);

	for (int seg = 0; seg < plik.info.height - 2; seg++) {
		
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < plik.info.width; j++)
				ifs.read(reinterpret_cast<char*>(&plik.piksele[i * plik.info.width + j]), 3);
			ifs.seekg(plik.info.width % 4, ios::cur);
		}
		ifs.seekg(-2 * ((plik.info.width * 3) + 2*(plik.info.width % 4)), ios::cur);
		for (int i = 1; i < plik.info.width - 1; i++) {
			zr[0] = plik.piksele[i - 1];
			zr[1] = plik.piksele[i];
			zr[2] = plik.piksele[i + 1];
			zr[3] = plik.piksele[plik.info.width + i - 1];
			zr[4] = plik.piksele[plik.info.width + i];
			zr[5] = plik.piksele[plik.info.width + i + 1];
			zr[6] = plik.piksele[2 * plik.info.width + i - 1];
			zr[7] = plik.piksele[2 * plik.info.width + i];
			zr[8] = plik.piksele[2 * plik.info.width + i + 1];
			for (int k = 0; k < 8; k++)
				wynik[i] = plik.piksele[plik.info.width + i];
				//maska(zr, wynik[i], s[k]);
		}

		for (int i = 0; i < plik.info.width; i++)
			ofs.write((char*)&wynik[i], 3);
		ofs.seekp(plik.info.width % 4, ios::cur);
	}
	


	czarnalinia(ofs, plik.info.width);
	ofs.close();
	ifs.close();

}

int main()
{
	draw_logo();
	string path = "test.bmp";
	string path_out = "testcz.bmp";
	if (false) {
		cout << "Wpisz nazwe pliku: ";

		cin >> path;

		cout << "Wpisz nazwe pliku wyjsciowego: ";

		cin >> path_out;
	}
	BMP plik;
	if (false) {
		if (readBMP(plik, path) == -1)
			return -1;
		if (sprawdz(plik) == -1)
			return -1;
		printinfo(plik);
		wykrywanie(plik);
		saveBMP(plik, path_out);

		delete[] plik.piksele;
	}
	czesci(plik, path, path_out);
} 
