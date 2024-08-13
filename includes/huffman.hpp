#ifndef HUFFMAN_HPP
#define HUFFMAN_HPP

#include "utilities.hpp"
#include "priority_queue.hpp"
#include "chrono"

class huffman_node
{
public:
    size_t frequency;
    unsigned char byte_data;
    bool is_leaf = false;
    std::shared_ptr<huffman_node> left = nullptr;
    std::shared_ptr<huffman_node> right = nullptr;

public:
    huffman_node(size_t freq, unsigned char dat) : frequency(freq), byte_data(dat), is_leaf(true) {}

    huffman_node() : frequency(0), is_leaf(false) {}

    huffman_node(const huffman_node &l, const huffman_node &r) : is_leaf(false), frequency(l.frequency + r.frequency)
    {
        left = std::make_shared<huffman_node>(l);
        right = std::make_shared<huffman_node>(r);
    }

    huffman_node(const std::shared_ptr<huffman_node> &l, const std::shared_ptr<huffman_node> &r) : is_leaf(false), frequency(l->frequency + r->frequency)
    {
        left = l;
        right = r;
    }

    huffman_node(const huffman_node &other) : frequency(other.frequency), byte_data(other.byte_data), is_leaf(other.is_leaf), left(other.left), right(other.right) {}

    huffman_node(huffman_node &&) = default;

    bool operator>(const huffman_node &other)
    {
        return this->frequency > other.frequency;
    }

    bool operator>=(const huffman_node &other)
    {
        return this->frequency >= other.frequency;
    }

    bool operator<(const huffman_node &other)
    {
        return this->frequency < other.frequency;
    }

    bool operator<=(const huffman_node &other)
    {
        return this->frequency <= other.frequency;
    }

    huffman_node &operator=(const huffman_node &other)
    {
        if (this != &other)
        {
            frequency = other.frequency;
            byte_data = other.byte_data;
            is_leaf = other.is_leaf;
            left = other.left;
            right = other.right;
        }
        return *this;
    }

    friend std::ostream &operator<<(std::ostream &os, const huffman_node &node)
    {
        os << node.frequency;
        return os;
    }

    static huffman_node empty;
};

huffman_node huffman_node::empty = huffman_node();

class huffman
{
private:
    /*Huffman related variables*/
    std::map<unsigned char, size_t> m_freq_table;
    std::map<unsigned char, std::string> m_encodings;
    std::shared_ptr<huffman_node> m_huffman_root_node;
    /*Bit Writer variables*/
    int m_current_bit = 7;
    unsigned char m_bit_buffer = '\0';
    /*Files*/
    size_t m_buf_size;          // chunk size to read from in file while encoding and decoding
    std::string m_in_file_name; // file to read from , can be used to read encoded or decoded data respectively
    std::ifstream m_in_file;
    std::string m_out_file_name; // file to write to , can be used to write encoded or decoded data respectively
    std::ofstream m_out_file;
    std::string m_tmp_file_name; // temporary file to store huffman header before writing it

    char m_eof_bits{'\0'};

public:
    /// @param m_in_file_name File to read data from
    /// @param m_out_file_name File to write data to
    /// @param buf_size chunk size of reading from file
    huffman(std::string m_in_file_name, std::string m_out_file_name, size_t buf_size = 1 << 16) : m_buf_size(buf_size), m_in_file_name(m_in_file_name), m_out_file_name(m_out_file_name), m_tmp_file_name("tmp_hufman.hufman") {}

    /// @brief encodes the data from the input file, NOTE: It doesn't output to ooutput file, call write to file for that
    /// @param data_size maximum data to read from in_file(NOTE: Writing with limited data is a future aspect)
    void encode(size_t data_size = INT32_MAX)
    {
        m_in_file.open(m_in_file_name, std::ios_base::binary);
        if (!m_in_file.is_open())
        {
            std::cerr << "Cant open file for input";
            return;
        }

        char *_buffer = new char[m_buf_size];
        if (_buffer == nullptr)
        {
            std::cerr << "Unable to allocate memory ";
            return;
        }

        size_t _total_read = 0;

        // Timer
        auto _start = std::chrono::high_resolution_clock::now();

        while (_total_read < data_size && !m_in_file.eof())
        {
            m_in_file.read(_buffer, m_buf_size);
            std::streamsize bytes_read = m_in_file.gcount(); // Get the actual number of bytes read

            if (bytes_read <= 0)
                break;

            for (std::streamsize j = 0; j < bytes_read; ++j)
            {
                m_freq_table[static_cast<unsigned char>(_buffer[j])]++;
            }

            _total_read += bytes_read;
            // std::cout << "Reached total_read val " << total_read << std::endl;
        }

        // Timer
        auto _end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> _dur = _end - _start;
        std::clog << "Time to generate Frequency table : " << _dur.count() << std::endl;

        m_in_file.close();
        delete[] _buffer;
        _buffer = nullptr;
        // std::cout<<"reached eof "<<m_in_file.eof()<<std::endl;
        // std::cout << "Final total_read val " << total_read <<" in data_size "<<data_size << std::endl;
        // show_freq();

        // Timer
        _start = std::chrono::high_resolution_clock::now();

        m_create_huffman_tree();

        // Timer
        _end = std::chrono::high_resolution_clock::now();
        _dur = _end - _start;

        std::clog << "Time to generate Huffman Tree : " << _dur.count() << std::endl;
    }

    /// @brief Writes encoded data to file(NOTE: Writing with limited data is a future aspect)
    void write_to_file()
    {
        m_out_file.open(m_out_file_name, std::ios_base::binary);
        // Ensure m_out_file is open
        if (!m_out_file.is_open())
        {
            std::cerr << "Output file is not open" << std::endl;
            return;
        }

        // Write header to temporary file
        std::ofstream _tmp_file(m_tmp_file_name, std::ios_base::binary);
        if (!_tmp_file.is_open())
        {
            std::cerr << "Unable to open temporary file for writing" << std::endl;
            return;
        }

        // std::clog << "In write to file" << std::endl;
        // m_out_file.clear();
        // m_out_file.seekp(0, std::ios::beg);

        // Timer
        auto _start = std::chrono::high_resolution_clock::now();

        auto _h_start = _tmp_file.tellp();
        write_header(_tmp_file, m_huffman_root_node);

        while (m_current_bit != 7)
        {
            WriteBit(_tmp_file, 0);
        }

        auto _h_end = _tmp_file.tellp();
        _tmp_file.close();

        // Write the header size to the output file
        uint32_t _h_bytes = static_cast<uint32_t>(_h_end - _h_start);
        _h_bytes += 1; // eof bits
        // std::clog << "written header is " << _h_bytes << std::endl;
        m_out_file.write(reinterpret_cast<char *>(&_h_bytes), sizeof(_h_bytes));

        // Open the temporary file for reading
        std::ifstream _tmp_file_in(m_tmp_file_name, std::ios_base::binary);
        if (!_tmp_file_in.is_open())
        {
            std::cerr << "Unable to open temporary file for reading" << std::endl;
            return;
        }

        // Read the entire content of the temporary file into a buffer
        std::vector<char> _buffer((std::istreambuf_iterator<char>(_tmp_file_in)), std::istreambuf_iterator<char>());
        _tmp_file_in.close();

        // Write the buffer to the output file
        m_out_file.write(_buffer.data(), _buffer.size());
        auto _eof_loc = m_out_file.tellp();
        // write the eof byte
        for (int _i = 0; _i < 8; _i++)
        {
            WriteBit(m_out_file, 0);
        }

        // Reset m_in_file to the beginning and read from it
        write_body();
        m_out_file.seekp(_eof_loc);
        m_out_file.write(&m_eof_bits, sizeof(m_eof_bits));

        // Timer
        auto _end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> _dur = _end - _start;
        std::clog << "Time to write to file : " << _dur.count() << std::endl;

        m_out_file.close();
    }

    void show_freq(bool _show_in_binary = false)
    {
        if (_show_in_binary)
        {
            for (auto _i : m_freq_table)
            {
                displayCharBits(_i.first);
                std::cout << " " << _i.second << std::endl;
            }
        }
        else
        {
            for (auto _i : m_freq_table)
            {
                std::cout << _i.first << " " << _i.second << std::endl;
            }
        }
    }

    /// @brief Change the input and output streams
    void change_streams(std::string _in_file_name, std::string _out_file_name)
    {
        this->m_in_file_name = _in_file_name;
        this->m_out_file_name = _out_file_name;
    }

    /// @brief Decodes the data to output file
    void decode()
    {
        m_in_file.open(m_in_file_name, std::ios::binary);
        m_out_file.open(m_out_file_name, std::ios::binary);
        if (!m_in_file.is_open())
        {
            std::cerr << "Error opening file " << m_in_file_name << std::endl;
            return;
        }
        if (!m_out_file.is_open())
        {
            std::cerr << "Error opening file " << m_out_file_name << std::endl;
            return;
        }

        // Timer
        auto _start = std::chrono::high_resolution_clock::now();

        uint32_t _header_size;
        m_in_file.read(reinterpret_cast<char *>(&_header_size), sizeof(_header_size));
        if (m_in_file.gcount() != sizeof(_header_size))
        {
            std::cerr << "Error reading header size" << std::endl;
            return;
        }

        // sub 1 for eof bytes
        _header_size -= 1;
        // std::cout << "Header is " << _header_size << " long\n";

        char *_header = new char[_header_size];
        m_in_file.read(_header, _header_size);
        if (m_in_file.gcount() != _header_size)
        {
            std::cerr << "Error reading header" << std::endl;
            delete[] _header;
            _header = nullptr;
            return;
        }

        bit_reader _reader(_header, _header_size);

        decode_header(_reader);

        delete[] _header;
        _header = nullptr;
        m_in_file.read(&m_eof_bits, sizeof(m_eof_bits));
        // std::cout << "eof bits are " << int(m_eof_bits);
        char *_buffer = new char[m_buf_size];
        unsigned char _dat = '\0';
        m_in_file.clear();

        std::string _cur_input = "";
        while (!m_in_file.eof())
        {
            m_in_file.read(_buffer, m_buf_size);
            size_t _bytes_read = m_in_file.gcount();
            if (_bytes_read == 0)
                break;
            // std::cout << "Bytes read " << _bytes_read << std::endl;
            if (m_in_file.eof() == 1)
            {
                _reader.re_initialize(_buffer, _bytes_read, m_eof_bits);
            }
            else
                _reader.re_initialize(_buffer, _bytes_read);

            int _res = _reader.get_next_bit();
            while (_res != -1)
            {
                _cur_input.push_back(_res + '0');
                if (check_string(m_huffman_root_node, _cur_input, _dat))
                {
                    _cur_input = "";
                    // std::cout << "wrote " << _dat << " to out file\n";
                    m_out_file.write(reinterpret_cast<char *>(&_dat), sizeof(_dat));
                }
                _res = _reader.get_next_bit();
            }
        }

        // Timer
        auto _end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> _dur = _end - _start;
        std::clog << "Time taken to Decode is : " << _dur.count() << std::endl;

        delete[] _buffer;
        _buffer = nullptr;
        m_in_file.close();
        m_out_file.close();
    }

    void clear_data()
    {
        m_freq_table.clear();
        m_encodings.clear();
    }
    ~huffman() = default;

private:
    /// @brief checks for the given string in the huffman tree
    /// @return if such an encoding exists returns true and sets the _dat parameter to the node data else returns false
    bool check_string(std::shared_ptr<huffman_node> root, std::string str, unsigned char &_dat)
    {
        // std::cout<<"Input str "<<str<<std::endl;
        if (str.length() == 0)
        {
            if (root != nullptr && root->is_leaf)
            {
                _dat = root->byte_data;
                return true;
            }
            return false;
        }
        if (str[0] == '1')
        {
            if (root->right != nullptr)
            {
                // std::cout<<"went right! \n";
                return check_string(root->right, str.substr(1), _dat);
            }
            return false;
        }
        else if (str[0] == '0')
        {
            if (root->left != nullptr)
            {
                // std::cout<<"went left! \n";
                return check_string(root->left, str.substr(1), _dat);
            }
            return false;
        }
        return false;
    }

    /// @brief decodes the header from the given stream
    /// @param _reader bit reader with reference to header
    void decode_header(bit_reader &_reader)
    {
        m_huffman_root_node.reset();
        m_huffman_root_node = ReadNode(_reader);
        int _res = _reader.get_next_bit();
        /*while (_res != -1)
        {
            std::cout << "\nRead bit is " << _res;
            _res = _reader.get_next_bit();
        }*/
        generate_tree_encodings(m_huffman_root_node);
        // show_encodings();
        // reading padding bits
        _reader.read_byte_compeletely();
    }

    /// @brief Function to read the root and all chil nodes from the given stream
    /// @param _reader bit reader which constains the buffer to tree header
    /// @return The root node of the generated huffman tree
    std::shared_ptr<huffman_node> ReadNode(bit_reader &_reader)
    {
        int _res = _reader.get_next_bit();
        if (_res == 1)
        {
            unsigned char _dat = '\0';
            for (int i = 7; i >= 0; i--)
            {
                _dat |= (_reader.get_next_bit() << i);
            }
            // std::cout << "\n read " << _dat;
            return std::make_shared<huffman_node>(10, _dat); // arbitrary frequency
        }
        else if (_res == 0)
        {
            auto _left_child = ReadNode(_reader);
            auto _right_child = ReadNode(_reader);
            return std::make_shared<huffman_node>(_left_child, _right_child);
        }
        else
        {
            std::cerr << "\ngot -1\n";
            return nullptr;
        }
    }

    /// @brief Writes bit to file, NOTE: it is users responsibility to ensure that compelete bytes are written , the function will not flush the output until it recieves a byte compeletely;
    /// @param file_to_write output file
    /// @param bit 0 or 1
    void WriteBit(std::ofstream &file_to_write, int bit)
    {
        if (bit == 1)
            m_bit_buffer |= (1 << m_current_bit);

        m_current_bit--;
        if (m_current_bit == -1)
        {
            // fwrite(&m_bit_buffer, 1, 1, f);
            file_to_write.write(reinterpret_cast<char *>(&m_bit_buffer), 1);
            std::bitset<8> _b(m_bit_buffer);
            // std::cout << "\nstoring " << _b << std::endl;
            m_current_bit = 7;
            m_bit_buffer = 0;
        }
    }

    /// @brief writes the huffman equivalent of the data
    /// @param c the char whose mapping will be used
    void Write_in_huffman(unsigned char c)
    {
        std::string encoding = m_encodings[c];
        for (auto i : encoding)
        {
            WriteBit(m_out_file, (i == '1') ? 1 : 0);
        }
    }

    void m_create_huffman_tree()
    {
        pop::min_pq<huffman_node> _huffman_tree((m_freq_table.size()));
        huffman_node emp = huffman_node::empty;
        for (auto &[cha, freq] : m_freq_table)
        {
            // std::clog<<"Adding huffman node "<<freq<<" "<<cha<<std::endl;
            _huffman_tree.insert(huffman_node(freq, cha));
        }
        // _huffman_tree.show();
        // std::clog << "Created Huffman tree with last element index " << _huffman_tree.get_capacity() << "\n";
        // for (int i = 0; i < _huffman_tree.get_capacity(); i++)
        // {
        //     std::cout<<"min element "<<_huffman_tree.get_min()<<std::endl;
        //     _huffman_tree.deletemin();
        // }
        int capacity = _huffman_tree.get_capacity();
        for (int i = 0; i < capacity - 1; i++)
        {
            const huffman_node &smallest = _huffman_tree.get_min();
            _huffman_tree.deletemin();
            const huffman_node &smaller = _huffman_tree.get_min();
            _huffman_tree.deletemin();
            // std::clog<<"Combined "<<smallest.frequency<<" and "<<smaller.frequency<<std::endl;
            _huffman_tree.insert(huffman_node(smallest, smaller));
            // std::cout<<_huffman_tree.get_capacity()<<std::endl;
        }
        // std::cout<<_huffman_tree.get_capacity();
        // _huffman_tree.show();
        const huffman_node &root = _huffman_tree.get_min();
        _huffman_tree.deletemin();
        m_huffman_root_node = std::make_shared<huffman_node>(root);
        generate_tree_encodings(std::make_shared<huffman_node>(root));
        // show_encodings();
    }

    void write_body()
    {
        m_in_file.open(m_in_file_name, std::ios_base::binary);

        m_in_file.seekg(0, std::ios_base::end);
        auto _end = m_in_file.tellg();
        m_in_file.seekg(0, std::ios_base::beg);
        // std::cout<<"end at "<<end;
        char *_buffer = new char[m_buf_size];
        while (!m_in_file.eof())
        {
            m_in_file.read(_buffer, m_buf_size);
            std::streamsize bytes_read = m_in_file.gcount();
            // std::cout << "bytes " << bytes_read;
            if (bytes_read <= 0)
                break;

            for (std::streamsize j = 0; j < bytes_read; ++j)
            {
                unsigned char _c = static_cast<unsigned char>(_buffer[j]);
                if (!m_encodings.count(_c))
                {
                    std::cerr << "Error in frequency table" << std::endl;
                    break;
                }
                else
                {
                    Write_in_huffman(_c);
                }
            }
        }
        // size_t _eof_bits {};
        while (m_current_bit != 7)
        {
            WriteBit(m_out_file, 0);
            m_eof_bits++;
        }
        // std::cout << "Eof bits are " << m_eof_bits;
        // Writing eof bits
        //  m_out_file.seekp(sizeof());//skip the header size
        m_in_file.close();
    }

    /// @brief prints the tree to the output file
    /// @param _file_to_write file to write to
    /// @param _root root node of huffman tree
    /// @param _encoded leave empty to start from root
    void write_header(std::ofstream &_file_to_write, const std::shared_ptr<huffman_node> &_root, std::string _encoded = "")
    {
        if (!_root)
        {
            std::cerr << "root node doesn't exist" << std::endl;
        }
        if (_root->is_leaf)
        {
            WriteBit(_file_to_write, 1);
            // std::cout << "bit write 1\n";
            for (int i = 7; i >= 0; i--)
            {
                WriteBit(_file_to_write, (_root->byte_data >> i) & 1);
                // std::cout << "bit write " << ((_root->byte_data >> i) & 1) << "\n";
            }
            // std::clog << "wrote " << _root->byte_data << std::endl;
        }
        else
        {
            WriteBit(_file_to_write, 0);
            // std::cout << "bit write 0\n";
            write_header(_file_to_write, _root->left, _encoded);
            write_header(_file_to_write, _root->right, _encoded);
        }
    }

    void generate_tree_encodings(const std::shared_ptr<huffman_node> &root, std::string _encoded = "")
    {
        if (root->is_leaf)
        {
            m_encodings[root->byte_data] = _encoded;
        }
        else
        {
            generate_tree_encodings(root->left, _encoded + "0");
            generate_tree_encodings(root->right, _encoded + "1");
        }
    }

    void show_encodings()
    {
        for (auto i : m_encodings)
        {
            std::cout << i.first << " " << i.second << std::endl;
        }
    }
};

#endif