#include <gtest/gtest.h>
#include <conf/yaml.hpp>


TEST(RyamlTest, DeepNesting) {
    conf::RyamlConfigReader reader("deep_nesting.yaml");
    conf::SchemaValidator validator;

    validator.check_string(reader, "hello.this.is.my.configuration.file", conf::StringConstraint{.is_contained = "Hello World!"})
             .check_string(reader, "hello.have.a.nice.day", conf::StringConstraint{.is_contained = "bruh"})
             .check_string(reader, "hello.have.a.good.day", conf::StringConstraint{.is_contained = "bruh"});

    conf::ValidationResult result = validator.release();
    std::cout << result.get_log() << std::endl;
    EXPECT_TRUE(result.is_valid());
}