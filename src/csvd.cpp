#include <algorithm>
#include <string>
#include <sstream>
#include <expected>
#include <charconv>
#include <limits>
#include <csvd/csvd.hpp>

namespace csvd{

    static std::string_view char_symbol(char c) {
        switch (static_cast<unsigned char>(c)) {

            // Control characters
            case '\0': return "\\0";   // NUL
            case '\x01': return "SOH";
            case '\x02': return "STX";
            case '\x03': return "ETX";
            case '\x04': return "EOT";
            case '\x05': return "ENQ";
            case '\x06': return "ACK";
            case '\a':   return "\\a"; // BEL
            case '\b':   return "\\b"; // BS
            case '\t':   return "\\t"; // TAB
            case '\n':   return "\\n"; // LF
            case '\v':   return "\\v"; // VT
            case '\f':   return "\\f"; // FF
            case '\r':   return "\\r"; // CR
            case '\x0E': return "SO";
            case '\x0F': return "SI";

            case '\x10': return "DLE";
            case '\x11': return "DC1";
            case '\x12': return "DC2";
            case '\x13': return "DC3";
            case '\x14': return "DC4";
            case '\x15': return "NAK";
            case '\x16': return "SYN";
            case '\x17': return "ETB";
            case '\x18': return "CAN";
            case '\x19': return "EM";
            case '\x1A': return "SUB";
            case '\x1B': return "\\e"; // ESC
            case '\x1C': return "FS";
            case '\x1D': return "GS";
            case '\x1E': return "RS";
            case '\x1F': return "US";

            // Printable characters
            case ' ':  return " ";
            case '!':  return "!";
            case '"':  return "\"";
            case '#':  return "#";
            case '$':  return "$";
            case '%':  return "%";
            case '&':  return "&";
            case '\'': return "'";
            case '(':  return "(";
            case ')':  return ")";
            case '*':  return "*";
            case '+':  return "+";
            case ',':  return ",";
            case '-':  return "-";
            case '.':  return ".";
            case '/':  return "/";

            case '0': return "0";
            case '1': return "1";
            case '2': return "2";
            case '3': return "3";
            case '4': return "4";
            case '5': return "5";
            case '6': return "6";
            case '7': return "7";
            case '8': return "8";
            case '9': return "9";

            case ':': return ":";
            case ';': return ";";
            case '<': return "<";
            case '=': return "=";
            case '>': return ">";
            case '?': return "?";
            case '@': return "@";

            case 'A': return "A";
            case 'B': return "B";
            case 'C': return "C";
            case 'D': return "D";
            case 'E': return "E";
            case 'F': return "F";
            case 'G': return "G";
            case 'H': return "H";
            case 'I': return "I";
            case 'J': return "J";
            case 'K': return "K";
            case 'L': return "L";
            case 'M': return "M";
            case 'N': return "N";
            case 'O': return "O";
            case 'P': return "P";
            case 'Q': return "Q";
            case 'R': return "R";
            case 'S': return "S";
            case 'T': return "T";
            case 'U': return "U";
            case 'V': return "V";
            case 'W': return "W";
            case 'X': return "X";
            case 'Y': return "Y";
            case 'Z': return "Z";

            case '[':  return "[";
            case '\\': return "\\\\";
            case ']':  return "]";
            case '^':  return "^";
            case '_':  return "_";
            case '`':  return "`";

            case 'a': return "a";
            case 'b': return "b";
            case 'c': return "c";
            case 'd': return "d";
            case 'e': return "e";
            case 'f': return "f";
            case 'g': return "g";
            case 'h': return "h";
            case 'i': return "i";
            case 'j': return "j";
            case 'k': return "k";
            case 'l': return "l";
            case 'm': return "m";
            case 'n': return "n";
            case 'o': return "o";
            case 'p': return "p";
            case 'q': return "q";
            case 'r': return "r";
            case 's': return "s";
            case 't': return "t";
            case 'u': return "u";
            case 'v': return "v";
            case 'w': return "w";
            case 'x': return "x";
            case 'y': return "y";
            case 'z': return "z";

            case '{': return "{";
            case '|': return "|";
            case '}': return "}";
            case '~': return "~";

            // DEL
            case '\x7F': return "DEL";

            default: return "N/A";
        }
    }

    /**
     * @brief Reads characters from the string until a delimiter has been found
     * 
     * Will not read the delimiter from the stream. So the next character in the stream
     * will be the delimiter or the end of the stream.
     * 
     * @param stream A reference to the stream object
     * @return A string containing all characters until the delimiter
     */
    std::string CSVd::read_cell(std::istream& stream){
        std::string result;
        bool is_in_quote = false;
        while(stream.eof() == false){
            bool has_quote = std::ranges::contains(this->settings_.quotes, stream.peek());

            // toggle being in quotes
            if(is_in_quote){
                if(has_quote){
                    is_in_quote = false;
                }
            }else{
                if(has_quote){
                    is_in_quote = true;
                }
            }

            
            bool has_value_separator = std::ranges::contains(this->settings_.value_separator, stream.peek());
            bool has_line_separator = std::ranges::contains(this->settings_.line_separator, stream.peek());
            
            // ignore value separator if in quotes
            if(is_in_quote){
                has_value_separator = false;
            }

            // break on separators
            if(has_value_separator || has_line_separator){
                break;
            }

            //Append characters
            result += stream.get();
        }
        return result;
    }

    static void skip(std::istream& stream, std::string_view skip){
        while((stream.eof() == false) && (stream.bad() == false) && (std::ranges::contains(skip, stream.peek()))){
            (void)stream.ignore(); // consume
        }
    }

    static void skip_whitespaces(std::istream& stream){
        const std::string_view whitespaces(" \a\b\t\n\v\f\r");
        skip(stream, whitespaces);
    }

    [[nodiscard]] static std::string_view trim(std::string_view string, std::string_view trim){
        const std::string::size_type pos1 = string.find_first_not_of(trim);
        const std::string::size_type pos2 = string.find_last_not_of(trim);
        const std::string::size_type count = pos2 - pos1 + 1;
        
        if(pos1 == std::string_view::npos){
            return std::string_view("", 0);
        }else if(pos2 == std::string::npos){
            return string.substr(pos1);
        }else{
            return string.substr(pos1, count);
        }
    }

    [[nodiscard]] static std::string_view trim_whitespaces(std::string_view string){
        const std::string_view whitespaces(" \a\b\t\n\v\f\r");
        return trim(string, whitespaces);
    }

    void CSVd::set_header_type(HeaderType header) {
        this->settings_.header_type = header;
    }
            
    HeaderType CSVd::header_type() const {
        return this->settings_.header_type;
    }

    void CSVd::set_value_separator(std::string_view separator){
        if(separator.empty() == false){
            this->settings_.value_separator = separator;
        }
    }

    std::string_view CSVd::value_separator() const {
        return this->settings_.value_separator;
    }

    void CSVd::set_line_separator(std::string_view separator){
        if(separator.empty() == false){
            this->settings_.line_separator = separator;
        }
    }

    std::string_view CSVd::line_separator() const {
        return this->settings_.line_separator;
    }

    CSVd::iterator CSVd::find(std::string_view name){
        auto itr = this->begin();
        for(; itr != this->end(); ++itr){
            if(itr->name == name){
                return itr;
            }
        }
        return this->end();
    }

    CSVd::const_iterator CSVd::find(std::string_view name) const {
        auto itr = this->begin();
        for(; itr != this->end(); ++itr){
            if(itr->name == name){
                return itr;
            }
        }
        return this->end();
    }

    std::expected<void, std::string> CSVd::read(std::istream& stream){
        if(stream.eof()){
            std::string error_message("Error parsing csv. Stream already at EOF\n");
            return std::unexpected(std::move(error_message));
        }
        if(stream.bad()){
            std::string error_message("Error parsing csv. Passed a bad stream\n");
            return std::unexpected(std::move(error_message));
        }
        
        // read data
        int column = 0;
        int row = 0;

        this->clear();
        
        HeaderType header_type = this->settings_.header_type;

        if(header_type == HeaderType::Auto) {
            skip_whitespaces(stream);
            if(std::isdigit(stream.peek()) || stream.peek() == '+' || stream.peek() == '-'){
                // detected numeric data in the first row --> no header
                header_type = HeaderType::None;
            }else{
                // detected non-numeric string in the first row --> has header
                header_type = HeaderType::FirstRow;
            }
        }

        // read header
        if(header_type == HeaderType::FirstRow){
            std::expected<void, std::string> result = this->read_with_header(stream);
            if(result.has_value() == false){
                return result;
            }
            ++row;
        }else{
            std::expected<void, std::string> result = this->read_without_header(stream);
            if(result.has_value() == false){
                return result;
            }
            ++row;
        }

        // read data
        while((stream.eof() == false)){

            if(stream.bad()){
                std::stringstream error_message;
                error_message 
                    << "Error parsing csv\n"
                    << "  column: " << (column+1) << "\n"
                    << "  row: " << (row+1) << "\n"
                    << "  message: Bad stream";
                return std::unexpected(std::move(error_message).str());
            }
            
            skip_whitespaces(stream);

            // check for eof after new line
            if(stream.eof()){
                if(column == 0){
                    // eof after new colum --> probably last empty line --> ok
                    break;
                }else{
                    std::stringstream error_message;
                    error_message 
                        << "Error parsing csv\n"
                        << "  column: " << (column+1) << "\n"
                        << "  row: " << (row+1) << "\n"
                        << "  message: End of line (EOF) reached too early, expected a cell entry.";
                    return std::unexpected(std::move(error_message).str());
                }
            }

            std::string cell = read_cell(stream);
            cell = trim_whitespaces(cell);
            
            double value = 0;
            {
                const std::from_chars_result result = std::from_chars(cell.data(), cell.data() + cell.size(), value);
                if(result.ec != std::errc{}){
                    std::stringstream error_message;
                    error_message 
                        << "Error parsing csv\n"
                        << "  column: " << (column + 1) << "\n"
                        << "  row: " << (row + 1) << "\n"
                        << "  cell: " << cell << "\n"
                        << "  message: Cannot convert cell to floating-point number\n";
                    return std::unexpected(std::move(error_message).str());
                }
            }

            if(column < this->size()){
                this->at(column).data.emplace_back(value);
            }else{
                std::stringstream error_message;
                error_message 
                    << "Error parsing csv\n"
                    << "  column: " << (column+1) << "\n"
                    << "  row: " << (row+1) << "\n"
                    << "  cell: " << cell << "\n"
                    << "  message: Cell out of range. Header has only " << this->size() << " columns\n";
                return std::unexpected(std::move(error_message).str());
            }

            if(std::ranges::contains(this->settings_.line_separator, stream.peek()) || stream.eof()){
                if(column+1 != this->size()){
                    std::stringstream error_message;
                    error_message 
                        << "Error parsing csv\n"
                        << "  column: " << (column+1) << "\n"
                        << "  row: " << (row+1) << "\n"
                        << "  cell: " << cell << "\n"
                        << "  message: Line terminator too early. Header has " << this->size() << " columns\n";
                    return std::unexpected(std::move(error_message).str());
                }
                column = 0;
                ++row;
            }else{
                if(column+1 == this->size()){
                    std::stringstream error_message;
                    error_message
                        << "Error parsing csv\n"
                        << "  column: " << (column+1) << "\n"
                        << "  row: " << (row+1) << "\n"
                        << "  cell: " << cell << "\n"
                        << "  message: Expected line separator but got '" << char_symbol(stream.peek()) << "' Header has " << this->size() << " columns\n";
                    return std::unexpected(std::move(error_message).str());
                }

                if(!std::ranges::contains(this->settings_.value_separator, stream.peek())){
                    std::stringstream error_message;
                    error_message
                        << "Error parsing csv\n"
                        << "  column: " << (column+1) << "\n"
                        << "  row: " << (row+1) << "\n"
                        << "  cell: " << cell << "\n"
                        << "  message: Expected value separator [";
                    for(char c : this->settings_.value_separator){
                        error_message << char_symbol(c);
                    }
                    error_message << "] but got '" << char_symbol(stream.peek()) << "' Header has " << this->size() << " columns\n";
                    return std::unexpected(std::move(error_message).str());
                }
                ++column;
            }

            // consume the delimiter
            stream.ignore();
        }
        return {};
    }

    std::expected<void, std::string> CSVd::read_with_header(std::istream& stream){
        // read header names
        size_t column_index = 0;
        while(stream.eof() == false){

            if(stream.bad()){
                std::stringstream error_message;
                error_message 
                    << "Error parsing csv\n"
                    << "  column: " << (column_index+1) << "\n"
                    << "  row: " << (1) << "\n"
                    << "  message: Bad stream";
                return std::unexpected(std::move(error_message).str());
            }

            // parse cell
            std::string cell = read_cell(stream);
            cell = trim_whitespaces(cell);
            if(this->settings_.auto_quotes){
                cell = trim(cell, this->settings_.quotes);
            }
            
            // create new column
            Column column;
            column.name = cell;
            this->push_back(std::move(column));

            // check if the line has ended
            const bool has_line_separator = std::ranges::contains(this->settings_.line_separator, stream.peek());
            
            // consume delimiter
            stream.ignore();

            // break if the line has ended
            if(has_line_separator){
                break;
            }

            ++column_index;
        }

        return {};
    }

    std::expected<void, std::string> CSVd::read_without_header(std::istream& stream){
        // read first line and allocate columns
        size_t column_index = 0;
        while(stream.eof() == false){

            if(stream.bad()){
                std::stringstream error_message;
                error_message 
                    << "Error parsing csv\n"
                    << "  column: " << (column_index+1) << "\n"
                    << "  row: " << (1) << "\n"
                    << "  message: Bad stream";
                return std::unexpected(std::move(error_message).str());
            }

            std::string cell = read_cell(stream);

            cell = trim_whitespaces(cell);

            if(this->settings_.auto_quotes){
                cell = trim(cell, this->settings_.quotes);
            }
            
            Column column;
            double value = 0;
            {
                const std::from_chars_result result = std::from_chars(cell.data(), cell.data() + cell.size(), value);
                if(result.ec != std::errc{}){
                    std::stringstream error_message;
                    error_message 
                        << "Error parsing csv\n"
                        << "  column: " << (column_index + 1) << "\n"
                        << "  row: " << (1) << "\n"
                        << "  cell: " << cell << "\n"
                        << "  message: Cannot convert cell to floating-point number\n";
                    return std::unexpected(std::move(error_message).str());
                }
            }

            column.data.emplace_back(value);
            this->push_back(std::move(column));

            // check if the line has ended
            const bool has_line_separator = std::ranges::contains(this->settings_.line_separator, stream.peek());
            
            // consume delimiter
            stream.ignore();

            // break if the line has ended
            if(has_line_separator){
                break;
            }

            ++column_index;
        }

        return {};
    }

    void CSVd::write(std::ostream& stream) const {
        // check if this has elements and can be accessed
        if(this->empty()){
            return;
        }

        HeaderType header_type = this->settings_.header_type;

        if(header_type == HeaderType::Auto){
            header_type = HeaderType::None;
            for(const csvd::Column& col : *this){
                if(col.name.empty() == false){
                    header_type = HeaderType::FirstRow;
                    break;
                }
            }
        }

        // print header
        if(header_type == HeaderType::FirstRow){
            auto col_itr = this->begin();
            size_t index = 0;
            while(col_itr != this->end()){
                if(this->settings_.auto_quotes){
                    stream << this->settings_.quotes.front(); // quotes is guaranteed to not be empty
                }

                if(col_itr->name.empty() == false){
                    stream << col_itr->name;
                }else{
                    // print column number in quotes
                    if(this->settings_.auto_quotes == false){
                        stream << this->settings_.quotes.front(); // quotes is guaranteed to not be empty
                    }
                    stream << index;
                    if(this->settings_.auto_quotes == false){
                        stream << this->settings_.quotes.front(); // quotes is guaranteed to not be empty
                    }
                }

                if(this->settings_.auto_quotes){
                    stream << this->settings_.quotes.front(); // quotes is guaranteed to not be empty
                }

                ++col_itr;
                ++index;

                if(col_itr == this->end()){
                    // end of line
                    stream << this->settings_.line_separator.front(); // line separator is guaranteed to not be empty
                }else{
                    // end of cell
                    stream << this->settings_.value_separator.front(); // value_separator is guaranteed to not be empty
                }
            }
        }

        // get the smallest data vector size
        size_t min_data_length = std::numeric_limits<size_t>::max();
        for(const csvd::Column& col : *this){
            min_data_length = std::min(col.data.size(), min_data_length);
        }

        // print data
        for(size_t row = 0; row < min_data_length; ++row){
            auto col_itr = this->begin();
            while(col_itr != this->end()){
                stream << col_itr->data[row];
                ++col_itr;
                if(col_itr == this->end()){
                    // end of line
                    stream << this->settings_.line_separator.front(); // line separator is guaranteed to not be empty
                }else{
                    // end of cell
                    stream << this->settings_.value_separator.front(); // value_separator is guaranteed to not be empty
                }
            }
        }
    }

}//namespace csvd