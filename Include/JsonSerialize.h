#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <memory>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>

#include <json/json.h>
#include <map>
#include <functional>
#include <json_helper.h>

struct KeyMap
{
    std::string keyDir;
    std::string keyLevel;
    int level;
    std::string pos;
};

class JsonSerialize
{
public:
    enum class TupleJsonType : int
    {
        Level = 0,
        Member,
        MemberType,
        MemberValue
    };

    using Pointer = std::shared_ptr<JsonSerialize>;
    using Member = const std::string;
    using MemberType = const Json::ValueType;
    using MemberValue = std::string;
    using Level = int;

    using TupleJson = std::tuple<Level, Member, MemberType, MemberValue>;
    using KeyDir = std::string;
    using KeyLevel = std::string;
    using MapJsonInfo = std::map<KeyDir, TupleJson>;
    using MapJsonLevel = std::multimap<KeyLevel, KeyDir>;
    using ItMapJsonInfo = MapJsonInfo::const_iterator;

    explicit JsonSerialize() {}
    explicit JsonSerialize(const Json::Value& jsonData)
    {
        serialize(jsonData);
    }
    virtual ~JsonSerialize() = default;

    // static JsonSerialize::Pointer create(const std::string& jsonString)
    // {
    //     Json::Value jsonData;
    //     try
    //     {
    //         jsonData = jsonhelpers::convertToJsonValue(jsonString);
    //     }
    //     catch (const std::exception& e)
    //     {
    //         throw(std::runtime_error("Error on create Json Serialize: " + std::string(e.what())));
    //     }
    //     return (JsonSerialize::create(jsonData));
    // }

    // static JsonSerialize::Pointer create(const Json::Value& jsonSchema)
    // {
    //     return (std::make_shared<JsonSerialize>(jsonSchema));
    // }

    template<typename TYPE>
    void saveData(const KeyMap& keyMap, Level level, Member& member, MemberType& memberType, TYPE memberValue)
    {
        TupleJson tupleJson = std::make_tuple(level, member, memberType, boost::lexical_cast<std::string>(memberValue));

        std::string keyDir = keyMap.keyDir + member;
        std::string keyLevel = keyMap.pos.empty() ? keyMap.keyLevel.substr(0, keyMap.keyLevel.size() -1) : keyMap.keyLevel + keyMap.pos;

        mapJsonInfo.emplace(keyDir, tupleJson);
        mapJsonLevel.emplace(keyLevel, keyDir);

        std::cout << std::get<(int)TupleJsonType::MemberType>(tupleJson) << " "
            << std::get<(int)TupleJsonType::Level>(tupleJson) << " " << keyLevel << " " << keyMap.keyDir + member
            << " {" << std::get<(int)(int)TupleJsonType::MemberValue>(tupleJson) << "}" << std::endl;
    }

    bool serializeArray(KeyMap keyMap, const std::string& member, Level level, size_t index, const Json::Value& jsonData)
    {
        if (index + 1 > jsonData.size()) {
            return (false);
        }
        Json::Value element = jsonData[(int)index];

        if (element.isObject()) {
            
            KeyMap localKeyMap;
            localKeyMap = keyMap;
            localKeyMap.level = level;
            localKeyMap.keyDir = keyMap.keyDir + member + "[" + boost::lexical_cast<std::string>(index) + "]";
            localKeyMap.keyLevel = keyMap.keyLevel + keyMap.pos + "." + boost::lexical_cast<std::string>(index + 1) + ".";
            localKeyMap.pos = "";
            saveData(localKeyMap, level, "", element.type(), "");

            localKeyMap.keyDir = localKeyMap.keyDir + "/";
            serialize(localKeyMap, element);
        }
        else {
            saveData(keyMap, keyMap.level, "[" + boost::lexical_cast<std::string>(index) + "]", element.type(),
                element.asString());
        }
        return (serializeArray(keyMap, member, level, index + 1, jsonData));
    }

    //void savePredicateItem(auto iMapJsonInfo, KeyDir keyDir)
    //{
    //    //std::cout << "X " << iMapJsonInfo->first << std::endl;

    //    const auto& tupleJson = iMapJsonInfo->second;
    //    Member member = std::get<(int)TupleJsonType::Member>(tupleJson);
    //    MemberType memberType = std::get<(int)TupleJsonType::MemberType>(tupleJson);
    //    MemberValue memberValue = std::get<(int)TupleJsonType::MemberValue>(tupleJson);

    //    if (member == "scope" && memberType == Json::ValueType::objectValue) {
    //        return (getPredicateItens(++iMapJsonInfo, iUpperMapJsonInfo, keyDir));
    //    }
    //    std::string key = keyDir + member;
    //    std::string value = memberValue;

    //    std::cout << "Y " << key << " " << value << std::endl;
    //}

    bool getPredicateItens(auto iMapJsonInfo, auto iUpperMapJsonInfo, int index, Level prevLevel, KeyDir keyDir)
    {

        if (iMapJsonInfo == iUpperMapJsonInfo) {
            return(false);
        }

        const auto& tupleJson = iMapJsonInfo->second;
        Level level = std::get<(int)TupleJsonType::Level>(tupleJson);
        if (level < prevLevel) {
            return (false);
        }

        Member member = std::get<(int)TupleJsonType::Member>(tupleJson);
        MemberType memberType = std::get<(int)TupleJsonType::MemberType>(tupleJson);
        MemberValue memberValue = std::get<(int)TupleJsonType::MemberValue>(tupleJson);

        if (member == "scope" && memberType == Json::ValueType::objectValue) {
            return (getPredicateItens(++iMapJsonInfo, iUpperMapJsonInfo, 0, level, keyDir));
        }
        std::string key = index == 0 ? keyDir + member : keyDir + "[" + std::to_string(index - 1) + "]";
        std::string value = memberValue;

        std::cout << "Y " << key << " " << value << std::endl;

        
        if (memberType == Json::ValueType::arrayValue) {
            memberValue.erase(std::remove(memberValue.begin(), memberValue.end(), '<'), memberValue.end());
            memberValue.erase(std::remove(memberValue.begin(), memberValue.end(), '>'), memberValue.end());
            for (int idx = 0; idx < boost::lexical_cast<int>(memberValue); ++idx) {
                return (getPredicateItens(++iMapJsonInfo, iUpperMapJsonInfo, index + 1, level, key ));
            }
        }

        return (getPredicateItens(++iMapJsonInfo, iUpperMapJsonInfo, 0, level, key + "/"));

    }
    bool getPredicate(const std::string predicate, KeyDir keyDir, Level level)
    {
        for (const auto& [infoKey, infoValue] : this->mapJsonInfo) {
            const auto& tupleJson = mapJsonInfo[infoKey];
            Level level = std::get<(int)TupleJsonType::Level>(tupleJson);
            Member member = std::get<(int)TupleJsonType::Member>(tupleJson);
            if (member != "predicate") {
                continue;
            }
            MemberType memberType = std::get<(int)TupleJsonType::MemberType>(tupleJson);
            if (memberType != Json::ValueType::arrayValue) {
                continue;
            }

            MemberValue memberValue = std::get<(int)TupleJsonType::MemberValue>(tupleJson);
            memberValue.erase(std::remove(memberValue.begin(), memberValue.end(), '<'), memberValue.end());
            memberValue.erase(std::remove(memberValue.begin(), memberValue.end(), '>'), memberValue.end());
            for (int idx = 0; idx < boost::lexical_cast<int>(memberValue); ++idx) {
               const std::string& predicateKey = "/predicate[" + boost::lexical_cast<std::string>(idx) + "]";
               auto iMapJsonInfo = this->mapJsonInfo.find(predicateKey +"/name");
               if (iMapJsonInfo == this->mapJsonInfo.end()) {
                    continue;
               }
               const auto& tupleJson = iMapJsonInfo->second;
               MemberValue memberValue = std::get<(int)TupleJsonType::MemberValue>(tupleJson);
               if (memberValue != predicate) {
                   continue;
               }

               auto iLowMapJsonInfo = this->mapJsonInfo.lower_bound(predicateKey + "/scope");
               auto iUpperMapJsonInfo = this->mapJsonInfo.upper_bound(predicateKey + "/xxxxx");
               

               return (getPredicateItens(iLowMapJsonInfo, iUpperMapJsonInfo, 0, level, keyDir));
               
               for (auto iMapJsonInfo = iLowMapJsonInfo; iMapJsonInfo != iUpperMapJsonInfo; ++iMapJsonInfo)
               {
                   std::cout << iMapJsonInfo->first << std::endl;

                   const auto& tupleJson = iMapJsonInfo->second;
                   Member member = std::get<(int)TupleJsonType::Member>(tupleJson);
                   MemberType memberType = std::get<(int)TupleJsonType::MemberType>(tupleJson);
                   MemberValue memberValue = std::get<(int)TupleJsonType::MemberValue>(tupleJson);
                   if (member == "scope" && memberType == Json::ValueType::objectValue) {
                       continue;
                   }
                   std::string key = keyDir + member;
                   std::string value = memberValue;

                   std::cout << key << " " << value << std::endl;
                   // std::cout << memberType << " " << memberValue << " * " << key << std::endl;
                   continue;
                   KeyMap localKeyMap;
                   localKeyMap.level = level;
                   //localKeyMap.keyDir = keyDir + member + "[" + boost::lexical_cast<std::string>(index) + "]";
                   //localKeyMap.keyLevel = keyMap.keyLevel + keyMap.pos + "." + boost::lexical_cast<std::string>(index + 1) + ".";
                   localKeyMap.pos = "";
                   saveData(localKeyMap, level, member, memberType, memberValue);
               }
               return(true);
            }  
        }

        return(false);
    }
    bool serialize(KeyMap keyMap, const Json::Value& jsonData)
    {
        int pos = 0;
        Json::Value::Members members = jsonData.getMemberNames();
        for (const auto& member : members) {
            ++pos;
            KeyMap localKeyMap;
            localKeyMap.keyDir = keyMap.keyDir + member + "/";
            localKeyMap.level = keyMap.level + 1;
            localKeyMap.pos = std::to_string(pos);
            localKeyMap.keyLevel = keyMap.keyLevel + std::to_string(pos) + ".";

            keyMap.pos = std::to_string(pos);
            Json::Value jsonTag = jsonData[member];
            switch (jsonTag.type()) {
            case Json::ValueType::objectValue:
                saveData(keyMap, localKeyMap.level, member, jsonTag.type(), "");
                serialize(localKeyMap, jsonTag);
                break;
            case Json::ValueType::arrayValue:
                saveData(keyMap, localKeyMap.level, member, jsonTag.type(), "<" + std::to_string(jsonTag.size()) + ">");
                serializeArray(keyMap, member, localKeyMap.level + 1, 0, jsonTag);
                break;
            case Json::ValueType::stringValue:
                if (member == "predicate") {
                    getPredicate(jsonTag.asString(), keyMap.keyDir, localKeyMap.level);
                }
                saveData(keyMap, localKeyMap.level, member, jsonTag.type(), jsonTag.asString());
                break;
            case Json::ValueType::intValue:
                saveData(keyMap, localKeyMap.level, member, jsonTag.type(), jsonTag.asInt());
                break;
            case Json::ValueType::uintValue:
                saveData(keyMap, localKeyMap.level, member, jsonTag.type(), jsonTag.asUInt());
                break;
            case Json::ValueType::realValue:
                saveData(keyMap, localKeyMap.level, member, jsonTag.type(), jsonTag.asDouble());
                break;
            case Json::ValueType::booleanValue:
                saveData(keyMap, keyMap.level, member, jsonTag.type(), jsonTag.asBool());
                break;
            case Json::ValueType::nullValue:
                saveData(keyMap, localKeyMap.level, member, jsonTag.type(), "");
                break;
            default:
                break;
            }
        }
        return true;
    }
    Json::ValueType convertType(const std::string& type)
    {
        std::unordered_map<std::string, Json::ValueType> const mapType = {
          {"null", Json::ValueType::nullValue},     {"integer", Json::ValueType::intValue},
          {"uinteger", Json::ValueType::uintValue}, {"real", Json::ValueType::realValue},
          {"string", Json::ValueType::stringValue}, {"boolean", Json::ValueType::booleanValue},
          {"array", Json::ValueType::arrayValue},   {"object", Json::ValueType::objectValue} };

        auto it = mapType.find(type);
        if (it != mapType.end()) {
            return it->second;
        }

        return (Json::ValueType::nullValue);
    }

    bool serialize(const Json::Value& jsonData)
    {
        KeyMap localKeyMap;
        localKeyMap.keyDir = "/";
        localKeyMap.level = 0;
        localKeyMap.pos = "1";
        localKeyMap.keyLevel = "";

        return serialize(localKeyMap, jsonData);
    }

    bool serialize(const std::string& jsonString)
    {
        return (serialize(jsonhelpers::convertToJsonValue(jsonString)));
    }

    bool findJsonData(const std::string& keyDir)
    {
        std::cout << "getJsonData: " << keyDir << std::endl;
        ItMapJsonInfo iMapJsonInfo = mapJsonInfo.find(keyDir);
        if (iMapJsonInfo == mapJsonInfo.end()) {
            return (false);
        }
        return (true);
    }

    template<typename TYPE>
    TYPE getJsonAttribute(const std::string& keyDir, TYPE defaultValue)
    {
        ItMapJsonInfo iMapJsonInfo = mapJsonInfo.find(keyDir);
        if (iMapJsonInfo == mapJsonInfo.end()) {
            return (defaultValue);
        }

        TupleJson tupleJson = std::get<1>(*iMapJsonInfo);
        std::string returnVal = std::get<(int)TupleJsonType::MemberValue>(tupleJson);
        return (boost::lexical_cast<TYPE>(returnVal));
    }

protected:
    MapJsonInfo mapJsonInfo;
    MapJsonLevel mapJsonLevel;
};

class JsonSerializeData : public JsonSerialize
{
public:
    using Pointer = std::shared_ptr<JsonSerializeData>;
    using ValidateTypeFunc = std::function<void(const Member& member, const KeyDir& keyLevel, MemberType MemberType)>;
    explicit JsonSerializeData() {}
    explicit JsonSerializeData(const Json::Value& jsonData)
        : JsonSerialize(jsonData)
    {
    }
    virtual ~JsonSerializeData() = default;

    static JsonSerializeData::Pointer create(const std::string& jsonString)
    {
        Json::Value jsonData;
        try {
            jsonData = jsonhelpers::convertToJsonValue(jsonString);
        }
        catch (const std::exception& e) {
            throw(std::runtime_error("Error on create Json Serialize: " + std::string(e.what())));
        }
        return (JsonSerializeData::create(jsonData));
    }

    static JsonSerializeData::Pointer create(const Json::Value& jsonData)
    {
        return (std::make_shared<JsonSerializeData>(jsonData));
    }

    void validateDataTags(const ValidateTypeFunc& validateFunc)
    {
        for (const auto& [keyLevel, keyDir] : this->mapJsonLevel) {
            const auto& tupleJson = mapJsonInfo[keyDir];
            Level level = std::get<(int)TupleJsonType::Level>(tupleJson);
            Member member = std::get<(int)TupleJsonType::Member>(tupleJson);
            MemberType memberType = std::get<(int)TupleJsonType::MemberType>(tupleJson);
            MemberValue memberValue = std::get<(int)TupleJsonType::MemberValue>(tupleJson);

            std::cout << std::get<(int)TupleJsonType::MemberType>(tupleJson) << " "
                << std::get<(int)TupleJsonType::Level>(tupleJson) << " " << keyLevel << " " << keyDir << " "
                << std::get<(int)TupleJsonType::MemberValue>(tupleJson) << std::endl;

            validateFunc(member, keyDir, memberType);            
        }
    }
};

class JsonSerializeSchema : public JsonSerialize
{
public:
    using Pointer = std::shared_ptr<JsonSerializeSchema>;
    explicit JsonSerializeSchema() {}
    explicit JsonSerializeSchema(const Json::Value& jsonData)
        : JsonSerialize(jsonData)
    {
    }
    virtual ~JsonSerializeSchema() = default;

    static JsonSerializeSchema::Pointer create(const std::string& jsonString)
    {
        Json::Value jsonData;
        try {
            jsonData = jsonhelpers::convertToJsonValue(jsonString);
        }
        catch (const std::exception& e) {
            throw(std::runtime_error("Error on create Json Serialize: " + std::string(e.what())));
        }
        return (JsonSerializeSchema::create(jsonData));
    }

    static JsonSerializeSchema::Pointer create(const Json::Value& jsonData)
    {
        return (std::make_shared<JsonSerializeSchema>(jsonData));
    }

    Json::Value createPropertyAttDependecies(const Member& member, const KeyDir& keyDir)
    {
        Json::Value jsonDepType;
        jsonDepType["xx"] = "ipv6";
        Json::Value jsonSchema = Json::Value(Json::arrayValue);
        jsonSchema.append(jsonDepType);
        return (jsonSchema["dependencies"]);
    }

    Json::Value createPropertyAttribute(const Member& member, const KeyDir& keyDir)
    {
        Json::Value jsonSchema;

        std::string keyAtt = "/schema" + keyDir;

        jsonSchema["schema"] = keyAtt;
        jsonSchema["member"] = member;
        jsonSchema["key"] = keyDir;
        jsonSchema["isMember"] = findJsonData(keyAtt);

        jsonSchema["required"] = getJsonAttribute<bool>(keyAtt + "/required", true);
        jsonSchema["description"] = getJsonAttribute<std::string>(keyAtt + "/description", "");
        jsonSchema["type"] = getJsonAttribute<std::string>(keyAtt + "/type", "undefined");
        jsonSchema["minItems"] = getJsonAttribute<int>(keyAtt + "/minItems", 0);
        if (findJsonData(keyAtt + "/domain")) {
            std::string domainSize = getJsonAttribute<std::string>(keyAtt + "/domain", "");

            domainSize.erase(std::remove(domainSize.begin(), domainSize.end(), '<'), domainSize.end());
            domainSize.erase(std::remove(domainSize.begin(), domainSize.end(), '>'), domainSize.end());

            int size = boost::lexical_cast<int>(domainSize);
            for (int index = 0; index < size; ++index) {
                getJsonAttribute<std::string>(keyAtt + "/domain[" + std::to_string(index) + "]", "");
            }
        }
        jsonSchema["minLength"] = getJsonAttribute<int>(keyAtt + "/minLength", 0);
        jsonSchema["maxLength"] = getJsonAttribute<int>(keyAtt + "/maxLength", 0);
        jsonSchema["dependencies"] = getJsonAttribute<std::string>(keyAtt + "/dependencies", "");
        jsonSchema["enum"] = getJsonAttribute<std::string>(keyAtt + "/enum", "");
        jsonSchema["pattern"] = getJsonAttribute<std::string>(keyAtt + "/pattern", "");

        Json::Value jsonReturn;
        jsonReturn["schema"] = jsonSchema;
        return (jsonReturn);
    }

    Json::Value lookupOnDir(int index, std::vector<std::string>& vkeyDir, const Member& member, KeyDir currentKeyDir,
        KeyDir nextKeyDir)
    {
        if (index == vkeyDir.size()) {
            return (createPropertyAttribute(member, currentKeyDir));
        }
        std::string key = vkeyDir[index];

        std::string keyAtt = nextKeyDir + "/" + key;
        std::string keyLevel = currentKeyDir + "/" + key;

        const std::string type = getJsonAttribute<std::string>("/schema" + keyAtt + "/type", "undefined");
        switch (convertType(type)) {
        case Json::ValueType::objectValue:
            std::cout << "type: " << type << " Key: " << keyAtt << std::endl;
            nextKeyDir = keyAtt + "/properties";
            break;
        default:
            break;
        }

        return (lookupOnDir(index + 1, vkeyDir, member, keyAtt, nextKeyDir));
    }

    Json::Value getPropertyAttribute(const Member& member, const KeyDir& keyDir)
    {
        std::vector<std::string> vkeyDir;

        boost::split(vkeyDir, keyDir.substr(1, keyDir.size()), boost::is_any_of("/"));
        return (lookupOnDir(0, vkeyDir, member, "", ""));

        //std::cout << "getPropertyAttribute: Member: " << member << " KeyDir: " << keyDir << std::endl;
    }
};

