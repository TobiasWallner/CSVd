#include <csvd/csvd.hpp>

#include <sstream>
#include <ranges>

// google test
#include <gtest/gtest.h>

TEST(csvd, read_correct_file){
    std::stringstream file;
    file << 
    "\"Frequencies (Hz)\", Magnitudes ('dB'), Phases (deg), Real, Imag\n"
    "0.0159155, 0.0432094, -5.76789, 0.999899, -0.101\n"
    "0.01629, 0.0452557, -5.90549, 0.999889, -0.103425\n"
    "0.0166733, 0.0473984, -6.04646, 0.999878, -0.105911\n"
    "0.0170657, 0.0496418, -6.19089, 0.999866, -0.108459\n " << std::endl;

    // test if the csv can be read
    tl::expected<csvd::CSVd, csvd::ReadError> csv = csvd::read(file);
    ASSERT_TRUE(csv.has_value());

    // test if csv has content
    ASSERT_FALSE(csv.value().empty());

    // test if names have been read correctly
    std::vector<std::string_view> expected_names{"Frequencies (Hz)", "Magnitudes ('dB')", "Phases (deg)", "Real", "Imag"};
    for(const auto& [column, name] : std::views::zip(csv.value(), expected_names)){
        ASSERT_EQ(column.name, name);
    }
    
    // test if csv has read values correctly
    const csvd::Column& col = csv.value()[0]; // get first column
    ASSERT_EQ(col.data.size(), 4);
    ASSERT_NEAR(col.data.front(), 0.0159155, 0.00001);
    ASSERT_NEAR(col.data.back(), 0.0170657, 0.00001);
}
