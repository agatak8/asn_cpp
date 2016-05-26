/** \file
 * Name: asn.cpp
 * Purpose: metody klas ASN
 * @author: Agata Kłoss
*/ 

#include "asn.hpp"


void ASN_INTEGER::writeToBuf(BYTE_BUF &buf)
{
    if (!isSet)
    {
        throw(Exception(EMPTY_OBJECT));
    }
    writeTLToBuf(buf);
    
    if(value & (0x01 << 8*length - 1)) // jesli liczba jest dodatnia i jej najstarszy bit = 1
    {
        if (value > 0)
            buf.push_back(0x00); // to musimy dodac bajt zer
    }
    
    for(int i = 0; i < length; i++)
    {
        buf.push_back((value & (0xFF << 8*(length - i - 1))) >> 8*(length - i - 1));
        // dla i = 0 -> length - i - 1 = length - 1 -> najstarszy bajt
        // dla i = length - 1 -> length - i - 1 = 0 -> najmłodszy bajt
        // 0xFF << 8*(length-i-1) - maska 1111 1111 przesunięta na dany bajt
        // value & maska -> bajt inta na danej pozycji
        // bajt >> 8*(length-i-1) -> bajt dosunięty do prawej czyli
        //                           wartość szesnastkowa tego bajtu
        //                           którą zapisujemy do bufora
    }
    
    
}

void ASN_INTEGER::readFromBuf(const BYTE_BUF &buf, uint offset)
{
   if (buf[offset] != tag) throw(Exception(INVALID_TAG));
   else
   {
       std::pair<int,int> length_offset = readLength(buf,offset);
       length = length_offset.first; // ile bajtów do odczytu
       uint off = length_offset.second; // ile bajtów ominąć (które kodują długość) - 1
       
       value = 0;
       for(int i = 0; i < length - 1; i++)
       {
           value += buf[offset + (length - i + 1) + off] << 8*i; // dodawanie danego bajtu * jego waga w systemie 256
       }
       
       // ostatni bajt, zawiera on bit znaku
       BYTE tmp_val = (buf[offset + 2 + off]);
       if (tmp_val & 0x80) // int ujemny
       {
           value += (tmp_val - 0x80) << 8*(length - 1); // dodajemy wartość 7 ostatnich bitów
           value -= (0x80 << 8*(length - 1)); // odejmujemy wagę 1-go bitu
       }
       else // int dodatni, normalna wartość
       {
           value += (tmp_val << 8*(length - 1));
       }

   }
   isSet = true;
}



void ASN_BITSTRING::writeToBuf(BYTE_BUF &buf)
{
    
    if (bits.size() == 0)
    {
        throw(Exception(EMPTY_OBJECT));
    }
    
    setLength();
    writeTLToBuf(buf);
    
    BYTE byte_tmp, bit_tmp;
    
    uint bit_size = bits.size(); // licznik bitów pozostałych do przeróbki
    int8 unused = 8 - (bit_size % 8); // nieużywane bity na końcu ostatniego bajtu
    unsigned int j_limit; // ile bitów zamieniamy na bajt w pętli for z j
    
    buf.push_back(unused);
    
    for(uint i = 0; i < length - 1; i++) // i-ty bajt ciągu binarnego
    {
        if (bit_size >= 8)
            j_limit = 8;
        else
            j_limit = bit_size;
            
        byte_tmp = 0;
        for(uint j=0; j < j_limit; j++)
        {
            bit_tmp = (bits[8*i+j]); // j-ty bit w i-tym bajcie ciągu
            
            byte_tmp += (int8(bit_tmp) << 7-j); // dodajemy go "odwracajac" starszenstwo bitow w bajcie
                                                // ponieważ pozycje w tablicy są w odwrotnej kolejności niż wagi
        }
        bit_size -= j_limit;
        buf.push_back(byte_tmp); // gotowy bajt do paczki
    }
}

void ASN_BITSTRING::readFromBuf(const BYTE_BUF &buf, uint offset)
{
    if (buf[offset] != tag) throw ("Not an ASN BITSTRING");
    
    std::pair<int,int> length_offset = readLength(buf,offset);
    length = length_offset.first; // ile bajtów do odczytu (wraz z bajtem unused)
    uint off = length_offset.second; // ile bajtów do pominięcia (kodujących długość)
    
    int8 unused = buf[offset + 2 + off];
    
    int j_limit; // ile bitów przerabiamy
    
    
    bits.clear();
    
    BIT bit_tmp;
    BYTE byte_tmp;
    
    for(int i=0; i < length - 1; i++) // length - 1, bo bez bajtu unused
    {
        byte_tmp = buf[offset + (i+3) + off];
        
        
        if (i != length - 2)
            j_limit = 8;
        else
            j_limit = 8 - unused; // ostatnie bity
            
        for(int j = 0; j < j_limit; j++)
        {
            bits.push_back(byte_tmp & (1 << (7-j))); // j-ty bit w i-tym bajcie bits = (7-j)-ty bit w byte_tmp
        }
    }
    setLength();
    isSet = true;
}

void ASN_ENUMERATED::readFromBuf(const BYTE_BUF &buf, uint offset)
{
   if (buf[offset] != tag) throw(Exception(INVALID_TAG));
   else
   {
       std::pair<int,int> length_offset = readLength(buf,offset);
       length = length_offset.first; // ile bajtów do odczytu
       uint off = length_offset.second; // ile bajtów do pominięcia (kodujących długość)
       
       int tmp_value;


       tmp_value = 0;
       for(int i = 0; i < length; i++)
       {
           tmp_value += buf[offset + (length - i + 1) + off] << 8*i; // zaczynamy od najmłodszego bajtu, kończymy na najstarszym
       }
       
       if (isInDict(tmp_value)) // sprawdzamy czy jest w mapie
       {
           value = tmp_value;
       }
       else
       {
           throw(Exception(WRONG_ENUM_VALUE));
       }
   }
   
   isSet = true;
}

void ASN_UTF8STRING::readFromBuf(const BYTE_BUF &buf, uint offset)
{
    if (buf[offset] != tag)
    {
        throw(Exception(INVALID_TAG));
    }
    
    std::pair<int,int> length_offset = readLength(buf,offset);
    length = length_offset.first; // ile bajtów do odczytu
    uint off = length_offset.second; // ile bajtów do pominięcia (kodujących długość)

    utf8string.clear();
    
    for (int i = 0; i < length; i++)
    {
        utf8string.push_back(buf[offset + (i+2) + off]);
    }
    isSet = true;
}

void ASN_UTF8STRING::writeToBuf(BYTE_BUF &buf)
{
    if (!isSet)
    {
        throw(Exception(EMPTY_OBJECT));
    }
    
    writeTLToBuf(buf);
    
    for (auto it = utf8string.begin(); it != utf8string.end(); it++)
    {
        buf.push_back(*it);
    }
}

void ASN_SEQUENCE::writeToBuf(BYTE_BUF &buf)
{
    setLength();
    writeTLToBuf(buf);
    
    for (auto it = object_ptrs.begin(); it != object_ptrs.end(); it++)
    {
        (*it)->writeToBuf(buf);
    }
}

void ASN_SEQUENCE::readFromBuf(const BYTE_BUF &buf, uint offset)
{
    if (buf[offset] != tag)
        throw(Exception(INVALID_TAG));
        
    std::pair<int,int> length_offset = readLength(buf,offset);
    length = length_offset.first; // ile bajtów do odczytu
    uint off_seq = length_offset.second; // ile bajtów do pominięcia (kodujących długość)

    
    int off = offset + 2 + off_seq; // gdzie rozpoczynamy odczyt
    int counter = 0; // licznik przerobionych bajtów
    for (auto it = object_ptrs.begin(); it != object_ptrs.end(); it++)
    {
        if (counter == length)
            break; // odczytaliśmy length bajtów, koniec
        (*it)->readFromBuf(buf, counter+off); // counter + off = gdzie zaczyna się kolejny obiekt
        counter += (*it)->fullSize();
    }
}
