# CSVd

A small, **intentionally simple** CSV reader and writer for tables containing **numeric (`double`) values only**.

CSVd is designed for scientific, engineering, and data-processing workflows where CSV files are:
- purely numeric
- small to medium sized
- human-readable

It intentionally avoids the complexity of fully RFC-compliant CSV parsers.

---

## Features

- Reads and writes CSV files containing `double` values
- Optional header row with column names
- Automatic header detection (heuristic)
- Customizable:
  - value separators (multiple allowed)
  - line separators (multiple allowed)
  - quote characters (multiple allowed)
- Clear, detailed error reporting (row, column, offending cell)
- Stream-based API (`std::istream` / `std::ostream`)

---

## Non-Goals / Limitations

CSVd is **not** a fully-featured CSV implementation.

It deliberately does **not** support:
- escape characters (e.g. `\\`)
- escaped quotes
- string or mixed-type columns
- missing values
- RFC 4180 edge cases

Quote characters are treated as **structural delimiters only**, not data.

If you need full CSV compliance, use a dedicated CSV library instead.

---

## Supported Input Formats

With header:

```csv
Time, Value 1, Value 2
1, 0.1, 0.1
2, 0.2, 0.3
3, 0.3, 0.2
4, 0.4, 0.1
```

```csv
"Time", "Value 1", "Value 2"
1, 0.1, 0.1
2, 0.2, 0.3
3, 0.3, 0.2
4, 0.4, 0.1
```

Without header:

```csv
1, 0.1, 0.1 
2, 0.2, 0.3
3, 0.3, 0.2
4, 0.4, 0.1
```

---

## Get the Library

### Using CMake and CPM.cmake

Automatically download and use the Library using CPM in CMake.

```cmake
include(cmake/cpm.cmake)

CPMAddPackage("gh:TobiasWallner/CSVd#v2.0.1")

add_executable(my_app main.cpp)

target_link_libraries(my_app PRIVATE csvd)
```

### Using CMake as a Subdirectory

If CSVd is included directly in your repository (e.g. as a git submodule):

```cmake
add_subdirectory(external/csvd)

add_executable(my_app main.cpp)

target_link_libraries(my_app PRIVATE csvd)
```

### Using CMake FetchContent

CMake itself offers a way to automatically download and use libraries.

```cmake
include(FetchContent)

FetchContent_Declare(
	csvd
	GIT_REPOSITORY https://github.com/TobiasWallner/CSVd.git
	GIT_TAG v2.0.1
)

FetchContent_MakeAvailable(csvd)

add_executable(my_app main.cpp)

target_link_libraries(my_app PRIVATE csvd)
```
---

## Basic Usage

Include the header:

```cpp
#include <csvd/csvd.hpp>
```

### Reading a CSV file

CSVd works with **any input stream**, not just files.

```cpp
std::ifstream file("data.csv");
if(!file.is_open()){
    // handle error
}

csvd::CSVd csv;
std::expected<void, std::string> result = csv.read(file);

if(!result){
    std::cerr << result.error() << std::endl;
}
```

---

### Accessing Columns

Each column contains:
- a name (empty if no header)
- a vector of `double` values

```cpp
for(const csvd::Column& col : csv){
    std::cout << col.name << " (" << col.data.size() << ")" << std::endl;
}
```

Access by index:

```cpp
const csvd::Column& col = csv[0];
```

Find by name:

```cpp
auto it = csv.find("Time");
if(it != csv.end()){
    std::cout << it->data[0] << std::endl;
}
```

---

### Modifying Columns

CSVd exposes a vector-like interface:

```cpp
csv.erase(csv.find("Unused Column"));
csv.push_back({"New Column", {1.0, 2.0, 3.0}});
```

---

### Writing a CSV file

```cpp
std::ofstream out("output.csv");
csv.write(out);
```

Notes:
- Only as many rows as the **shortest column** are written
- Headers are written automatically if any column has a name. Empty names are replaced by quoted column numbers.

---

## Settings

```cpp
csvd::Settings settings;
settings.value_separator = ",;"; 	// Note: Undefined behaviour if empty
settings.line_separator  = "\n"; 	// Note: Undefined behaviour if empty
settings.quotes          = "\"'"; 	// Note: Undefined behaviour if empty
settings.header_type     = csvd::HeaderType::Auto;

csvd::CSVd csv(settings);
```

### Header Detection

When `HeaderType::Auto` is used:
- The first non-whitespace character is inspected
- If it is a digit, `+`, or `-` → first row is assumed to be data
- Otherwise → first row is assumed to be a header

This is a heuristic and may not work for all inputs.

---

## Error Handling

All parsing errors return:

```cpp
std::expected<void, std::string>
```

Error messages include:
- row number
- column number
- offending cell content
- a description

