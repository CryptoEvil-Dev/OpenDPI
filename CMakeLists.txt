cmake_minimum_required(VERSION 3.10)
project(dpi LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "DEBUG." FORCE)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Wall          # Включает базовый набор предупреждений
        -Wextra        # Включает дополнительные полезные предупреждения
        -Wpedantic     # Требует строгого соответствия стандарту ISO C++
        # -Werror        # Превращает ВСЕ предупреждения в ошибки (сборка упадет при любом варнинге)
        
        # Дополнительный строгий аудит сетевого кода:
        -Wshadow               # Предупреждает, если локальная переменная перекрывает имя в более внешней области
        -Wnon-virtual-dtor     # Предупреждает, если у класса есть виртуальные методы, но нет виртуального деструктора
        -Wold-style-cast       # Запрещает си-подобное приведение типов вроде (int)x, требуя static_cast/reinterpret_cast
        -Wcast-align           # Предупреждает о приведении типов, которое приводит к невыровненному адресу памяти
        -Wunused               # Предупреждает о любых неиспользуемых переменных, функциях или параметрах
        -Woverloaded-virtual   # Предупреждает, если перегруженная виртуальная функция скрывает другую виртуальную функцию
        -Wconversion           # Предупреждает о неявных преобразованиях типов, где могут потеряться данные (например, double в int)
        -Wsign-conversion      # Предупреждает о неявных преобразованиях со знаком / без знака
        -Wnull-dereference     # Предупреждает, если обнаружено потенциальное разыменование нулевого указателя
    )
endif()


add_subdirectory(src/net)
add_subdirectory(src/conf)
add_executable(main main.cpp)
target_link_libraries(main PRIVATE netlib conflib)

option(BUILD_TESTING "Build the testing suite" ON)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

link_directories(src)