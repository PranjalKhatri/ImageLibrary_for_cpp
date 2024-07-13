#ifndef UTILITIES_HPP
#define UTILITIES_HPP

#include <bit>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <bitset>
#include <map>

typedef unsigned char BYTE;  // 1
typedef unsigned short WORD; // 2
typedef unsigned long DWORD; // 4
typedef long LONG;

#pragma pack(push, 1) // Disable padding
typedef struct tagRGBTRIPLE
{
    BYTE rgbtBlue;
    BYTE rgbtGreen;
    BYTE rgbtRed;
} RGBTRIPLE, *PRGBTRIPLE, *NPRGBTRIPLE, *LPRGBTRIPLE;
#pragma pop(0) // enable padding

void displayCharBits(char c)
{
    std::bitset<8> bits(c);
    std::cout << bits << " ";
}

class bit_reader
{
    char *m_buffer;
    size_t m_bytes;
    size_t m_cur_bit = 7;
    size_t m_cur_byte{};
    size_t m_eof_bits{};

public:
    bit_reader(char *_buf, size_t _bytes, size_t _eof_bits = 0)
        : m_buffer(_buf), m_bytes(_bytes), m_eof_bits(_eof_bits) {}
    int get_next_bit()
    {
        if (m_cur_byte == m_bytes - 1 && m_cur_bit == m_eof_bits - 1)
        {
            return -1;
        }
        if (m_cur_bit == -1)
        {
            m_cur_bit = 7;
            m_cur_byte++;
        }
        if (m_cur_byte >= m_bytes)
        {
            std::cerr << "bit reader index out of range";
            return -1;
        }
        else
        {
            int res = ((m_buffer[m_cur_byte] >> m_cur_bit) & 1);
            m_cur_bit--;
            return res;
        }
    }
    int read_byte_compeletely()
    {
        size_t cnt{};
        while (m_cur_bit != -1)
        {
            cnt++;
            get_next_bit();
        }
        return cnt;
        // std::cout<<"read "<<cnt<<" padding bits";
    }

    void re_initialize(char *_buff, size_t _bytes, size_t _eof_bits = 0)
    {
        m_bytes = _bytes;
        m_buffer = _buff;
        m_cur_bit = 7;
        m_cur_byte = 0;
        m_eof_bits = _eof_bits;
    }
};

#endif