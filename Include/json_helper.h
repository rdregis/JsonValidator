////////////////////////////////////////////////////////////////////////////
//   Copyright 2020 Teradici Corporation
//   All Rights Reserved
//   No portions of this material may be reproduced in any form without the
//   written permission of Teradici Corporation.  All information contained
//   in this document is Teradici Corporation company private, proprietary,
//   and trade secret.
////////////////////////////////////////////////////////////////////////////

#pragma once

#include <json/json.h>

#include <istream>
#include <string>
#include <vector>
#include <string>
#include <memory>

namespace jsonhelpers {

bool parse(std::istream& stream, Json::Value& value, std::string& errors);
bool parse(std::istream& stream, Json::Value& value);
bool parse(const std::string& data, Json::Value& value, std::string& errors);
bool parse(const std::string& data, Json::Value& value);
bool parse(const std::vector<char>& data, Json::Value& value, std::string& errors);
bool parse(const std::vector<char>& data, Json::Value& value);

using SerializedString = std::string;

/**
 * @brief helper functions to convert between Json::Value and serialized string which uses markers to retain the type information
 * e.g.
 *   Json::Value("hello") and serialized string "\"hello\"\\n"
 *   Json::Value(true) and serialized string "true\\n"
 *   Json::Value(1234) and serialized string "1234\\n"
 */
Json::Value convertToJsonValue(const SerializedString& serializedStr);
SerializedString convertToTypedString(const Json::Value& jsonValue);

/**
 * Determine if the given text is a JSON string.  Blanks strings are not JSON.
 * @returns @p true only if the string can be parsed as valid JSON.
 */
bool isJson(const SerializedString& str);

}  // namespace jsonhelpers
