#pragma once

/*++

Copyright Notice

MIT License

Copyright (C) 2020 BlueWingTan. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

ThreadPool.h
Copyright (c) 2012 Jakob Progsch, Václav Zeman

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.

json.h
MIT License

Copyright (c) 2013-2020 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

spdlog
The MIT License (MIT)

Copyright (c) 2016 Gabi Melman.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

-- NOTE: Third party dependecy used by this sofware --
This software depends on the fmt lib (MIT License),
and users must comply to its license: https://github.com/fmtlib/fmt/blob/master/LICENSE.rst

--*/

#include <ctime>
#include <cctype>

#include <map>
#include <mutex>
#include <regex>
#include <string>
#include <vector>
#include <chrono>
#include <memory>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <functional>

#include <json.h>
#include <ThreadPool.h> 
#include <spdlog/sinks/stdout_color_sinks.h>

namespace bwt {
// String literal
constexpr const char* CONFIG = "config";
constexpr const char* GENERATE_SEED = "generate_seed";
constexpr const char* FILE_SEED = "file_seed";
constexpr const char* GENERATE_RULE = "generate_rule";
constexpr const char* CAPITALIZE = "capitalize";
constexpr const char* FORMATION = "formation";
constexpr const char* CONTENT = "content";
constexpr const char* KEEP_IN_ORDER = "keep_in_order";
constexpr const char* TRANSFORM = "transform";
constexpr const char* ACTIVE = "active";
constexpr const char* RULES = "rules";
constexpr const char* GENERATE_FILTER = "generate_filter";
constexpr const char* MINIMUM_LENGTH = "minimum_length";
constexpr const char* OPTIONAL_FILTER = "optional";
constexpr const char* NUMBER = "number";
constexpr const char* LOWER_LETTER = "lower_letter";
constexpr const char* UPPER_LETTER = "upper_letter";
constexpr const char* SPECIAL_LETTER = "special_letter";
constexpr const char* ACHIEVE_OPTIONAL = "achieve_optional";
constexpr const char* GENERATE_ADDITIONAL = "generate_additional";

constexpr const char* DIST_PATH = "./dist/";
constexpr const char* CONFIG_PATH = "./config/";
constexpr const char* GENERATE_PATH = "./generated/";


class PasswordMaker {
public:
	/// <summary> Constructor. </summary>
	/// <remarks> BlueWingTan, 2020/4/16. </remarks>
	/// <param name="configFileName"> [in,out] Filename of the configuration file. </param>
	PasswordMaker(const std::string& configFileName, const std::size_t threadNumber = std::thread::hardware_concurrency()) :
		_configFileName(CONFIG_PATH + configFileName),
		_threadPool(threadNumber) {
		_workerLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [id:%6t] %v");
	}

	~PasswordMaker() {}

public:

	/// <summary> Generates password dictionary. </summary>
	/// <remarks> BlueWingTan, 2020/4/20. </remarks>
	/// <returns> True if it succeeds, false if it fails. </returns>
	bool generate() {
		_mainLogger->info("Loading configuration from {}.", _configFileName);
		if (!load_config()) {
			return false;
		}

		_mainLogger->info("Parsing configuration file to get generate formation.");
		const auto& multipleFormations = get_generate_formation();
		if (!check_generate_formation(multipleFormations)) {
			return false;
		}

		std::vector<std::string> results;

		_mainLogger->info("Getting serial file name.");
		get_serial_file_name();

		_mainLogger->info("Generating password with multiple thread, pool size are {}.", _threadPool.size());
		for (const auto& singleFormation : multipleFormations) {
			_mainLogger->info("Generating formation [{}].", std::accumulate(singleFormation.begin(), singleFormation.end(), std::string(""), [&](auto& lhs, const auto& rhs) {
				return lhs.empty() ? rhs : lhs + " " + rhs; }));
			results.clear();
			results.reserve(estimate_capacity(singleFormation));
			std::size_t currentIndex = 0;
			for (const auto& formation : singleFormation) {
				_mainLogger->info("Generating pattern [{}].", formation);
				auto contents(get_seed_content(formation));
				// Last loop turn to serial
				_mainLogger->info("Initiating worker processor.");
				map_to_processor(results, contents, (currentIndex++ == singleFormation.size() - 1));
			}
		}

		_mainLogger->info("Appendding additional dictionary.");
		append_additional_dictionary();

		_mainLogger->info("Done.");
		return true;
	}

private:
	std::mutex _serialLock;
	std::string _serialFileName;
	std::string _configFileName;
	nlohmann::json _configuration;
	std::shared_ptr<spdlog::logger> _mainLogger{ spdlog::stdout_color_mt("Main") };
	std::shared_ptr<spdlog::logger> _workerLogger{ spdlog::stdout_color_mt("Worker") };
	ThreadPool _threadPool;

	using attribure_t = struct {
		bool optNumber;
		bool optLowerLetter;
		bool optUpperLetter;
		bool optSpecialLetter;
		std::size_t achiveOptional;
		std::size_t minimumLength;
	};
	using string_array_t = std::vector<std::string>;

private:

	/// <summary> Processor. </summary>
	/// <remarks> BlueWingTan, 2020/4/21. </remarks>
	/// <param name="base">		    [in,out] The base. </param>
	/// <param name="addon">	    The addon. </param>
	/// <param name="shouldSerial"> True if should serial. </param>
	/// <returns> True if it succeeds, false if it fails. </returns>
	bool processor(string_array_t& base, const string_array_t& addon, const bool shouldSerial) {
		_workerLogger->info("Processing with base size {} and addon size {}.", base.size(), addon.size());
		auto generated(password_generate(base, addon));
		if (shouldSerial) {
			password_capitalize(generated);
			password_transform(generated);
			password_filter(generated);
			_workerLogger->info("Serializing generated password with size {}.", generated.size());
			password_serial(generated);
		}
		replace_from_to(generated, base);
		_workerLogger->info("Done.");
		return true;
	}

	/// <summary> Map to processor. </summary>
	/// <remarks> BlueWingTan, 2020/4/21. </remarks>
	/// <param name="alreadyGenerated"> [in,out] The already generated. </param>
	/// <param name="formationContent"> The formation content. </param>
	/// <param name="serialThisTurn">   True to serial this turn. </param>
	void map_to_processor(string_array_t& alreadyGenerated, const string_array_t& formationContent, const bool serialThisTurn) {
		const auto threadWokerNumber = _threadPool.size();
		// Multiple thread optimization
		// check alreadyGenerated and formationContent size
		// if the size less than threadWokerNumber
		// then should not start thread pool
		if (alreadyGenerated.size() < threadWokerNumber) {
			if (formationContent.size() < threadWokerNumber) {
				// Single thread to process
				processor(alreadyGenerated, formationContent, serialThisTurn);
			}
		} else {
			// Proceed the generated content
			const auto properLoad = alreadyGenerated.size() / threadWokerNumber;
			const auto extraLoad = alreadyGenerated.size() % threadWokerNumber;
			auto currentIt = alreadyGenerated.begin();
			std::vector<std::future<bool>> results;
			std::vector<string_array_t> managed;
			managed.reserve(threadWokerNumber);
			results.reserve(threadWokerNumber);
			for (size_t i = 0; i < threadWokerNumber - 1; i++, std::advance(currentIt, properLoad)) {
				managed.emplace_back(string_array_t(currentIt, currentIt + properLoad));
			}
			managed.emplace_back(string_array_t(currentIt, currentIt + properLoad + extraLoad));
			std::for_each(managed.begin(), managed.end(), [&](auto& generatedSlice) {
				results.emplace_back(_threadPool.enqueue(&PasswordMaker::processor, this, std::ref(generatedSlice), std::cref(formationContent), serialThisTurn)); });
			// Wait future
			std::for_each(results.begin(), results.end(), [](const auto& result) {result.wait(); });
			// Replace already generated
			replace_from_to(std::accumulate(managed.begin(), managed.end(), string_array_t(), [&](auto& lhs, auto& rhs) { std::copy(rhs.begin(), rhs.end(), std::back_inserter(lhs)); return lhs; }),
							alreadyGenerated);
		}
	}

	/// <summary> Loads the configuration. </summary>
	/// <remarks> BlueWingTan, 2020/4/17. </remarks>
	/// <returns> True if it succeeds, false if it fails. </returns>
	bool load_config() {
		try {
			std::fstream file(_configFileName);
			_configuration = nlohmann::json::parse(file);
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to open or parse configuration file from '{}' with {}.", _configFileName, ex.what());
			return false;
		}
		return true;
	}

	/// <summary> Get serialize file name with time suffix. </summary>
	/// <remarks> BlueWingTan, 2020/4/17. </remarks>
	inline void get_serial_file_name() {
		std::stringstream ss;
		try {
			const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			ss << std::put_time(std::localtime(&now), "%Y-%m-%d-%H-%M-%S.txt");
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to serialize the file time with {}.", ex.what());
		}
		_serialFileName = ss.str();
	}

	/// <summary>
	///		<para> Serials the given contents, should be invoked after get_serial_file_name(). </para>
	///		<para> This function is thread-safe by using std::lock_guard. </para>
	///	</summary>
	/// <remarks> BlueWingTan, 2020/4/20. </remarks>
	/// <param name="contents"> The contents. </param>
	/// <returns> True if it succeeds, false if it fails. </returns>
	bool password_serial(const string_array_t& contents) {
		// Make sure serial_safe_impl already returned
		// to prevent early extract contents
		std::lock_guard<std::mutex> lock(_serialLock);
		std::fstream file(GENERATE_PATH + _serialFileName, std::fstream::app | std::fstream::out);

		if (!file) {
			_mainLogger->critical("Failed to open {} to serialize.", GENERATE_PATH + _serialFileName);
			return false;
		} else {
			std::for_each(contents.cbegin(), contents.cend(), [&](auto& content) {file << content << std::endl; });
		}
		return true;
	}

	/// <summary> Replace from to. </summary>
	/// <remarks> BlueWingTan, 2020/4/19. </remarks>
	/// <param name="from"> Source for the. </param>
	/// <param name="to">   [in,out] to. </param>
	inline void replace_from_to(const string_array_t& from, string_array_t& to) const {
		to.clear();	// In standard, clear() do not release underlying buffers
		std::copy(from.cbegin(), from.cend(), std::back_inserter(to));
	}

	/// <summary> Estimate total vector capacity. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="formations"> The formations. </param>
	/// <returns> Estimated capacity. </returns>
	std::size_t estimate_capacity(const string_array_t& formations) const {
		std::vector<std::size_t> estimateSize;
		std::size_t singleFormationCapacity = 1;

		std::for_each(formations.cbegin(), formations.cend(), [&](const auto& content) {singleFormationCapacity *= get_seed_content(content).size(); });
		estimateSize.emplace_back(singleFormationCapacity);
		return *std::max_element(estimateSize.begin(), estimateSize.end());
	}

	/// <summary> Gets generate formation. </summary>
	/// <remarks> BlueWingTan, 2020/4/17. </remarks>
	/// <returns> The generate formation. </returns>
	std::vector<string_array_t> get_generate_formation() const {
		std::vector<string_array_t> multipleFormations;
		try {
			for (const auto& item : _configuration[CONFIG][GENERATE_RULE][FORMATION][CONTENT].get<string_array_t>()) {
				// Tokenize the formation content
				std::regex delimiter(R"(\s+)");
				auto singleFormation = string_array_t(
					std::sregex_token_iterator(item.cbegin(), item.cend(), delimiter, -1),
					std::sregex_token_iterator());

				if (_configuration[CONFIG][GENERATE_RULE][FORMATION][KEEP_IN_ORDER].get<bool>()) {
					multipleFormations.emplace_back(singleFormation);
				} else {
					// First we should arrange the formation
					std::vector<string_array_t> permutation;
					std::sort(singleFormation.begin(), singleFormation.end());
					do {
						permutation.emplace_back(singleFormation);
					} while (std::next_permutation(singleFormation.begin(), singleFormation.end()));

					// Then we should erase duplicates
					std::sort(permutation.begin(), permutation.end());
					permutation.erase(std::unique(permutation.begin(), permutation.end()), permutation.end());

					multipleFormations.insert(multipleFormations.end(), permutation.begin(), permutation.end());
				}
			}
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to parse configuration file for formation generating with {}.", ex.what());
		}

		return multipleFormations;
	}

	/// <summary> Confirm whether the formation is consistent with the definition in the configuration. </summary>
	/// <remarks> BlueWingTan, 2020/4/17. </remarks>
	/// <param name="formations"> The formations. </param>
	/// <returns> True if it succeeds, false if it fails. </returns>
	bool check_generate_formation(const std::vector<string_array_t>& formations) const {
		string_array_t legalSeeds;
		try {
			const auto& generateSeed = _configuration[CONFIG][GENERATE_SEED];
			for (const auto& generalSeed : generateSeed.items()) {
				if (generalSeed.key() != FILE_SEED) {
					legalSeeds.emplace_back(generalSeed.key());
				}
			}
			for (const auto& fileSeed : generateSeed[FILE_SEED].items()) {
				legalSeeds.emplace_back(fileSeed.key());
			}

			std::sort(legalSeeds.begin(), legalSeeds.end());
			// Check if it is a subset
			// Must deep copy to prevent change the origin vector
			for (auto singleFormation : formations) {
				std::sort(singleFormation.begin(), singleFormation.end());
				singleFormation.erase(std::unique(singleFormation.begin(), singleFormation.end()), singleFormation.end());
				if (!std::includes(legalSeeds.begin(), legalSeeds.end(), singleFormation.begin(), singleFormation.end())) {
					_mainLogger->critical("Formation is not in generate seeds.");
					return false;
				}
			}
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to parse configuration file for formation checking with {}.", ex.what());
			return false;
		}

		return true;
	}

	/// <summary> Gets seed content with given formation. </summary>
	/// <remarks> BlueWingTan, 2020/4/17. </remarks>
	/// <param name="formation"> The formation. </param>
	/// <returns> The seed content. </returns>
	string_array_t get_seed_content(const std::string& formation) const {
		string_array_t contents;
		try {
			auto fileSeeds = _configuration[CONFIG][GENERATE_SEED][FILE_SEED];
			auto fileSeedIndex = fileSeeds.find(formation);
			auto generalSeeds = _configuration[CONFIG][GENERATE_SEED];
			auto generalSeedIndex = generalSeeds.find(formation);

			if (fileSeedIndex != fileSeeds.end()) {
				// Current formation is file seed
				auto seedFileName = fileSeedIndex->get<std::string>();
				std::fstream file(DIST_PATH + seedFileName, std::fstream::in);
				if (file) {
					for (std::string line; std::getline(file, line);) {
						contents.emplace_back(line);
					}
				} else {
					_mainLogger->critical("Failed to open file for seed content acquire.");
				}
			} else if (generalSeedIndex != generalSeeds.end()) {
				// Current formation is general seed
				if (*generalSeedIndex == FILE_SEED) {
					_mainLogger->critical("file_seed is reversed key word that should not used.");
				} else {
					auto generalSeedContent = generalSeedIndex->get<string_array_t>();
					contents.insert(contents.end(), generalSeedContent.begin(), generalSeedContent.end());
				}
			} else {
				// If checked generate formation
				// never goto here
				_mainLogger->critical("Wild formation as {}, is check_generate_formation invoked?", formation);
			}
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to parse configuration file for seed content acquire with {}.", ex.what());
		}

		return contents;
	}

	/// <summary> Single password transformation by specified rules. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="password"> The password. </param>
	/// <param name="rules">    The rules. </param>
	/// <returns> Transformed password. </returns>
	std::string password_transform_single(const std::string& password, const std::map<std::string, std::string>& rules) const {
		std::string replaced = password;
		std::for_each(rules.cbegin(), rules.cend(), [&](const auto& rule) {std::regex_replace(replaced, std::regex(rule.first), rule.second); });
		return replaced;
	}

	/// <summary> Transform password with specified rules, and insert into the original container. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="passwords"> [in,out] The passwords. </param>
	void password_transform(string_array_t& passwords) const {
		try {
			const auto& transformConfig = _configuration[CONFIG][GENERATE_RULE][TRANSFORM];
			if (transformConfig[ACTIVE].get<bool>()) {
				// Activated
				const auto& rules = transformConfig[RULES].get<std::map<std::string, std::string>>();
				string_array_t transformedPasswords;

				std::for_each(passwords.begin(), passwords.end(), [&](const auto& password) {
					auto transformed(password_transform_single(password, rules));
					if (transformed != password) {
						transformedPasswords.emplace_back(transformed);
					}});
				std::copy(passwords.begin(), passwords.end(), std::back_inserter(transformedPasswords));
			}
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to parse configuration file for transforming passwords with {}.", ex.what());
		}
	}

	/// <summary> Weather password has specified attribute defined in configuration. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="password"> The password. </param>
	/// <returns> True if it password has specified attribute, false otherwise. </returns>
	bool password_has_achieved_attributes(const std::string& password, const attribure_t& attributes) const {
		using filter_result_t = struct {
			std::function<bool(std::string::value_type)> test;
			bool supposed;
			bool actual;
		};

		std::vector<filter_result_t> attributeTests{
			{[&](std::string::value_type ch) { return std::islower(ch) != 0; }, attributes.optLowerLetter, false},
			{[&](std::string::value_type ch) { return std::isupper(ch) != 0; }, attributes.optUpperLetter, false},
			{[&](std::string::value_type ch) { return std::isdigit(ch) != 0; }, attributes.optNumber, false},
			{[&](std::string::value_type ch) {
				const auto& specialLetters = get_seed_content(SPECIAL_LETTER);
				std::for_each(specialLetters.cbegin(), specialLetters.cend(),
							  [&](const auto& letter) { if (letter.find(ch)) { return true; } return false; }); return false; },
			attributes.optSpecialLetter, false }
		};

		// Now we check it
		std::for_each(password.cbegin(), password.cend(), [&](const auto& ch) {
			std::for_each(attributeTests.begin(), attributeTests.end(), [&](auto& result) {
				if (!result.actual) { result.actual = result.test(ch); }}); });

		// Meet the request
		auto achievedOptional = std::accumulate(attributeTests.begin(), attributeTests.end(), std::size_t(0), [](const auto& meets, const auto& attribute) {
			return meets + (attribute.supposed == attribute.actual); });

		return ((achievedOptional >= attributes.achiveOptional) && (password.length() >= attributes.minimumLength));
	}

	/// <summary> Password attributeConfig. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="passwords"> [in, out] The passwords. </param>
	void password_filter(string_array_t& passwords) const {
		try {
			const auto& attributeConfig = _configuration[CONFIG][GENERATE_FILTER];
			attribure_t attributes{
				attributeConfig[OPTIONAL_FILTER][NUMBER].get<bool>(),
				attributeConfig[OPTIONAL_FILTER][LOWER_LETTER].get<bool>(),
				attributeConfig[OPTIONAL_FILTER][UPPER_LETTER].get<bool>(),
				attributeConfig[OPTIONAL_FILTER][SPECIAL_LETTER].get<bool>(),
				attributeConfig[ACHIEVE_OPTIONAL].get<std::size_t>(),
				attributeConfig[MINIMUM_LENGTH].get<std::size_t>()
			};
			passwords.erase(std::remove_if(passwords.begin(), passwords.end(), [&](const auto& password) {
				return !password_has_achieved_attributes(password, attributes); }), passwords.end());
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to parse configuration file for password filtering with {}.", ex.what());
		}
	}

	/// <summary> Password capitalize. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="passwords"> [in,out] The passwords. </param>
	void password_capitalize(string_array_t& passwords) const {
		try {
			if (_configuration[CONFIG][GENERATE_RULE][CAPITALIZE].get<bool>()) {
				std::for_each(passwords.begin(), passwords.end(), [&](auto& password) {password[0] = std::toupper(password[0]); });
			}
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to parse configuration file for capitalize password with {}.", ex.what());
		}
	}

	/// <summary> Password generate. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="base">  [in,out] The base. </param>
	/// <param name="addon"> The addon. </param>
	string_array_t password_generate(string_array_t& base, const string_array_t& addon) const {
		string_array_t generated;
		generated.reserve(std::max(base.size(), std::size_t(1)) * addon.size());

		std::for_each(addon.cbegin(), addon.cend(), [&](const auto& addonSingle) {
			if (base.empty()) {
				generated.emplace_back(addonSingle);
			} else {
				std::transform(base.begin(), base.end(), std::back_inserter(generated), [&](const auto& baseSingle) {
					return baseSingle + addonSingle; });
			}});

		return generated;
	}

	/// <summary> Appends the additional dictionary. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	void append_additional_dictionary() {
		try {
			for (const auto& additionalDict : _configuration[CONFIG][GENERATE_ADDITIONAL].get<string_array_t>()) {
				std::fstream file(DIST_PATH + additionalDict);
				if (file) {
					string_array_t content;
					std::copy(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), std::back_inserter(content));
					password_serial(content);
				}
			}
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to parse configuration file for append additional dictionary with {}.", ex.what());
		}
	}
};	// class PasswordMaker
}	// namespace bwt