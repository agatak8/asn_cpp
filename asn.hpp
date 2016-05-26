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


/// \brief główna klasa ASN
///
/// Zawiera interfejs zapisu/odczytu do/z bufora bajtów BYTE_BUF,
///  wartości tag i length zawarte w każdym obiekcie ASN
///  i funkcje zapisu tychże.
class ASNobject: public IStorable
{
    protected:
        BYTE tag;
        uint length;
        /// flaga, czy obiekt ma ustawioną wartość
        bool isSet = false;
        
	public:
        /// zwraca całkowity rozmiar obiektu w bajtach po serializacji
        int fullSize()
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
        /// serializacja
		virtual void writeToBuf(BYTE_BUF &buf) = 0;
        /// deserializacja
		virtual void readFromBuf(const BYTE_BUF &buf, uint offset = 0) = 0;
        
        /// \brief metoda zapisu tagu i długości do bufora
        /// Najpierw zapisuje bajt tagu. 
        /// Jeśli długość < 128 to zapisuje ją na jednym bajcie,
        ///  w przeciwnym wypadku pierwszy bit pierwszego bajtu to 1
        ///  a pozostałe bity kodują na ilu bajtach zakodowana jest długość. 
        ///  Drugi i kolejne bajty kodują długość jako unsigned int.
        
        void writeTLToBuf(BYTE_BUF &buf)
        {
            buf.push_back(tag);
            if (length < 128)
                buf.push_back(length);
            else //TODO
            {
                uint tmp_len = length;
                BYTE len_of_len = 1; // na ilu bajtach zakodowana dlugosc
                while(tmp_len = (tmp_len >> 8))
                {
                    len_of_len++;
                }
                // zakladamy ze len_of_len < 128
                buf.push_back(len_of_len | 0x80);
                
                for(int i = 0; i < len_of_len; i++)
                {
                    buf.push_back((length & (0xFF << 8*(len_of_len - i - 1))) >> 8*(len_of_len - i - 1));
                    // maska poprzez AND z odp przesunietym 0xFF da dany bajt inta
                    //
                    // 0xFF = 1111 1111
                    // i = 0 -> przesuwamy 0xFF do najstarszego bajtu
                    // i = len_of_len - 1 -> 0xFF zostaje, to najmlodszy bajt
                    // 
                    // length & przesuniete 0xFF -> dany bajt inta ktory jeszcze trzeba przesunac na najmlodszy bajt
                }
                
            }
        }
        
        /// \brief metoda odczytująca długość zakodowaną jako 1 lub więcej bajtów z bufora bajtów
        ///
        /// Zwraca parę intów (a,b) gdzie a - odczytana długość b - ile bajtów zajęła długość - 1
        std::pair<int, int> readLength(const BYTE_BUF& buf, uint offset = 0)
        {
            // jednobajtowa
            if (!(buf[offset+1] & 0x80))
            {
                return std::make_pair(buf[offset+1], 0);
            }
            // wielobajtowa
            else
            {
                uint len_of_len = buf[offset+1] - 0x80; // usuwamy najstarszy bit ktory jest 1
                uint length = 0;
                for(int i = 0; i < len_of_len; i++)
                {
                    length += buf[offset + (len_of_len - i + 1)] << 8*i; // odczytujemy wartosc z bajtow
                }
                return std::make_pair(length, len_of_len);
            }
        }
        
        /// zapis do pliku
        void writeToFile(std::string filename)
        {
            BYTE_BUF buf;
            writeToBuf(buf);
            buf.writeToFile(filename);
        }
        
        /// odczyt z pliku
        void readFromFile(std::string filename)
        {
            BYTE_BUF buf;
            buf.readFromFile(filename);
            readFromBuf(buf);
        }
};

/// \brief klasa ASN_INTEGERA
///
/// tag: 0x02
///
/// typ przechowujący wartość: int value
///
/// kodowanie: [tag][długość (1 lub więcej bajtów)][wartość jako U2 ze znakiem]
///  gdzie wartość zawiera się w minimalnej ilości bajtów
class ASN_INTEGER: public ASNobject
{
    protected:
        /// wartość
        int value;
        /// metoda pomocnicza wywoływana przy zmianie wartości (przez użytkownika lub z odczytu z bufora/pliku)
        ///  w celu zaaktualizowania pola length
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


/// \brief Klasa ASN_BITSTRING
///
/// tag: 0x03
///
/// typ przechowujący zawartość: BIT_ARRAY
///
/// kodowanie: [tag][długość(1 lub więcej bajtów)][nieżywane bity][bajty ciągu binarnego]
///  gdzie [nieużywane bity] określa ile bitów na końcu ostatniego bajtu nie wchodzi do ciągu
class ASN_BITSTRING: public ASNobject
{
    private:
        /// metoda pomocnicza wywoływana przy zmianie wartości (przez użytkownika lub z odczytu z bufora/pliku)
        ///  w celu zaaktualizowania pola length
        void setLength()
        {
            length = (bits.size() / 8) + 1 + 1; // + 1 na unused
        }
        /// zawartość
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

/// \brief Klasa ASN_ENUMERATED
///
/// tag: 0x02
///
/// typ przechowujący zawartość: int
///
/// kodowanie: jak ASN_INTEGER
///  z tą różnicą, że może przyjąć tylko określone wartości
class ASN_ENUMERATED: public ASN_INTEGER

{
    protected:
        /// mapa dozwolonych wartości i ich nazw
        std::unordered_map<std::string, int> value_dict;
        
        /// metoda określająca czy dana wartość jest dozwolona
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
        // tak jak INTEGER tyle ze sprawdza czy wartosc jest w mapie
        void readFromBuf(const BYTE_BUF &buf, uint offset = 0);
        
        
        ASN_ENUMERATED()
        {
            tag = 0x02;
        }

        
        /// \brief przypisanie nazw wartości poprzez listę stringów
        ///
        /// przypisuje nazwom domyślne wartości 0,1,2,... w kolejności ich wystąpienia
        void operator=(const std::initializer_list<std::string> il)
        {
            int i = 0;
            for (auto it = il.begin(); it != il.end(); it++)
            {
                value_dict.insert({(*it), i});
                i++;
            }
        }
        
        /// \brief przypisanie nazw wartości i ich wartości przez mapę stringów i intów
        ///
        /// Określa dokładnie jakie wartości mają dane nazwy
        void operator=(const std::unordered_map<std::string, int> &map)
        {
            value_dict = map;
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
        
        /// konwersja do nazwy tekstowej określającej wartość
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

/// \brief Klasa ASN_UTR8STRING
///
/// tag: 0x0C
///
/// typ przechowujący zawartość: std::string
///
/// kodowanie: [tag][długość][ciąg charów]
class ASN_UTF8STRING: public ASNobject
{
    private:
        /// metoda pomocnicza wywoływana przy zmianie wartości (przez użytkownika lub z odczytu z bufora/pliku)
        ///  w celu zaaktualizowania pola length
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

/// \brief Klasa ASN_SEQUENCE
///
/// tag: 0x30
///
/// typ przechowujący wartość: std::vector<ASNobject *>
/// 
/// kodowanie: [tag][długość][obiekty zawierające się]
class ASN_SEQUENCE: public ASNobject
{
    protected:
        std::vector<ASNobject *> object_ptrs; // wskaźniki na obiekty ASN w sekwencji
        /// metoda pomocnicza wywoływana przy zmianie wartości (przez użytkownika lub z odczytu z bufora/pliku)
        ///  w celu zaaktualizowania pola length
        void setLength()
        {
            int tmp = 0;
            for (auto it = object_ptrs.begin(); it != object_ptrs.end(); it++)
            {
                tmp += (*it)->fullSize(); // pelne dlugosci obiektow
            }
            length = tmp;
        }
    
    public:    
        ASN_SEQUENCE()
        {
            tag = 0x30;
        }
        void writeToBuf(BYTE_BUF &buf);
        void readFromBuf(const BYTE_BUF &buf, uint offset = 0);
        
};
#endif
