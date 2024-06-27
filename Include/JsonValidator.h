#include <string>
#include <vector>
#include <memory>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <json/json.h>
#include <unordered_map>
#include "JsonSerialize.h"
#include <iostream>


class JsonValidate
{
public:
    using Pointer = std::shared_ptr<JsonValidate>;
    using PairJson = std::pair<const Json::ValueType, std::string>;
    using MapJson = std::unordered_map<std::string, PairJson>;

    explicit JsonValidate() {}
    explicit JsonValidate(const std::string& jsonSchema)
      : serializeSchema(JsonSerializeSchema::create(jsonSchema))
    {
    }
    explicit JsonValidate(const Json::Value& jsonSchema)
      : serializeSchema(JsonSerializeSchema::create(jsonSchema))
    {
    }

    virtual ~JsonValidate() = default;

    static JsonValidate::Pointer create(const std::string& jsonSchema)
    {
        return std::make_shared<JsonValidate>(jsonSchema);
    }
    static JsonValidate::Pointer create(const Json::Value& jsonSchema)
    {
        return std::make_shared<JsonValidate>(jsonSchema);
    }

    void validateJsonTag(const JsonSerialize::Member& member, const JsonSerialize::KeyDir& keyDir,
                          JsonSerialize::MemberType memberType)
    {
        std::cout << "valDataType: Member: " << member << " KeyDir: " << keyDir << " MemberType: " << memberType
                  << std::endl;
        Json::Value jsonShema = serializeSchema->getPropertyAttribute(member, keyDir);
        std::cout << "JsonSchema: " << jsonShema.toStyledString() << std::endl;
        auto yyy = jsonShema["schema"]["type"].asString();
        auto xx = serializeSchema->convertType(jsonShema["schema"]["type"].asString());
        if (serializeSchema->convertType(jsonShema["schema"]["type"].asString()) != memberType) {
            const std::string message(("Error on validate type for: " + keyDir));
            throw (std::exception(message.c_str()));

        }
        
    }

 

    bool validate(const std::string& jsonString)
    {
        serializeData = JsonSerializeData::create(jsonString);

        serializeData->validateDataTags(std::bind(&JsonValidate::validateJsonTag, this, std::placeholders::_1,
                                              std::placeholders::_2, std::placeholders::_3));

        return true;
    }

private:
    const std::string jsonSchema;
    JsonSerializeSchema::Pointer serializeSchema;
    JsonSerializeData::Pointer serializeData;
};
