#ifndef MAP_TO_CSV_FILE_HANDLER_H
#define MAP_TO_CSV_FILE_HANDLER_H

#include <unordered_map>
#include <fstream>

class MapToCsvFileHandler {
public:
    template<class T, class D>
    static void to_csv(const std::string& filename, std::unordered_map<T, D> map, char delimiter) {
        std::ofstream csv_file{filename};

        for (auto key_value : map) {
            csv_file << std::to_string(key_value.first) << delimiter << std::to_string(key_value.second) << "\n";
        }
    }
};

#endif
