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
        throw("cannot write empty ASNobject");
    }
    writeTLToBuf(buf);
    
    if(value & (0x01 << 8*length - 1)) // dodatnia i najstarszy bit = 1
    {
        if (value > 0)
            buf.push_back(0x00); // dodajemy zera dla znaku
    }
    
    buf.push_back((value & (0xFF << 8*(length - 1))) >> 8*(length - 1));
    
    for(int i = 1; i < length; i++)
    {
        buf.push_back((value & (0xFF << 8*(length - i - 1))) >> 8*(length - i - 1)); // nakladamy maske i dosuwamy do prawej aby wyluskac interesujacy nas bajt
    }
    
    
}

void ASN_INTEGER::readFromBuf(const BYTE_BUF &buf, uint offset)
{
   if (buf[offset] != tag) throw("not an ASN integer");
   else
   {
       std::pair<int,int> length_offset = readLength(buf,offset);
       length = length_offset.first;
       uint off = length_offset.second;
       value = 0;
       for(int i = 0; i < length - 1; i++)
       {
           value += buf[offset + (length - i + 1) + off] << 8*i;
       }
       
       // ostatni bajt
       BYTE tmp_val = (buf[offset + 2 + off]);
       if (tmp_val & 0x80) // int ujemny
       {
           value += (tmp_val - 0x80) << 8*(length - 1);
           value -= (0x80 << 8*(length - 1));
       }
       else
       {
           value += (tmp_val << 8*(length - 1));
       }

   }
   isSet = true;
}



void ASN_BITSTRING::writeToBuf(BYTE_BUF &buf)
{
    
    if (!isSet)
    {
        throw("cannot write empty ASNobject");
    }
    
    writeTLToBuf(buf);
    
    BYTE byte_tmp, bit_tmp;
    
    uint bit_size = bits.size();
    int8 unused = 8 - (bit_size % 8);
    unsigned int j_limit;
    
    buf.push_back(unused);
    
    for(uint i = 0; i < length - 1; i++) // i-ty bajt zawartosci
    {
        if (bit_size >= 8)
            j_limit = 8;
        else
            j_limit = bit_size;
            
        byte_tmp = 0;
        for(uint j=0; j < j_limit; j++)
        {
            bit_tmp = (bits[8*i+j]); // j-ty bit w i-tym "bajcie" w bits
            
            byte_tmp += (int8(bit_tmp) << 7-j); // dodajemy go "odwracajac" starszenstwo bitow w bajcie
        }
        bit_size -= j_limit; 
        buf.push_back(byte_tmp); // gotowy bajt do paczki
    }
}

void ASN_BITSTRING::readFromBuf(const BYTE_BUF &buf, uint offset)
{
    if (buf[offset] != tag) throw ("Not an ASN BITSTRING");
    
    std::pair<int,int> length_offset = readLength(buf,offset);
    length = length_offset.first;
    uint off = length_offset.second;
    
    //length = buf[offset + 1]; // ilosc bajtow wlacznie z unused
    int8 unused = buf[offset + 2 + off];
    
    int j_limit;
    
    
    bits.clear();
    
    BIT bit_tmp;
    BYTE byte_tmp;
    
    for(int i=0; i < length - 1; i++)
    {
        byte_tmp = buf[offset + (i+3) + off];
        
        
        if (i != length - 2)
            j_limit = 8;
        else
            j_limit = 8 - unused;
            
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
   if (buf[offset] != tag) throw("not an ASN enumerated");
   else
   {
       std::pair<int,int> length_offset = readLength(buf,offset);
       length = length_offset.first;
       uint off = length_offset.second;
       
       int tmp_value;
       //length = buf[offset+1];


       tmp_value = 0;
       for(int i = 0; i < length; i++)
       {
           tmp_value += buf[offset + (length - i + 1) + off] << 8*i;
       }
       
       if (isInDict(tmp_value))
       {
           value = tmp_value;
       }
       else
       {
           throw("ASN_ENUMERATED: value not in value dict");
       }
   }
   
   isSet = true;
}

void ASN_UTF8STRING::readFromBuf(const BYTE_BUF &buf, uint offset)
{
    if (buf[offset] != tag)
    {
        throw("not an ASN_UTF8STRING");
    }
    
    std::pair<int,int> length_offset = readLength(buf,offset);
    length = length_offset.first;
    uint off = length_offset.second;
    //length = buf[offset + 1];
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
        throw("cannot write empty ASNobject");
    }
    
    writeTLToBuf(buf);
    
    for (auto it = utf8string.begin(); it != utf8string.end(); it++)
    {
        buf.push_back(*it);
    }
}
