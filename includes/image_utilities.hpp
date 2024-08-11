#ifndef IMAGE_UTILITIES_HPP
#define IMAGE_UTILITIES_HPP

#include "utilities.hpp"
#include "bmp.hpp"
#include <math.h>
#include <cmath>
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
            // clamping
            _accumulator = _accumulator > 255 ? 255 : _accumulator;
            // storing
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
            // Sepia formula
            size_t _outputRed = ((*_dat)[_row][_col].rgbtRed * .393f) + ((*_dat)[_row][_col].rgbtGreen * .769f) + ((*_dat)[_row][_col].rgbtBlue * .189f);
            size_t _outputGreen = ((*_dat)[_row][_col].rgbtRed * .349f) + ((*_dat)[_row][_col].rgbtGreen * .686f) + ((*_dat)[_row][_col].rgbtBlue * .168f);
            size_t _outputBlue = ((*_dat)[_row][_col].rgbtRed * .272f) + ((*_dat)[_row][_col].rgbtGreen * .534f) + ((*_dat)[_row][_col].rgbtBlue * .131f);
            // clamping
            _outputRed = _outputRed > 255 ? 255 : _outputRed;
            _outputGreen = _outputGreen > 255 ? 255 : _outputGreen;
            _outputBlue = _outputBlue > 255 ? 255 : _outputBlue;
            // storing
            (*_dat)[_row][_col].rgbtRed = static_cast<BYTE>(_outputRed);
            (*_dat)[_row][_col].rgbtGreen = static_cast<BYTE>(_outputGreen);
            (*_dat)[_row][_col].rgbtBlue = static_cast<BYTE>(_outputBlue);
        }
    }
}

void edge_detection(std::shared_ptr<rgb_data> &_dat)
{
    size_t _height = _dat->size();
    size_t _width = _dat->at(0).size();

    std::vector<std::vector<int>> _xmat = {{0, 0, 0}, {-2, 0, 2}, {-1, 0, 1}};
    std::vector<std::vector<int>> _ymat = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

    // Create a temporary vector to store the edge detection results
    rgb_data temp(_height, std::vector<RGBTRIPLE>(_width));

    for (int _row = 0; _row < _height; _row++)
    {
        for (int _col = 0; _col < _width; _col++)
        {
            int gx_blue = 0, gx_red = 0, gx_green = 0;
            int gy_blue = 0, gy_red = 0, gy_green = 0;

            for (int _r = -1; _r <= 1; _r++)
            {
                for (int _c = -1; _c <= 1; _c++)
                {
                    int _row_offset = _row + _r;
                    int _col_offset = _col + _c;

                    if (_row_offset >= 0 && _col_offset >= 0 && _row_offset < _height && _col_offset < _width)
                    {
                        gx_blue += _xmat[_r + 1][_c + 1] * (*_dat)[_row_offset][_col_offset].rgbtBlue;
                        gx_red += _xmat[_r + 1][_c + 1] * (*_dat)[_row_offset][_col_offset].rgbtRed;
                        gx_green += _xmat[_r + 1][_c + 1] * (*_dat)[_row_offset][_col_offset].rgbtGreen;

                        gy_blue += _ymat[_r + 1][_c + 1] * (*_dat)[_row_offset][_col_offset].rgbtBlue;
                        gy_red += _ymat[_r + 1][_c + 1] * (*_dat)[_row_offset][_col_offset].rgbtRed;
                        gy_green += _ymat[_r + 1][_c + 1] * (*_dat)[_row_offset][_col_offset].rgbtGreen;
                    }
                }
            }

            // Calculate the magnitude of the gradient and clamp to [0, 255]
            int blue_magnitude = std::min(static_cast<int>(std::sqrt(gx_blue * gx_blue + gy_blue * gy_blue)), 255);
            int red_magnitude = std::min(static_cast<int>(std::sqrt(gx_red * gx_red + gy_red * gy_red)), 255);
            int green_magnitude = std::min(static_cast<int>(std::sqrt(gx_green * gx_green + gy_green * gy_green)), 255);

            // std::cout<<blue_magnitude<<" "<<red_magnitude<<" "<<green_magnitude<<" ";
            // Store the result in the temporary vector
            temp[_row][_col].rgbtBlue = blue_magnitude;
            temp[_row][_col].rgbtRed = red_magnitude;
            temp[_row][_col].rgbtGreen = green_magnitude;
        }
    }

    // Copy the results from the temporary vector to _dat
    for (int _i = 0; _i < _height; _i++)
    {
        for (int _j = 0; _j < _width; _j++)
        {
            (*_dat)[_i][_j].rgbtBlue = temp[_i][_j].rgbtBlue;
            (*_dat)[_i][_j].rgbtRed = temp[_i][_j].rgbtRed;
            (*_dat)[_i][_j].rgbtGreen = temp[_i][_j].rgbtGreen;
        }
    }
}

void invert_colours(std::shared_ptr<rgb_data> &_dat)
{
        size_t _height = _dat->size();
    size_t _width = _dat->at(0).size();

    for (int _row = 0; _row < _height; _row++)
    {
        for (int _col = 0; _col < _width; _col++)
        {
            (*_dat)[_row][_col].rgbtBlue = 255-(*_dat)[_row][_col].rgbtBlue;
            (*_dat)[_row][_col].rgbtRed = 255-(*_dat)[_row][_col].rgbtRed;
            (*_dat)[_row][_col].rgbtGreen = 255-(*_dat)[_row][_col].rgbtGreen;
        }
        
    }
    
}

#endif