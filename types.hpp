/** \file
 * Name: types.hpp
 * Purpose: typy pomocnicze używane przez klasy ASN
 * @author: Agata Kłoss
*/ 

#ifndef TYPES_H_
#define TYPES_H_
#include <iostream>
#include <fstream>
#include <vector>

#include "except.hpp"
/// interfejs zapisywania do pliku
class IStorable
{
	public:
		virtual void writeToFile(std::string filename) = 0;
};

/// interfejs wyświetlania na ekran
class IDisplayable
{
	public:
		virtual void display() = 0;
};

typedef unsigned char BYTE;
typedef bool BIT;
/// ciąg binarny do BITSTRING
typedef std::vector<BIT> BIT_ARRAY;
typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned int uint;

/// bufor bajtów
class BYTE_BUF: public std::vector<BYTE>, public IStorable
{
    public:
        void writeToFile(std::string filename)
        {
            std::ofstream file;
            file.open(filename, std::ios::out | std::ios::binary);
            
            if (file.is_open())
            {
                file.write((char*)data(), (int)size());
                file.close();
            }
            else
            {
                throw(Exception(FILE_NOT_OPEN));
            }
        }
        
        void readFromFile(std::string filename)
        {
            std::ifstream file;
            file.open(filename, std::ios::in | std::ios::binary | std::ios::ate);
            if (file.is_open())
            {
                int size = file.tellg();
                resize(size);
                
                file.seekg (0, std::ios::beg);
                file.read((char*)data(), size);
                file.close();
            }
            else
            {
                throw(Exception(FILE_NOT_OPEN));
            }
        }
};

#endif
