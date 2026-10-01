# 3. Конфигурационные файлы. /conf/iconf[.cpp / .hpp]
Модуль для работы с конфигурационными файлами и валидацией файлов. Имеет под капотом базовый класс **IConfigReader** для реализации парсеров (По сути, если реализуешь в своём парсере get_value и get_sequence, то можешь использовать валидатор для валидации данных).

- `get_value(path)`: возвращает `std::optional<std::string_view>` — значение по пути, или `nullopt`, если узел отсутствует или не является скаляром.
- `get_sequence(path)`: возвращает `std::optional<std::vector<std::string_view>>` — последовательность скаляров, или `nullopt`.

**Возвращённые `string_view` ссылаются на память внутри читателя** — не используйте их после его разрушения.

В модуле есть **SchemaValidator** — очень гибкая штука, проверяет не только тип данных хранящийся по маршруту, но и умеет самостоятельно валидировать данные через **Constraints**, их всего 2:
- **NumericConstraint**: проверяет минимум, максимум и точное равенство.
- **StringConstraint**: минимум/максимум длины и подстрока/равенство.

Есть ещё такой класс как **ValidationResult**, это а-ля поток ошибок, возвращается из `SchemaValidator::release()`, через него можно получить все ошибки в виде красивого лога (`std::string`) или сырого списка (`ValidationData`).

**ValidationData** — структура с двумя полями: `path` (путь до проблемного узла) и `message` (описание ошибки).

Пример использования `SchemaValidator`:
```cpp
conf::RyamlConfigReader reader("config.yaml");
conf::SchemaValidator validator;

validator.check_string(reader, "system.host",
                       conf::StringConstraint{.min_length = 1});

validator.check_numeric<int>(reader, "system.port",
                             conf::NumericConstraint<int>{.min = 1, .max = 65535});

validator.check_sequence(reader, "security.honeypots",
                         conf::StringConstraint{.min_length = 1});

conf::ValidationResult result = validator.release();
if (!result.is_valid()) {
    std::cerr << result.get_log();
    return 1;
}