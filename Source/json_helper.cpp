////////////////////////////////////////////////////////////////////////////
//   Copyright 2020 Teradici Corporation
//   All Rights Reserved
//   No portions of this material may be reproduced in any form without the
//   written permission of Teradici Corporation.  All information contained
//   in this document is Teradici Corporation company private, proprietary,
//   and trade secret.
////////////////////////////////////////////////////////////////////////////

#include "json_helper.h"

#include <memory>


namespace {
template<typename T>
bool parseFromContainer(const T& data, Json::Value& value, std::string& errors)
{
    Json::CharReaderBuilder builder;
    auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
    return reader->parse(data.data(), data.data() + data.size(), &value, &errors);
}

template<typename T>
bool parseFromContainer(const T& data, Json::Value& value)
{
    std::string errors;
    return parseFromContainer(data, value, errors);
}
}  // namespace

namespace jsonhelpers {
bool parse(std::istream& stream, Json::Value& value, std::string& errors)
{
    Json::CharReaderBuilder builder;
    return Json::parseFromStream(builder, stream, &value, &errors);
}
bool parse(std::istream& stream, Json::Value& value)
{
    std::string errors;
    return parse(stream, value, errors);
}
bool parse(const std::string& data, Json::Value& value, std::string& errors)
{
    return parseFromContainer(data, value, errors);
}
bool parse(const std::string& data, Json::Value& value)
{
    return parseFromContainer(data, value);
}
bool parse(const std::vector<char>& data, Json::Value& value, std::string& errors)
{
    return parseFromContainer(data, value, errors);
}
bool parse(const std::vector<char>& data, Json::Value& value)
{
    return parseFromContainer(data, value);
}

Json::Value convertToJsonValue(const SerializedString& serializedStr)
{
    std::string errors;
    Json::Value jsonVal;
    if (!parse(serializedStr, jsonVal, errors))
    {
        throw std::runtime_error("Failure in parsing json typed string: " + errors);
    }

    return jsonVal;
}

SerializedString convertToTypedString(const Json::Value& jsonValue)
{
    Json::StreamWriterBuilder builder{};
    builder["indentation"] = "   ";
    return Json::writeString(builder, jsonValue);
}

bool isJson(const SerializedString& str)
{
    if (str.empty())
    {
        return false;
    }

    // ignore leading whitespace
    auto pos = str.find_first_not_of(" \t\r\n");
    if (pos == std::string::npos)
    {
        // not JSON if the string is composed of whitespace
        return false;
    }

    // first character of JSON *must* be a curly brace (json object) or a square bracket (json array)
    const auto startsWith = str.at(pos);
    if (startsWith != '{' && startsWith != '[')
    {
        return false;
    }

    // last character of JSON must match the character used to start the string
    pos = str.find_last_not_of(" \t\r\n");
    const auto endsWith = str.at(pos);
    if (startsWith == '{' && endsWith != '}')
    {
        return false;
    }
    if (startsWith == '[' && endsWith != ']')
    {
        return false;
    }

    try
    {
        convertToJsonValue(str);
        return true;
    }
    catch (...)
    {
        // no need to do anything, it just means this is not valid JSON
    }
    return false;
}

}  // namespace jsonhelpers
