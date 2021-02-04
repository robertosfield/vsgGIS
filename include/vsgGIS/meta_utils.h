#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2021 Robert Osfield

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsgGIS/Export.h>

#include <sstream>

namespace vsgGIS
{

    template<typename T>
    struct in_brackets
    {
        in_brackets(T& v) : value(v) {}
        T& value;
    };

    template<typename T>
    std::istream& operator>>(std::istream& input, in_brackets<T> field)
    {
        while (input.peek() == ' ') input.get();

        std::string str;
        if (input.peek() == '(')
        {
            input.ignore();

            input >> field.value;

            if constexpr (std::is_same_v<T, std::string>)
            {
                if (!field.value.empty() && field.value[field.value.size()-1] == ')')
                {
                    field.value.erase(field.value.size()-1);
                    return input;
                }
                else
                {
                    while (input.peek() != ')')
                    {
                        int c = input.get();
                        if (input.eof()) return input;

                        field.value.push_back(c);
                    }
                }
            }

            if (input.peek() == ')')
            {
                input.ignore();
            }
        }
        else
        {
            input >> field.value;
        }

        return input;
    }

    /// helper class for reading decimal degree in the form of (degress) (minutes) (seconds) as used wiht EXIF_GPSLatitude and EXIF_GPSLongitude tags.
    /// usage:
    ///    std::stringstream str(EXIF_GPSLatitude_String);
    ///    double latitude;
    ///    str >> dms_in_brackets(latitude);
    struct dms_in_brackets
    {
        dms_in_brackets(double& angle) : value(angle) {}
        double& value;
    };

    std::istream& operator>>(std::istream& input, dms_in_brackets field)
    {
        double degrees = 0.0, minutes = 0.0, seconds = 0.0;
        input >> in_brackets(degrees) >> in_brackets(minutes) >> in_brackets(seconds);
        field.value = degrees + (minutes + seconds/60.0)/60.0;
        return input;
    }

} // namespace vsgGIS
