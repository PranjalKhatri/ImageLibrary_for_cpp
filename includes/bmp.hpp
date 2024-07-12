#ifndef BMP_HPP
#define BMP_HPP

#include "utilities.hpp"

#pragma region file_headers
// Swap between little and small endian

template <class T>
void endswap(T *objp)
{
    unsigned char *memp = reinterpret_cast<unsigned char *>(objp);
    std::reverse(memp, memp + sizeof(T));
}

#pragma pack(push, 1) // Disable padding
// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapfileheader
typedef struct tagBITMAPFILEHEADER
{
    WORD bfType; // MB
    DWORD bfSize;
    WORD bfReserved1; // 0000
    WORD bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *LPBITMAPFILEHEADER, *PBITMAPFILEHEADER;
#pragma pop(0) // enable padding

#pragma pack(push, 1) // Disable padding
// https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
typedef struct tagBITMAPINFOHEADER
{
    DWORD biSize; // 40
    LONG biWidth;
    LONG biHeight;
    WORD biPlanes;
    WORD biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG biXPelsPerMeter;
    LONG biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;
#pragma pop(0) // enable padding

#pragma pack(push, 1) // Disable padding
typedef struct tagRGBQUAD
{
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} RGBQUAD;
#pragma pop(0) // enable padding

#pragma pack(push, 1) // Disable padding
typedef struct tagRGBTRIPLE
{
    BYTE rgbtBlue;
    BYTE rgbtGreen;
    BYTE rgbtRed;
} RGBTRIPLE, *PRGBTRIPLE, *NPRGBTRIPLE, *LPRGBTRIPLE;
#pragma pop(0) // enable padding

#pragma end_region

// TODO: Implement support for different bmp versions
class bmp_reader
{
private:
    size_t m_file_size;
    std::string m_file_name;
    std::ifstream m_in_file;
    BITMAPFILEHEADER m_bfh;
    BITMAPINFOHEADER m_bih;
#if __cplusplus >= 202002L
    bool m_is_little_endian;
#endif
    size_t m_width, m_height;
    std::shared_ptr<std::vector<std::vector<RGBTRIPLE>>> pixel_data;
    char padding[4];

public:
    bmp_reader(std::string file_name) : m_file_name(file_name), pixel_data(std::make_shared<std::vector<std::vector<RGBTRIPLE>>>())
    {
#if __cplusplus >= 202002L
        if constexpr (std::endian::native == std::endian::little)
            m_is_little_endian = true;
        else
            m_is_little_endian = false;
#endif
        m_in_file.open(file_name, std::ios_base::binary);
        if (!m_in_file.is_open())
        {
            std::cerr << "No such file exists";
            return;
        }

        /*if (is_little_endian)
        {
            endswap(&bfh.bfType);
            endswap(&bfh.bfSize);
            endswap(&bfh.bfReserved2);
            endswap(&bfh.bfOffBits);
        }*/
        read_file_header();
        read_info_header();
        (*pixel_data).resize(m_height);
        for (auto &i : (*pixel_data))
        {
            i.resize(m_width);
        }
        read_pixel_data();
    }
    void read_file_header()
    {
        m_in_file.read(reinterpret_cast<char *>(&m_bfh), sizeof(BITMAPFILEHEADER));
        if (!check_bmp_header())
        {
            std::cerr << "Not a bitmap file!";
            m_in_file.close();
            return;
        }
        m_file_size = m_bfh.bfSize;
    }
    void read_info_header()
    {
        m_in_file.read(reinterpret_cast<char *>(&m_bih), sizeof(BITMAPINFOHEADER));
        m_width = m_bih.biWidth;
        m_height = m_bih.biHeight;
    }

    void read_pixel_data()
    {
        // Calculate the number of padding bytes per row
        size_t padding_width = (4 - (m_width * sizeof(RGBTRIPLE)) % 4) % 4;
        std::clog << "Padding width: " << padding_width << " bytes\n";

        // Iterate over each row of the image
        for (int i = 0; i < m_height; ++i)
        {
            for (int j = 0; j < m_width; ++j)
            {
                if (!m_in_file.read(reinterpret_cast<char *>(&pixel_data->at(i)[j]), sizeof(RGBTRIPLE)))
                {
                    std::cerr << "Error reading pixel data at row " << i << ", column " << j << "\n";
                    return;
                }
                // else{
                //     std::clog<<"Readed row "<<i<<" col "<<j<<std::endl;
                // }
            }
            // Skip the padding bytes at the end of the row
            m_in_file.ignore(padding_width);
        }

        // Check if we reached the end of the file
        if (m_in_file.peek() == EOF)
        {
            std::clog << "Reached the end of the file\n";
        }
        else
        {
            std::streampos cpos = m_in_file.tellg();
            m_in_file.seekg(0, std::ios::end);
            std::streampos epos = m_in_file.tellg();
            std::clog << "Did not reach the end of the file\n reamianing chars " << epos - cpos;
        }
    }

    void write_to_file(std::string output_file_name, std::shared_ptr<std::vector<std::vector<RGBTRIPLE>>> &dat)
    {
        std::ofstream out_file(output_file_name, std::ios_base::binary);
        if (!out_file.is_open())
        {
            std::cerr << "Can't open the output file!";
            return;
        }
        out_file.write(reinterpret_cast<char *>(&m_bfh), sizeof(BITMAPFILEHEADER));
        out_file.write(reinterpret_cast<char *>(&m_bih), sizeof(BITMAPINFOHEADER));
        size_t padding_width = (4 - (m_width * sizeof(RGBTRIPLE)) % 4) % 4;
        for (int i = 0; i < m_height; i++)
        {
            for (int j = 0; j < m_width; j++)
            {
                out_file.write(reinterpret_cast<char *>(&((*dat)[i][j])), sizeof(RGBTRIPLE));
            }
            out_file.write(0x00, padding_width);
        }
        out_file.close();
    }

    std::shared_ptr<std::vector<std::vector<RGBTRIPLE>>> get_pixel_data()
    {
        return pixel_data;
    }
    size_t getsize() const { return m_file_size; }
    bool check_bmp_header() const
    {
        if (m_bfh.bfType != 0x4d42 /*BM*/) /*little endian*/
            return false;
        return true;
    }
};

#endif