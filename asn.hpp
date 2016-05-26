/** \file
 * Name: asn.hpp
 * Purpose: klasy ASN
 * @author: Agata Kłoss
*/ 

#ifndef ASN_HEADER
#define ASN_HEADER

#include <iostream>
#include <cstdio>
#include <unordered_map>
#include <initializer_list>

#include "types.hpp"


class ASNobject: public IStorable
{
    protected:
        BYTE tag;
        uint length;
        bool isSet = false;
        
	public:
        int fullSize() // calkowity rozmiar w bajtach
        {
            if (length < 128)
                return 2 + length;  // 1 na tag,
                                    // 1 na dlugosc,
                                    // length na bajty zawartosci
            else
            {
                uint tmp_len = length;
                BYTE len_of_len = 1;
                while(tmp_len = (tmp_len >> 8))
                {
                    len_of_len++;
                }
                return 1 + 1 + len_of_len + length; // 1 na tag, 
                                                // 1 na ile bajtow kodujacych dlugosc, 
                                                // len_of_len na bajty kodujace dlugosc, 
                                                // length na bajty zawartosci
            }
        }
		virtual void writeToBuf(BYTE_BUF &buf) = 0; // serializacja
		virtual void readFromBuf(const BYTE_BUF &buf, uint offset = 0) = 0; // deserializacja
        
        void writeTLToBuf(BYTE_BUF &buf)
        {
            buf.push_back(tag);
            if (length < 128)
                buf.push_back(length);
            else //TODO
            {
                uint tmp_len = length;
                BYTE len_of_len = 1;
                while(tmp_len = (tmp_len >> 8))
                {
                    len_of_len++;
                }
                // zakladamy ze len_of_len < 128
                buf.push_back(len_of_len | 0x80);
                
                for(int i = 0; i < len_of_len; i++)
                {
                    buf.push_back((length & (0xFF << 8*(len_of_len - i - 1))) >> 8*(len_of_len - i - 1)); // nakladamy maske i dosuwamy do prawej aby otrzymac bajt
                }
                
            }
        }
        
        std::pair<int, int> readLength(const BYTE_BUF& buf, uint offset = 0) // zwraca {dlugosc, ile bajtow na dlugosc}
        {
            if (!(buf[offset+1] & 0x80))
            {
                return std::make_pair(buf[offset+1], 0);
            }
            else
            {
                uint len_of_len = buf[offset+1] - 0x80;
                uint length = 0;
                for(int i = 0; i < len_of_len; i++)
                {
                    length += buf[offset + (len_of_len - i + 1)] << 8*i;
                }
                return std::make_pair(length, len_of_len);
            }
        }
        void writeToFile(std::string filename)
        {
            BYTE_BUF buf;
            writeToBuf(buf);
            buf.writeToFile(filename);
        }
        void readFromFile(std::string filename)
        {
            BYTE_BUF buf;
            buf.readFromFile(filename);
            readFromBuf(buf);
        }
};


class ASN_INTEGER: public ASNobject
{
    protected:
        int value;
        void setLength()
        {
            int l = 1;
            int tmp_value = value;
            while(tmp_value  = tmp_value / 256)
            {
                l++;
            }
            
            if (value & (0x01 << 8*l - 1)) // jesli najstarszy bit to 1
            {
                if (value > 0) // i jesli dodatni, to dodatkowy bajt zer
                    l++;
            }
            else if (value < 0) // jesli najstarszy bit to 0 i ujemny to dodatkowy bajt potrzebny
            {
                l++;
            }
            
            length = l;
        }
        
    public:
        
		void writeToBuf(BYTE_BUF &buf);
        void readFromBuf(const BYTE_BUF &buf, uint offset = 0);
        
		ASN_INTEGER(): value(0) {tag = 0x02;}
        
        ASN_INTEGER(int value): value(value)
        {
            tag = 0x02;
            isSet = true;
            setLength();
        }
        
        
        operator int()
        {
            return value;
        }
        
        operator const int() const
        {
            int x = value;
            return x;
        }
        
        void operator=(int value)
        {
            isSet = true;
            this->value = value;
            setLength();
        }
};


class ASN_BITSTRING: public ASNobject
{
    private:
        void setLength()
        {
            length = (bits.size() / 8) + 1 + 1; // + 1 na unused
        }
        BIT_ARRAY bits;
    public:
        ASN_BITSTRING() {tag = 0x03;}
        ASN_BITSTRING(BIT_ARRAY &bits): bits(bits)
        {
            tag = 0x03;
            isSet = true;
            setLength();
        }
        
        void writeToBuf(BYTE_BUF &buf);
        void readFromBuf(const BYTE_BUF &buf, uint offset = 0);
        
        BIT operator[] (uint i)
        {
            if (isSet)
                return bits[i];
            else
                throw("Cannot access empty ASN_BITSTRING");
        }
        
        void operator= (const BIT_ARRAY& src_bits)
        {
            isSet = true;
            bits = src_bits;
            setLength();
        }
        
        bool operator== (const ASN_BITSTRING& rhs) const
        {
            return (bits == rhs.bits);
        }
};

class ASN_ENUMERATED: public ASN_INTEGER

{
    protected:
        std::unordered_map<std::string, int> value_dict;
        
        bool isInDict(int value)
        {
            for (auto it = value_dict.begin(); it != value_dict.end(); it++)
            {
                if (it->second == value)
                    return true;
            }
            return false;
        }
        
    public:
        void readFromBuf(const BYTE_BUF &buf, uint offset = 0);
        
        
        ASN_ENUMERATED()
        {
            tag = 0x02;
        }
        
        ASN_ENUMERATED(std::initializer_list<std::string> il)
        {
            tag = 0x02;
            int i = 0;
            for (auto it = il.begin(); it != il.end(); it++)
            {
                value_dict.insert({(*it), i});
                i++;
            }
        }
        ASN_ENUMERATED(std::initializer_list<std::pair<std::string, int>> il)
        {
            tag = 0x02;
            for (auto it = il.begin(); it != il.end(); it++)
            {
                value_dict.insert((*it));
            }
        }
        
        void operator=(const std::initializer_list<std::string> il)
        {
            value_dict.clear();
            int i = 0;
            for (auto it = il.begin(); it != il.end(); it++)
            {
                value_dict.insert({(*it), i});
                i++;
            }
        }
        void operator=(const std::initializer_list<std::pair<std::string, int>> il)
        {
            value_dict.clear();
            for (auto it = il.begin(); it != il.end(); it++)
            {
                value_dict.insert((*it));
            }
        }
        
        
        void operator=(int value)
        {
            if (isInDict(value))
            {
                isSet = true;
                this->value = value;
                setLength();
            }
            else
            {
                throw("ASN_ENUMERATED: tried to set to value not in value dict");
            }
        }
        
        void operator=(std::string str)
        {
            value = value_dict[str];
            setLength();
        }
        
        
        std::string toStr() const
        {
            for (auto it = value_dict.begin(); it != value_dict.end(); it++)
            {
                if (it->second == value)
                    return it->first;
            }
            throw("ASN_ENUMERATED: somehow, the internal value was not in the internal value dictionary.");
            return "";
        }
        operator std::string() const
        {
            return toStr();
        }
    
};

class ASN_UTF8STRING: public ASNobject // naiwna implementacja, nie sprawdza poprawnosci z UTF8
{
    private:
        void setLength()
        {
            length = utf8string.size();
        }
        std::string utf8string;
    public:
        void readFromBuf(const BYTE_BUF &buf, uint offset = 0);
        void writeToBuf(BYTE_BUF &buf);
        
        ASN_UTF8STRING() {tag = 0x0C;}
        ASN_UTF8STRING(const std::string &string): utf8string(string)
        {
            tag = 0x0C;
            isSet = true;
            setLength();
        }
        
        ASN_UTF8STRING(const char* utf8c)
        {
            tag = 0x0C;
            isSet = true;

            for (int i = 0; utf8c[i] != 0; i++)
            {
                utf8string.push_back(utf8c[i]);
            }
            setLength();
        }
        
        
        operator std::string ()
        {
            return utf8string;
        }
        
        
        const char& operator[] (uint i) const
        {
            if (i >= utf8string.size())
                throw ("ASN_UTF8STRING: array out of bounds");
            return utf8string[i];
        }
        
        bool operator==(const ASN_UTF8STRING& rhs) const
        {
            return (utf8string == rhs.utf8string);
        }
};

class ASN_SEQUENCE: public ASNobject
{
    protected:
        std::vector<ASNobject *> object_ptrs;
        void setLength()
        {
            int tmp = 0;
            for (auto it = object_ptrs.begin(); it != object_ptrs.end(); it++)
            {
                tmp += (*it)->fullSize();
            }
            length = tmp;
        }
    
    public:    
        ASN_SEQUENCE()
        {
            tag = 0x30;
        }
        
        void writeToBuf(BYTE_BUF &buf)
        {
            setLength();
            writeTLToBuf(buf);
            
            for (auto it = object_ptrs.begin(); it != object_ptrs.end(); it++)
            {
                (*it)->writeToBuf(buf);
            }
        }
        
        void readFromBuf(const BYTE_BUF &buf, uint offset = 0)
        {
            if (buf[offset] != tag)
                throw("Not an ASN sequence");
            std::pair<int,int> length_offset = readLength(buf,offset);
            length = length_offset.first;
            uint off_seq = length_offset.second;
            //length = buf[offset+1];
            
            int off = offset + 2 + off_seq;
            for (auto it = object_ptrs.begin(); it != object_ptrs.end(); it++)
            {
                if (off - (offset + 2) == length)
                    break;
                (*it)->readFromBuf(buf, off);
                off += (*it)->fullSize();
            }
        }
        
};
#endif
