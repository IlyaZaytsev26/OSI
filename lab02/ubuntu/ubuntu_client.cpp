#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <limits>
#include <cstring>
#include <vector>
#include <algorithm>
#include <locale>

using namespace std;

const string SHARED_FILE_PATH = "/mnt/c/Users/aaa/Desktop/shared_memory_file.txt";
const wstring DEFAULT_ERROR = L"Ошибка! Приложение требует перезагрузки\n";
const size_t MAX_WCHARS = 1024 / sizeof(wchar_t);

namespace Menu {
    constexpr int PROJECT_FILE = 1;
    constexpr int READ_DATA = 2;
    constexpr int EXIT = 3;
}

int get_Valid_Input(const vector<int>& validOptions);

int open_And_Map_File(void*& mapping_File, size_t& mapped_Size);

bool read_Data(int file, void* mapping_File, size_t mapped_Size);

void cleanup(int& file, void*& mapping_File, size_t& mapped_Size);

int menu_Buttons();

void menu();

int get_Valid_Input(const vector<int>& validOptions) {
    int input;

    while (true) {
        if (wcin >> input && find(validOptions.begin(), validOptions.end(), input) != validOptions.end()) {
            return input;
        }
        wcout << L"Некорректный ввод, попробуйте снова!\n";
        wcin.clear();
        wcin.ignore(numeric_limits<streamsize>::max(), L'\n');
    }
}

int open_And_Map_File(void*& mapping_File, size_t& mapped_Size) {
    int file;
    struct stat st;

    file = open(SHARED_FILE_PATH.c_str(), O_RDONLY);
    mapped_Size = 0;

    if (file == -1) {
        wcout << L"Ошибка при открытии файла. Убедитесь, что сервер создал файл!\n";
        mapping_File = nullptr;
        return -1;
    }

    if (fstat(file, &st) == -1) {
        wcout << L"Ошибка при получении информации о файле!\n";
        close(file);
        mapping_File = nullptr;
        return -1;
    }

    mapping_File = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, file, 0);
    if (mapping_File == MAP_FAILED) {
        wcout << L"Ошибка при проецировании файла\n";
        close(file);
        mapping_File = nullptr;
        return -1;
    }

    mapped_Size = st.st_size;
    wcout << L"Файл успешно проецирован в память\n";
    return file;
}

bool read_Data(int file, void* mapping_File, size_t mapped_Size) {
    if (file == -1 || mapping_File == nullptr || mapped_Size == 0) {
        wcout << L"Сначала выполните проецирование файла!\n";
        return false;
    }

    wcout << L"Клиент получил: " << static_cast<wchar_t*>(mapping_File) << L"\n";
    return true;
}

void cleanup(int& file, void*& mapping_File, size_t& mapped_Size) {
    if (mapping_File != nullptr && mapping_File != MAP_FAILED && mapped_Size > 0) {
        munmap(mapping_File, mapped_Size);
        mapping_File = nullptr;
        mapped_Size = 0;
    }

    if (file != -1) {
        close(file);
        file = -1;
    }
}

int menu_Buttons() {
    int user_Choose;

    wcout << L"\n=== Клиентское приложение ===\n\n";
    wcout << Menu::PROJECT_FILE << L" - Выполнить проецирование\n";
    wcout << Menu::READ_DATA << L" - Прочитать данные\n";
    wcout << Menu::EXIT << L" - Завершить работу\n";

    user_Choose = get_Valid_Input({Menu::PROJECT_FILE, Menu::READ_DATA, Menu::EXIT});
    return user_Choose;
}

void menu() {
    int user_Choose;
    int file;
    void* mapping_File;
    size_t mapped_Size;

    file = -1;
    mapping_File = nullptr;
    mapped_Size = 0;

    do {
        user_Choose = menu_Buttons();

        switch(user_Choose) {
            case Menu::PROJECT_FILE:
                cleanup(file, mapping_File, mapped_Size);
                file = open_And_Map_File(mapping_File, mapped_Size);
                break;
            case Menu::READ_DATA:
                if (!read_Data(file, mapping_File, mapped_Size)) {
                    wcout << L"Не удалось прочитать данные\n";
                }
                break;
            case Menu::EXIT:
                wcout << L"Завершение работы . . .\n";
                break;
            default:
                wcout << DEFAULT_ERROR;
        }
    } while (user_Choose != Menu::EXIT);

    cleanup(file, mapping_File, mapped_Size);
}

int main() {
    setlocale(LC_ALL, "");
    menu();
    return 0;
}
