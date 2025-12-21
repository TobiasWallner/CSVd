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

    void ReadError::print(std::ostream& stream) const {
        stream << "Error parsing csv\n";
        stream << "  column: " << (this->col()+1) << '\n';
        stream << "  row: " << (this->row()+1) << '\n';
        const std::string_view cell_content = this->cell();
        stream << "  cell: " << cell_content;
        if(cell_content.size() == this->cell_.size()){
            // if the buffer is actually full, assume that there was more text
            stream << "...";
        }
        stream << '\n';
        stream << "  message: ";
        
        switch(this->error_case()){
            break; case ErrorCase::BadStream:{
                stream << "Bad stream.";
            };
            break; case ErrorCase::UnexpectedEof:{
                stream << "Unexpected end of file (EOF).";
            }
            break; case ErrorCase::ErrorParsingFloat:{
                stream << "Cannot convert cell to floating-point number.";
            }
            break; case ErrorCase::CellOutOfRange:{
                stream << "Cell out of range. Data-row has more elements than the header. Cannot assign data-point to a column.";
            }
            break; case ErrorCase::UnexpectedLineSeparator:{
                stream << "Unexpected line-separator '" << char_symbol(this->peek_) << "', expected a value-separator [";
                for(size_t i = 0; i < this->expected_.size(); ++i){
                    const char separator = this->expected_[i];
                    if(separator == '\0') break;
                    if(i != 0) stream << ", ";
                    stream << '\'' << char_symbol(separator) << '\'';
                }
                stream << "]";
            }
            break; case ErrorCase::ExpectedLineSeparator :{
                stream << "Expected a line-separator [";
                for(size_t i = 0; i < this->expected_.size(); ++i){
                    const char separator = this->expected_[i];
                    if(separator == '\0') break;
                    if(i != 0) stream << ", ";
                    stream << '\'' << char_symbol(separator) << '\'';
                }
                stream << "] but got '" << char_symbol(this->peek_) << "'";
            }
            break; case ErrorCase::ExpectedValueSeparator :{
                stream << "Expected a value separator [";
                for(size_t i = 0; i < this->expected_.size(); ++i){
                    const char separator = this->expected_[i];
                    if(separator == '\0') break;
                    if(i != 0) stream << ", ";
                    stream << '\'' << char_symbol(separator) << '\'';
                }
                stream << "] but got '" << char_symbol(this->peek_) << "'";
            }
            break; case ErrorCase::CellTooLong :{
                stream << "Cell is too long and contains more than 128 characters. Note that this library does only support cells with a maximum length of 128 characters.";
            }
            break; default: {
                stream << "No error message for this error. This is an internal error. Please write an issue to the developers.";
            }

        };
        stream << "\n";
        stream.flush();
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
    std::optional<std::string_view> CSVd::read_cell(char* buffer_first, size_t buffer_size, std::istream& stream){
        char * itr = buffer_first;
        char * buffer_last = buffer_first + buffer_size;
        bool is_in_quote = false;
        while(stream.eof() == false){
            if(itr == buffer_last){
                return std::nullopt;
            }
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

            
            bool has_value_separator = std::ranges::contains(this->settings_.value_separators, stream.peek());
            bool has_line_separator = std::ranges::contains(this->settings_.line_separators, stream.peek());
            
            // ignore value separator if in quotes
            if(is_in_quote){
                has_value_separator = false;
            }

            // break on separators
            if(has_value_separator || has_line_separator){
                break;
            }

            //Append characters
            *itr = stream.get();
            ++itr;
        }
        return std::string_view(buffer_first, itr);
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

    [[nodiscard]] static std::string_view trim(std::string_view string, std::string_view trim_chars){
        const std::string::size_type pos1 = string.find_first_not_of(trim_chars);
        const std::string::size_type pos2 = string.find_last_not_of(trim_chars);
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

    void CSVd::set_value_separators(std::string_view separator){
        if(separator.empty()){
            return;
        }

        const size_t max_length = this->settings_.value_separators.size();
        
        size_t i = 0;
        for(; i < max_length && i < separator.size(); ++i){
            this->settings_.value_separators[i] = separator[i];
        }

        if(i < max_length){
            this->settings_.value_separators[i] = '\0';
        }
    }

    std::string_view CSVd::value_separators() const {
        const size_t max_length = this->settings_.value_separators.size();
        for(size_t i = 0; i < max_length; ++i){
            char c = this->settings_.value_separators[i];
            if(c == '\0'){
                return std::string_view(&this->settings_.value_separators[0], &this->settings_.value_separators[0] + i);
            }
        }
        return std::string_view(&this->settings_.value_separators[0], &this->settings_.value_separators[0] + max_length);
    }

    void CSVd::set_line_separators(std::string_view separator){
        if(separator.empty()){
            return;
        }

        const size_t max_length = this->settings_.line_separators.size();
        
        size_t i = 0;
        for(; i < max_length && i < separator.size(); ++i){
            this->settings_.line_separators[i] = separator[i];
        }

        if(i < max_length){
            this->settings_.line_separators[i] = '\0';
        }
    }

    std::string_view CSVd::line_separators() const {
        const size_t max_length = this->settings_.line_separators.size();
        for(size_t i = 0; i < max_length; ++i){
            char c = this->settings_.line_separators[i];
            if(c == '\0'){
                return std::string_view(&this->settings_.line_separators[0], &this->settings_.line_separators[0] + i);
            }
        }
        return std::string_view(&this->settings_.line_separators[0], &this->settings_.line_separators[0] + max_length);
    }

    std::string_view CSVd::quotes() const {
        const size_t max_length = this->settings_.quotes.size();
        for(size_t i = 0; i < max_length; ++i){
            char c = this->settings_.quotes[i];
            if(c == '\0'){
                return std::string_view(&this->settings_.quotes[0], &this->settings_.quotes[0] + i);
            }
        }
        return std::string_view(&this->settings_.quotes[0], &this->settings_.quotes[0] + max_length);
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

    std::expected<void, ReadError> CSVd::read(std::istream& stream){
        if(stream.eof()){
            return std::unexpected(ReadError(ErrorCase::UnexpectedEof, "", {'\0'}, 0, 0, '\0'));
        }
        if(stream.bad()){
            return std::unexpected(ReadError(ErrorCase::BadStream, "", {'\0'}, 0, 0, '\0'));
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
            std::expected<void, ReadError> result = this->read_with_header(stream);
            if(result.has_value() == false){
                return result;
            }
            ++row;
        }else{
            std::expected<void, ReadError> result = this->read_without_header(stream);
            if(result.has_value() == false){
                return result;
            }
            ++row;
        }

        // read data
        while((stream.eof() == false)){

            if(stream.bad()){
                return std::unexpected(ReadError(ErrorCase::BadStream, "", {'\0'}, column, row, '\0'));
            }
            
            skip_whitespaces(stream);

            // check for eof after new line
            if(stream.eof()){
                if(column == 0){
                    // eof after new colum --> probably last empty line --> ok
                    break;
                }else{
                    return std::unexpected(ReadError(ErrorCase::UnexpectedEof, "", {'\0'}, column, row, '\0'));
                }
            }

            char buffer[128];
            std::optional<std::string_view> opt_cell = read_cell(buffer, sizeof(buffer) / sizeof(char), stream);
            if(opt_cell.has_value() == false){
                return std::unexpected(ReadError(ErrorCase::CellTooLong, "", {'\0'}, column, row, stream.peek()));
            }
            std::string_view cell = trim_whitespaces(opt_cell.value());
            
            double value = 0;
            {
                const std::from_chars_result result = std::from_chars(cell.data(), cell.data() + cell.size(), value);
                if(result.ec != std::errc{}){
                    return std::unexpected(ReadError(ErrorCase::ErrorParsingFloat, cell, {'\0'}, column, row, stream.peek()));
                }
            }

            if(column < this->size()){
                this->at(column).data.emplace_back(value);
            }else{
                return std::unexpected(ReadError(ErrorCase::CellOutOfRange, cell, {'\0'}, column, row, stream.peek()));
            }

            if(std::ranges::contains(this->settings_.line_separators, stream.peek()) || stream.eof()){
                if(column+1 != this->size()){
                    return std::unexpected(
                        ReadError(ErrorCase::UnexpectedLineSeparator, cell, this->settings_.line_separators, column, row, stream.eof() ? '\0' : stream.peek()));
                }
                column = 0;
                ++row;
            }else{
                if(column+1 == this->size()){
                    return std::unexpected(ReadError(ErrorCase::ExpectedLineSeparator, cell, this->settings_.line_separators, column, row, stream.peek()));
                }

                if(!std::ranges::contains(this->settings_.value_separators, stream.peek())){
                    return std::unexpected(ReadError(ErrorCase::ExpectedValueSeparator, cell, this->settings_.value_separators, column, row, stream.peek()));
                }
                ++column;
            }

            // consume the delimiter
            stream.ignore();
        }
        return {};
    }

    std::expected<void, ReadError> CSVd::read_with_header(std::istream& stream){
        // read header names
        size_t column_index = 0;
        while(stream.eof() == false){

            if(stream.bad()){
                return std::unexpected(ReadError(ErrorCase::BadStream, "", {'\0'}, column_index, 0, '\0'));
            }

            // parse cell
            char buffer[128];
            std::optional<std::string_view> opt_cell = read_cell(buffer, sizeof(buffer) / sizeof(char), stream);
            if(opt_cell.has_value() == false){
                return std::unexpected(ReadError(ErrorCase::CellTooLong, "", {'\0'}, column_index, 0, stream.peek()));
            }
            std::string_view cell = trim_whitespaces(opt_cell.value());
            if(this->settings_.auto_quotes){
                cell = trim(cell, this->quotes());
            }
            
            // create new column
            Column column;
            column.name = cell;
            this->push_back(std::move(column));

            // check if the line has ended
            const bool has_line_separator = std::ranges::contains(this->settings_.line_separators, stream.peek());
            
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

    std::expected<void, ReadError> CSVd::read_without_header(std::istream& stream){
        // read first line and allocate columns
        size_t column_index = 0;
        while(stream.eof() == false){

            if(stream.bad()){
                return std::unexpected(ReadError(ErrorCase::BadStream, "", {'\0'}, column_index, 0, '\0'));;
            }

            char buffer[128];
            std::optional<std::string_view> opt_cell = read_cell(buffer, sizeof(buffer) / sizeof(char), stream);
            if(opt_cell.has_value() == false){
                return std::unexpected(ReadError(ErrorCase::CellTooLong, "", {'\0'}, column_index, 0, stream.peek()));
            }
            std::string_view cell = trim_whitespaces(opt_cell.value());

            if(this->settings_.auto_quotes){
                cell = trim(cell, this->quotes());
            }
            
            Column column;
            double value = 0;
            {
                const std::from_chars_result result = std::from_chars(cell.data(), cell.data() + cell.size(), value);
                if(result.ec != std::errc{}){
                    return std::unexpected(ReadError(ErrorCase::ErrorParsingFloat, cell, {'\0'}, column_index, 0, stream.peek()));;
                }
            }

            column.data.emplace_back(value);
            this->push_back(std::move(column));

            // check if the line has ended
            const bool has_line_separator = std::ranges::contains(this->settings_.line_separators, stream.peek());
            
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
                    stream << this->settings_.quotes[0]; // quotes is guaranteed to not be empty
                }

                if(col_itr->name.empty() == false){
                    stream << col_itr->name;
                }else{
                    // print column number in quotes
                    if(this->settings_.auto_quotes == false){
                        stream << this->settings_.quotes[0]; // quotes is guaranteed to not be empty
                    }
                    stream << index;
                    if(this->settings_.auto_quotes == false){
                        stream << this->settings_.quotes[0]; // quotes is guaranteed to not be empty
                    }
                }

                if(this->settings_.auto_quotes){
                    stream << this->settings_.quotes[0]; // quotes is guaranteed to not be empty
                }

                ++col_itr;
                ++index;

                if(col_itr == this->end()){
                    // end of line
                    stream << this->settings_.line_separators[0]; // line separator is guaranteed to not be empty
                }else{
                    // end of cell
                    stream << this->settings_.value_separators[0]; // value_separators is guaranteed to not be empty
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
                    stream << this->settings_.line_separators[0]; // line separator is guaranteed to not be empty
                }else{
                    // end of cell
                    stream << this->settings_.value_separators[0]; // value_separators is guaranteed to not be empty
                }
            }
        }
    }

    std::expected<CSVd, ReadError> read(std::istream& stream, Settings settings){
        CSVd csv(settings);
        std::expected<void, ReadError> r = csv.read(stream);
        if(r.has_value()){
            return csv;
        }else{
            return std::unexpected(r.error());
        }
    }

}//namespace csvd