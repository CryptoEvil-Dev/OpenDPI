#include <gtest/gtest.h>
#include <conf/yaml.hpp>


TEST(RyamlTest, DeepNesting) {
    conf::RyamlConfigReader reader("deep_nesting.yaml");
    conf::SchemaValidator validator;

    validator.check_string(reader, "hello.this.is.my.configuration.file", conf::StringConstraint{.is_contained = "Hello World!"})
             .check_string(reader, "hello.have.a.nice.day", conf::StringConstraint{.is_contained = "bruh"})
             .check_string(reader, "hello.have.a.good.day", conf::StringConstraint{.is_contained = "bruh"});

    conf::ValidationResult result = validator.release();
    EXPECT_TRUE(result.is_valid());
}

TEST(RyamlTest, Sequence) {
    conf::RyamlConfigReader reader("deep_nesting.yaml");

    std::string path_buffer = "hello.this.is.my.sequence";

    std::vector<std::string_view> seq = reader.get_sequence(path_buffer).value();
    EXPECT_EQ(seq[0], "Hello!");
    EXPECT_EQ(seq[1], "This");
    EXPECT_EQ(seq[2], "is");
    EXPECT_EQ(seq[3], "sequence");
    EXPECT_EQ(seq[4], "in");
    EXPECT_EQ(seq[5], "yaml");
    EXPECT_EQ(seq[6], "file");
}

TEST(RyamlTest, MoreSequnce) {
    conf::RyamlConfigReader reader("deep_nesting.yaml");
    std::string path_buffer = "seqs.node1";

    std::vector<std::string_view> seq = reader.get_sequence(path_buffer).value();
    EXPECT_EQ(seq[0], "node11");
    EXPECT_EQ(seq[1], "node12");
    EXPECT_EQ(seq[2], "node13");
    EXPECT_EQ(seq[3], "node14");
    EXPECT_EQ(seq[4], "node15");

    path_buffer = "seqs.node2";
    seq = reader.get_sequence(path_buffer).value();
    EXPECT_EQ(seq[0], "node21");
    EXPECT_EQ(seq[1], "node22");
    EXPECT_EQ(seq[2], "node23");
    EXPECT_EQ(seq[3], "node24");
    EXPECT_EQ(seq[4], "node25");


    path_buffer = "seqs.node3";
    seq = reader.get_sequence(path_buffer).value();
    EXPECT_EQ(seq[0], "node31");
    EXPECT_EQ(seq[1], "node32");
    EXPECT_EQ(seq[2], "node33");
    EXPECT_EQ(seq[3], "node34");
    EXPECT_EQ(seq[4], "node35");


    path_buffer = "seqs.node4";
    seq = reader.get_sequence(path_buffer).value();
    EXPECT_EQ(seq[0], "node41");
    EXPECT_EQ(seq[1], "node42");
    EXPECT_EQ(seq[2], "node43");
    EXPECT_EQ(seq[3], "node44");
    EXPECT_EQ(seq[4], "node45");


    path_buffer = "seqs.node5";
    seq = reader.get_sequence(path_buffer).value();
    EXPECT_EQ(seq[0], "node51");
    EXPECT_EQ(seq[1], "node52");
    EXPECT_EQ(seq[2], "node53");
    EXPECT_EQ(seq[3], "node54");
    EXPECT_EQ(seq[4], "node55");

}

TEST(RyamlTest, Duplicates) {
    conf::RyamlConfigReader reader("deep_nesting.yaml");
    std::string path_buffer = "dup.node1";

    std::vector<std::string_view> seq = reader.get_sequence(path_buffer).value();
    EXPECT_EQ(seq[0], "node1");
    EXPECT_EQ(seq[1], "node2");
    EXPECT_EQ(seq[2], "node3");
}

TEST(RyamlTest, MismatchField) {
    conf::RyamlConfigReader reader("deep_nesting.yaml");
    std::string path_buffer = "seqs.node1";

    try {
        std::string_view result = reader.get_value(path_buffer).value();
        FAIL() << "Expected: bad optional access";
    } catch(const std::exception& ex) {
        EXPECT_STREQ(ex.what(), "bad optional access");
    }

}

TEST(RyamlTest, MismatchField2) {
    conf::RyamlConfigReader reader("deep_nesting.yaml");
    std::string path_buffer = "hello.have.a.nice.day";

    try {
        std::vector<std::string_view> seq = reader.get_sequence(path_buffer).value();
        FAIL() << "Expected: bad optional access";
    } catch(const std::exception& ex) {
        EXPECT_STREQ(ex.what(), "bad optional access");
    }
}