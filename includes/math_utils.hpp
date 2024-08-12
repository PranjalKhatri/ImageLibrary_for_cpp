#ifndef MATH_UTILS_HPP
#define MATH_UTILS_HPP

#include <cmath>
#include<math.h>
#include <vector>
#include <memory>

#define PI 3.141592653589793

using vec_2d = std::vector<std::vector<int>>;
using smart_2d_ptr_int = std::shared_ptr<vec_2d>;

namespace Gaussian
{
    /// @brief Generates the gaussian matrix for a 2d field of given size
    /// @param _n the dimension of field (n*n)
    /// @param _std the standard deviation of the function
    /// @warning the maximum n supported is n =7
    std::pair<size_t, smart_2d_ptr_int> GaussianMatrix(int _n, float _std = 1.0f)
    {
        smart_2d_ptr_int _mat = std::make_shared<vec_2d>(_n, std::vector<int>(_n, 0));
        double _inv_std_sq = 1.0 / (2.0 * PI * _std * _std);
        double _sum = 0.0;
        // IMPORTANT: _scale factor to avoid truncation to 0 :|
        int _scale = 100; 
        //factor to center around the mid point
        int _half_n = _n / 2;

        for (int i = 0; i < _n; i++)
        {
            for (int j = 0; j < _n; j++)
            {
                int x = i - _half_n;
                int y = j - _half_n;
                double value = _inv_std_sq * exp(-(x * x + y * y) / (2.0 * _std * _std));
                (*_mat)[i][j] = static_cast<int>(value * _scale);
                _sum += (*_mat)[i][j];
            }
        }

        return std::make_pair(static_cast<size_t>(_sum), _mat);
    }
}; // namespace Gaussian

#endif
