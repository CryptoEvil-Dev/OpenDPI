# 5. Примеры использования conf

# RyamlConfigReader

## Получение данных
```cpp
#include <conf/yaml.hpp>

conf::RyamlConfigReader reader("path/to/file.yaml");
std::string path = "nodes.node1";

std::string_view result = reader.get_value(path).value();
std::cout << result << std::endl;
```

## Получение перечисления
```cpp
#include <conf/yaml.hpp>

conf::RyamlConfigReader reader("path/to/file.yaml");
std::string path = "nodes.node1";

std::vector<std::string_view> result = reader.get_sequence(path).value();

std::cout << result[0] << std::endl
std::cout << result[1] << std::endl
std::cout << result[2] << std::endl

```

# SchemaValidator

## Валидация скалярных данных
```cpp
#include <conf/yaml.hpp>

conf::RyamlConfigReader reader("path/to/file.yaml");
conf::SchemaValidator validator;

validator.check_numeric(reader, "nodes.node1")
         .check_numeric(reader, "nodes.node2")
         .check_numeric(reader, "nodes.node3");

/*
 * Или через constraints:
 * 
 * validator.check_numeric(reader, "nodes.node1", {.min=10, .max=100})
 *          .check_numeric(reader, "nodes.node2", {.min=20, .max=200})
 *          .check_numeric(reader, "nodes.node3", {.eq=1024});
 * 
*/

conf::ValidationResult res = validator.release();
if(!res.is_valid()) {
    std::cerr << res.get_log() << std::endl;
    exit(EXIT_FAILURE)
}
std::cout << "Configuration is valid"<< std::endl;

```

## Валидация строковых данных
```cpp
#include <conf/yaml.hpp>

conf::RyamlConfigReader reader("path/to/file.yaml");
conf::SchemaValidator validator;

validator.check_string(reader, "nodes.node1")
         .check_string(reader, "nodes.node2")
         .check_string(reader, "nodes.node3");

/*
 * Или через constraints:
 * 
 * validator.check_string(reader, "nodes.node1", {.max_length=25})
 *          .check_string(reader, "nodes.node2", {.min_length=5, .max_length=1024})
 *          .check_string(reader, "nodes.node3", {.is_contained="Hello World!"});
 * 
*/

conf::ValidationResult res = validator.release();
if(!res.is_valid()) {
    std::cerr << res.get_log() << std::endl;
    exit(EXIT_FAILURE)
}
std::cout << "Configuration is valid"<< std::endl;

```