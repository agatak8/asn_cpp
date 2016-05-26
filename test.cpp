/** \mainpage Dokumentacja projektu nr 3 na laboratoria PROI
 * \section autor Autor - Agata Kłoss
 * \section project_desc Opis projektu
 * 
 * 
 * 
 * \section compilation Kompilacja testów
 * Testy można skompilować przy użyciu cmake i make:
 * cmake $PWD && make
*/

/** \file
 * Name: test.cpp
 * Purpose: testy jednostkowe
 * @author: Agata Kłoss
*/ 

#include <iostream>
#include <fstream>
#include "asn.hpp"
#include "osoba.hpp"
#include "gtest/gtest.h"

TEST(INTEGER, writeToBuf)
{
    ASN_INTEGER x = 0x0123FF; // 74751
    BYTE_BUF buf;
    x.writeToBuf(buf);
    
    EXPECT_EQ(buf[0], 0x02);
    EXPECT_EQ(buf[1], 3);
    EXPECT_EQ(buf[2], 0x01);
    EXPECT_EQ(buf[3], 0x23);
    EXPECT_EQ(buf[4], 0xFF);

    x = 0x0000AEFF; // 1 bajt na 0 + 2 bajty
    x.writeToBuf(buf);
    EXPECT_EQ(buf[6], 3);
    EXPECT_EQ(buf[7], 0x00); // dodatkowy bajt 0 aby dobrze okreslic znak
    EXPECT_EQ(buf[8], 0xAE);
    EXPECT_EQ(buf[9], 0xFF);
    
    x = -413; // -413 = 0xFE63 w kodowaniu
    buf.clear();
    x.writeToBuf(buf);
    
    EXPECT_EQ(buf[0], 0x02);
    EXPECT_EQ(buf[1], 2);
    EXPECT_EQ(buf[2], 0xFE);
    EXPECT_EQ(buf[3], 0x63);
    
    x = -255;
    buf.clear();
    x.writeToBuf(buf);
    
    EXPECT_EQ(buf[0], 0x02);
    EXPECT_EQ(buf[1], 2);
    EXPECT_EQ(buf[2], 0xFF);
    EXPECT_EQ(buf[3], 0x01); // 0xFF01 = -255
}

TEST(INTEGER, readFromBuf)
{
    ASN_INTEGER x;
    BYTE_BUF buf;
    buf.push_back(0x02);
    buf.push_back(2);
    buf.push_back(0x01);
    buf.push_back(0xFF);
    x.readFromBuf(buf);
    
    EXPECT_EQ(x, 256 + 255); // 0x01FF = 256+255
    
    buf.clear();
    buf.push_back(0x02);
    buf.push_back(2);
    buf.push_back(0xFE);
    buf.push_back(0x63); // 0xFE63 = -413
    
    x.readFromBuf(buf);
    
    EXPECT_EQ(x, -413);
}



TEST(BITSTRING, writeToBuf)
{
    ASN_BITSTRING x;
    BIT_ARRAY z; // 5 bitow = 1 bajt, gdzie 3 bity unused
    BYTE_BUF buf;
    z.push_back(1);
    z.push_back(0);
    z.push_back(1);
    z.push_back(1);
    z.push_back(0); // 10110
    
    x = z;
    
    x.writeToBuf(buf);
    
    EXPECT_EQ(buf[0], 0x03);
    EXPECT_EQ(buf[1], 1+1); // + 1 na bajt unused;
    EXPECT_EQ(buf[2], 3);
    EXPECT_EQ(buf[3], 0xB0); // 10110000 = 0xB0
}

TEST(BITSTRING, readFromBuf)
{
    ASN_BITSTRING x;
    BYTE_BUF buf;
    
    buf.push_back(0x03);
    buf.push_back(1+2);
    buf.push_back(5); // 3 unused
    buf.push_back(0xF0); // 1111 0000
    buf.push_back(0xBF); // 1011 1111
    
    x.readFromBuf(buf);
    
    //EXPECT_EQ(x.bits.size(), 8+3);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ(x[i], 1);
    }
    for (int i = 4; i < 8; i++)
    {
        EXPECT_EQ(x[i], 0);
    }
    EXPECT_EQ(x[8], 1);
    EXPECT_EQ(x[9], 0);
    EXPECT_EQ(x[10], 1);
}

TEST(ENUMERATED, readFromBuf)
{
    ASN_ENUMERATED x;
    std::unordered_map<std::string, int> f = {{"x", 0}, {"y", 1}, {"z", 2}};
    x = f;
    BYTE_BUF buf;
    
    buf.push_back(0x02);
    buf.push_back(1);
    
    buf.push_back(0);

    x.readFromBuf(buf);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(std::string(x), "x");
    
    buf[2] = 1;
    x.readFromBuf(buf);
    EXPECT_EQ(std::string(x), "y");
    
    buf[2] = 2;
    x.readFromBuf(buf);
    EXPECT_EQ(std::string(x), "z");
    
    buf[2] = 3;
    EXPECT_ANY_THROW(x.readFromBuf(buf)); // 3 nie jest w slowniku
    
    buf[0] = 0x00;
    EXPECT_ANY_THROW(x.readFromBuf(buf)); // zly tag
}

TEST(UTF8STRING, writeToBuf)
{
    const char* utf8c = u8"Ćma"; // 4 bajty + 0x00
    BYTE_BUF buf;
    
    ASN_UTF8STRING x(utf8c);
    x.writeToBuf(buf);
    EXPECT_EQ(buf[0], 0x0c);
    EXPECT_EQ(buf[1], 4);
    
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ((char)buf[i+2], utf8c[i]);
    }
    
}

TEST(UTF8STRING, readFromBuf)
{
    ASN_UTF8STRING x;
    BYTE_BUF buf;
    buf.push_back(0x0C);
    buf.push_back(2);
    buf.push_back("ą"[0]);
    buf.push_back("ą"[1]);
    
    x.readFromBuf(buf);
    
    EXPECT_EQ(std::string(x), "ą");
    
}

TEST(SEQUENCE, readWriteBuf)
{
    class test_seq: public ASN_SEQUENCE
    {
        public:
            ASN_INTEGER a;
            ASN_UTF8STRING b;
            
            test_seq(int a1, std::string b1): a(a1), b(b1)
            {
                object_ptrs = {&a, &b};
            }
    };
    
    test_seq test(13, "Masło");
    
    BYTE_BUF buf;
    
    test.writeToBuf(buf);
    test.a = 0;
    test.b = "";
    test.readFromBuf(buf);
    
    EXPECT_EQ(test.a, 13);
    EXPECT_EQ(std::string(test.b), "Masło");
}

TEST(OSOBA, file)
{
    Osoba m, n;
    m.imie = "Mateusz";
    m.nazwisko = "Nowak";
    m.wiek = 25;
    m.plec = 1;
    m.zainteresowania = NARTY | LYZWY; // = 3
    
    m.writeToFile("mateusz.bin");
    n.readFromFile("mateusz.bin");
    
    EXPECT_EQ(m.imie, n.imie);
    EXPECT_EQ(m.nazwisko, n.nazwisko);
    EXPECT_EQ(m.wiek, n.wiek);
    EXPECT_EQ(m.plec, n.plec);
    EXPECT_EQ(m.zainteresowania, n.zainteresowania);
}

// testy na duzej ilosci danych

TEST(INTEGER, manyInts)
{
    std::ifstream f("test_int.txt"); // 10000 losowych intow
    if (f.is_open())
    {
        ASN_INTEGER z;
        int tmp;
        BYTE_BUF buf;
        while (f >> tmp)
        {
            buf.clear();
            z = tmp;
            z.writeToBuf(buf);
            z = 0;
            z.readFromBuf(buf);
            EXPECT_EQ(z, tmp);
        }
        f.close();
    }
    
}

TEST(UTF8STRING, bigLength)
{
    BYTE_BUF buf;
    ASN_UTF8STRING z;
    std::string m = "πrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyokoπrazyoko"; // > 128 bajtow
    z = m;
    z.writeToBuf(buf);
    z = "";
    z.readFromBuf(buf);
    
    EXPECT_EQ((std::string)z, m);
}
TEST(UTF8STRING, manyStrings)
{
    std::ifstream f("test_str.txt"); // 10000 losowych stringow utf8
    if (f.is_open())
    {
        ASN_UTF8STRING z;
        std::string tmp;
        BYTE_BUF buf;
        while (getline(f, tmp))
        {
            buf.clear();
            z = tmp;
            z.writeToBuf(buf);
            z = "";
            z.readFromBuf(buf);
            EXPECT_EQ((std::string)z, tmp);
        }
        f.close();
    }
}

int main(int argc, char **argv) 
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
