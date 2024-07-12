#ifndef IMAGE_UTILITIES_HPP
#define IMAGE_UTILITIES_HPP

#include "utilities.hpp"
#include "bmp.hpp"

using rgb_data = std::vector<std::vector<RGBTRIPLE>>;

void rgb_to_grayscale(std::shared_ptr<rgb_data> &_dat)
{
    size_t _height = _dat->size();
    size_t _width = _dat->at(0).size();

    // rgb_data _cpy(*_dat);

    for (int _row = 0; _row < _height; _row++)
    {
        for (int _col = 0; _col < _width; _col++)
        {
            size_t _accumulator{};
            _accumulator += static_cast<size_t>((*_dat)[_row][_col].rgbtRed);
            _accumulator += static_cast<size_t>((*_dat)[_row][_col].rgbtGreen);
            _accumulator += static_cast<size_t>((*_dat)[_row][_col].rgbtBlue);
            _accumulator /= 3;
            //clamping
            _accumulator = _accumulator > 255 ? 255 : _accumulator;
            //storing
            (*_dat)[_row][_col].rgbtBlue = static_cast<BYTE>(_accumulator);
            (*_dat)[_row][_col].rgbtGreen = static_cast<BYTE>(_accumulator);
            (*_dat)[_row][_col].rgbtRed = static_cast<BYTE>(_accumulator);
        }
    }
}

void rgb_to_sepia(std::shared_ptr<rgb_data> &_dat)
{
    size_t _height = _dat->size();
    size_t _width = _dat->at(0).size();

    // rgb_data _cpy(*_dat);

    for (int _row = 0; _row < _height; _row++)
    {
        for (int _col = 0; _col < _width; _col++)
        {
            //Sepia formula
            size_t _outputRed = ((*_dat)[_row][_col].rgbtRed * .393f) + ((*_dat)[_row][_col].rgbtGreen * .769f) + ((*_dat)[_row][_col].rgbtBlue * .189f);
            size_t _outputGreen = ((*_dat)[_row][_col].rgbtRed * .349f) + ((*_dat)[_row][_col].rgbtGreen * .686f) + ((*_dat)[_row][_col].rgbtBlue * .168f);
            size_t _outputBlue = ((*_dat)[_row][_col].rgbtRed * .272f) + ((*_dat)[_row][_col].rgbtGreen * .534f) + ((*_dat)[_row][_col].rgbtBlue * .131f);
            //clamping
            _outputRed = _outputRed > 255 ? 255 : _outputRed;
            _outputGreen = _outputGreen > 255 ? 255 : _outputGreen;
            _outputBlue = _outputBlue > 255 ? 255 : _outputBlue;
            //storing
            (*_dat)[_row][_col].rgbtRed = static_cast<BYTE>(_outputRed);
            (*_dat)[_row][_col].rgbtGreen = static_cast<BYTE>(_outputGreen);
            (*_dat)[_row][_col].rgbtBlue = static_cast<BYTE>(_outputBlue);
        }
    }
}

#endif