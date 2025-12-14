#pragma once

#include <vector>
#include <string>
#include <expected>

namespace csvd{
    
    /**
     * @brief Represenst a column of a csv file with a name and data vector
     */
    struct Column{
        std::string name; ///< The name used in the header of the csv file. Empty if there is no header.
        std::vector<double> data; ///< The data vector that correlates to the header name
    };

    /**
     * @brief Specifies the header
     */
    enum class HeaderType{
        None,           ///< No header names, first row is data row
        FirstRow,       ///< First row is a header with names
        Auto            ///< First row is automatically determined. If the first character of the first row is a: digit, `+`, `-` --> the row is assumed to be a datarow. Any other character --> assumed to be a named header row.
    };

    struct Settings{
        HeaderType header_type = HeaderType::Auto;
        std::string value_separator = ",;\t";
        std::string quotes = "\"'";
        std::string line_separator = "\n";
        bool auto_quotes = true;
    };

    /**
     * @brief A CSV Parser and writer for tables containing floating-point numbers
     * 
     * Supports:
     *  - custom value separators (multiple allowed)
     *  - custom line separators (multiple allowed)
     *  - custom quotes (multiple allowed)
     *  - auto remove/add quotes
     *  - auto header detection (heuristic on the first non-whitespace. Digit, `+`, `-` is assumed to be a data row, else a named header row)
     *  
     * 
     * Does not support 
     *  - escape characters
     *  - escaped quotes
     * 
     */
    class CSVd{
        
        using iterator = std::vector<csvd::Column>::iterator;
        using const_iterator = std::vector<csvd::Column>::const_iterator;

        using pointer = std::vector<csvd::Column>::pointer;
        using const_pointer = std::vector<csvd::Column>::const_pointer;

        using reference = std::vector<csvd::Column>::reference;
        using const_reference = std::vector<csvd::Column>::const_reference;

        using value_type = std::vector<csvd::Column>::value_type;

        using size_type = std::vector<csvd::Column>::size_type;

        public:

            CSVd() = default;

            CSVd(csvd::Settings settings) : settings_(settings){};

            CSVd( const CSVd& other ) = default;
            CSVd( CSVd&& other ) = default;

            CSVd& operator=( const CSVd& other ) = default;
            CSVd& operator=( CSVd&& other ) = default;

            inline const csvd::Settings& settings() const {return this->settings_;} 

            inline reference at( size_type pos ) {return this->columns_.at(pos);}
            inline const_reference at( size_type pos ) const {return this->columns_.at(pos);}

            inline reference operator[]( size_type pos ){return this->columns_[pos];}
            inline const_reference operator[]( size_type pos ) const {return this->columns_[pos];}

            inline iterator begin() {return this->columns_.begin();}
            inline const_iterator begin() const {return this->columns_.begin();}
            inline const_iterator cbegin() const {return this->columns_.cbegin();}

            inline iterator end() {return this->columns_.end();}
            inline const_iterator end() const {return this->columns_.end();}
            inline const_iterator cend() const {return this->columns_.cend();}

            inline bool empty() const {return this->columns_.empty();}
            inline size_type size() const {return this->columns_.size();}
            inline size_type max_size() const {return this->columns_.max_size();}

            inline void reserve(size_type n) {this->columns_.reserve(n);}
            inline size_type capacity() const {return this->columns_.capacity();}

            inline void clear() {this->columns_.clear();}
            
            inline iterator insert(const_iterator pos, const csvd::Column& value){return this->columns_.insert(pos, value);}
            inline iterator insert(const_iterator pos, csvd::Column&& value){return this->columns_.insert(pos, std::move(value));}
            inline iterator erase( iterator pos ){return this->columns_.erase(pos);}
            inline iterator erase( const_iterator pos ){return this->columns_.erase(pos);}
            inline iterator erase( iterator first, iterator last ){return this->columns_.erase(first, last);}
            inline iterator erase( const_iterator first, const_iterator last ){return this->columns_.erase(first, last);}

            inline void push_back( const csvd::Column& value ){this->columns_.push_back(value);}
            inline void push_back( csvd::Column&& value ){this->columns_.push_back(std::move(value));}

            template< class... Args >
            inline auto emplace_back( Args&&... args ){return this->columns_.emplace_back(std::forward<Args>(args)...);}

            inline void pop_back(){this->columns_.pop_back();}

            void swap( CSVd& other ) noexcept {
                std::swap(this->settings_, other.settings_);
                std::swap(this->columns_, other.columns_);
            }

            /**
             * @brief Sets the header type. Select if there is a header or not.
             * 
             * HeaderType:
             *  - None
             *  - FirstRow
             * 
             * @param header The header type
             */
            void set_header_type(HeaderType header);
            
            /**
             * @brief Returns the current header type/setting
             */
            [[nodiscard]] HeaderType header_type() const;

            /**
             * @brief Sets the value separator
             * 
             * You can set multiple. The first character in the list will be used for writing values.
             * All will be used for reading values.
             * 
             * The new separator cannot be empty.
             * If an empty list of separator character is passed the old one will be kept.
             * 
             * @param separator the new separator that will be used for parsing
             */
            void set_value_separator(std::string_view separator);

            /**
             * @brief Returns the list of value separators
             * 
             * The first character in the list will be used for writing values.
             * All will be used for reading values.
             */
            [[nodiscard]] std::string_view value_separator() const;
        
            /**
             * @brief Provides a list of line separators to trigger on 
             * 
             * The first character in the list will be used for writing a csv.
             * All will be triggered on for reading.
             * 
             * The new separator cannot be empty.
             * If an empty list of separator character is passed the old one will be kept.
             * 
             * @param separator The new list of line separators
             */
            void set_line_separator(std::string_view separator);

            /**
             * @brief Returns the current list of line separators that this will trigger on
             * @return a string view of the characters
             */
            [[nodiscard]] std::string_view line_separator() const;

            /**
             * @brief finds the first column that matches the name
             * 
             * Returns the first colum that matches the name exactly.
             * 
             * @param name The name to search for
             * @returns An iterator the element that matches the name or then `end()` iterator if nothing matched
             */
            [[nodiscard]] iterator find(std::string_view name);

            /**
             * @brief finds the first column that matches the name
             * 
             * Returns the first colum that matches the name exactly.
             * 
             * @param name The name to search for
             * @returns An iterator the element that matches the name or then `end()` iterator if nothing matched
             */
            [[nodiscard]] const_iterator find(std::string_view name) const;

            /**
             * @brief Reds data from a CSV stream containing written data
             * @param stream The stream containing the CSV data
             * @return An expected void on success or an error string that conatins an error message
             */
            [[nodiscard]] std::expected<void, std::string> read(std::istream& stream);

            /**
             * @brief Writes the CSV data to the output stream
             * 
             * Only writes as many data rows as the smallest data vector has elements
             * 
             * @param stream The stream to write to
             */
            void write(std::ostream& stream) const;

        private:

            [[nodiscard]] std::expected<void, std::string> read_with_header(std::istream& stream);

            [[nodiscard]] std::expected<void, std::string> read_without_header(std::istream& stream);

            /**
             * @brief Reads characters from the string until a delimiter has been found
             * 
             * Will not read the delimiter from the stream. So the next character in the stream
             * will be the delimiter or the end of the stream.
             * 
             * @param stream A reference to the stream object
             * @param delimiter A list of delimiters
             * @return A string containing all characters until the delimiter
             */
            [[nodiscard]] std::string read_cell(std::istream& stream);

            std::vector<csvd::Column> columns_;
            Settings settings_;
    };
}
