#pragma once

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
constexpr char* CONFIG = "config";
constexpr char* GENERATE_SEED = "generate_seed";
constexpr char* FILE_SEED = "file_seed";
constexpr char* GENERATE_RULE = "generate_rule";
constexpr char* CAPITALIZE = "capitalize";
constexpr char* FORMATION = "formation";
constexpr char* CONTENT = "content";
constexpr char* KEEP_IN_ORDER = "keep_in_order";
constexpr char* TRANSFORM = "transform";
constexpr char* ACTIVE = "active";
constexpr char* RULES = "rules";
constexpr char* GENERATE_FILTER = "generate_filter";
constexpr char* MINIMUM_LENGTH = "minimum_length";
constexpr char* OPTIONAL_FILTER = "optional";
constexpr char* NUMBER = "number";
constexpr char* LOWER_LETTER = "lower_letter";
constexpr char* UPPER_LETTER = "upper_letter";
constexpr char* SPECIAL_LETTER = "special_letter";
constexpr char* ACHIEVE_OPTIONAL = "achieve_optional";
constexpr char* GENERATE_ADDITIONAL = "generate_additional";

constexpr char* DIST_PATH = "./dist/";
constexpr char* CONFIG_PATH = "./config/";
constexpr char* GENERATE_PATH = "./generated/";


class PasswordMaker {
public:
	/// <summary> Constructor. </summary>
	/// <remarks> BlueWingTan, 2020/4/16. </remarks>
	/// <param name="configFileName"> [in,out] Filename of the configuration file. </param>
	PasswordMaker(const std::string& configFileName) :
		_configFileName(CONFIG_PATH + configFileName) {
		_workerLogger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%^%l%$] [id:%6t] %v");
	}

	~PasswordMaker() {}

public:

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

		_mainLogger->info("Generating password with multiple thread, pool size are {}.", std::thread::hardware_concurrency());
		for (const auto& singleFormation : multipleFormations) {
			_mainLogger->info("Generating formation [{}].", std::accumulate(singleFormation.begin(), singleFormation.end(), std::string(""), [&](auto& lhs, const auto& rhs) {
				return lhs.empty() ? rhs : lhs + " " + rhs; }));
			results.clear();
			results.reserve(estimate_capacity(singleFormation));
			std::size_t currentIndex = 0;
			for (const auto& formation : singleFormation) {
				_mainLogger->info("Generating pattern [{}].", formation);
				auto& contents = get_seed_content(formation);
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
	std::mutex _appedLock;
	std::string _serialFileName;
	std::string _configFileName;
	nlohmann::json _configuration;
	std::shared_ptr<spdlog::logger> _mainLogger{ spdlog::stdout_color_mt("Main") };
	std::shared_ptr<spdlog::logger> _workerLogger{ spdlog::stdout_color_mt("Worker") };
	ThreadPool _threadPool{ std::thread::hardware_concurrency() };

	using attribure_t = struct {
		bool optNumber;
		bool optLowerLetter;
		bool optUpperLetter;
		bool optSpecialLetter;
		std::size_t achiveOptional;
		std::size_t minimumLength;
	};

private:

	/// <summary> Processor. </summary>
	/// <remarks> BlueWingTan, 2020/4/19. </remarks>
	/// <param name="base">		    [in,out] The base. </param>
	/// <param name="addon">	    The addon. </param>
	/// <param name="shouldSerial"> True if should serial. </param>
	/// <returns> True if it succeeds, false if it fails. </returns>
	bool processor(std::vector<std::string>& base, const std::vector<std::string>& addon, bool shouldSerial) {
		_workerLogger->info("Processing with base size {} and addon size {}.", base.size(), addon.size());
		auto& generated = password_generate(base, addon);
		password_capitalize(generated);
		password_transform(generated);
		password_filter(generated);
		if (shouldSerial) {
			_workerLogger->info("Serializing generated password with size {}.", generated.size());
			serial(generated);
		}
		replace_from_to(generated, base);
		_workerLogger->info("Done.");
		return true;
	}

	/// <summary> Map to processor. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="alreadyGenerated"> [in,out] The already generated. </param>
	/// <param name="formationContent"> The formation content. </param>
	/// <param name="serialThisTurn">   True to serial this turn. </param>
	void map_to_processor(std::vector<std::string>& alreadyGenerated, const std::vector<std::string>& formationContent, bool serialThisTurn) {
		// Multiple thread optimization
		// check alreadyGenerated and formationContent size
		// if the size less than std::thread::hardware_concurrency()
		// then should not start thread pool
		if (alreadyGenerated.size() < std::thread::hardware_concurrency()) {
			if (formationContent.size() < std::thread::hardware_concurrency()) {
				// Single thread to process
				processor(alreadyGenerated, formationContent, serialThisTurn);
			} else {
				// Proceed the formation content
				auto properLoad = formationContent.size() / std::thread::hardware_concurrency();
				auto extraLoad = formationContent.size() % std::thread::hardware_concurrency();
				auto currentIt = formationContent.cbegin();
				std::vector<std::future<bool>> results;
				results.reserve(std::thread::hardware_concurrency());
				for (std::size_t i = 0; i < std::thread::hardware_concurrency() - 1; i++, std::advance(currentIt, properLoad)) {
					results.emplace_back(_threadPool.enqueue(&PasswordMaker::processor, this, std::ref(alreadyGenerated), std::vector<std::string>(currentIt, currentIt + properLoad), serialThisTurn));
				}
				results.emplace_back(_threadPool.enqueue(&PasswordMaker::processor, this, std::ref(alreadyGenerated), std::vector<std::string>(currentIt, currentIt + properLoad + extraLoad), serialThisTurn));
				// Wait future
				std::for_each(results.begin(), results.end(), [](const auto& result) {result.wait(); });
			}
		} else {
			// Proceed the generated content
			auto properLoad = alreadyGenerated.size() / std::thread::hardware_concurrency();
			auto extraLoad = alreadyGenerated.size() % std::thread::hardware_concurrency();
			auto currentIt = alreadyGenerated.cbegin();
			std::vector<std::future<bool>> results;
			std::vector<std::vector<std::string>> managed;
			managed.reserve(std::thread::hardware_concurrency());
			results.reserve(std::thread::hardware_concurrency());
			for (size_t i = 0; i < std::thread::hardware_concurrency() - 1; i++, std::advance(currentIt, properLoad)) {
				managed.emplace_back(std::vector<std::string>(currentIt, currentIt + properLoad));
			}
			managed.emplace_back(std::vector<std::string>(currentIt, currentIt + properLoad + extraLoad));
			std::for_each(managed.begin(), managed.end(), [&](auto& generatedSlice) {
				results.emplace_back(_threadPool.enqueue(&PasswordMaker::processor, this, std::ref(generatedSlice), std::cref(formationContent), serialThisTurn)); });
			// Wait future
			std::for_each(results.begin(), results.end(), [](const auto& result) {result.wait(); });
			// TODO
			// Optimization
			alreadyGenerated = std::accumulate(managed.begin(), managed.end(), std::vector<std::string>(), [&](auto& lhs, auto& rhs) { std::copy(rhs.begin(), rhs.end(), std::back_inserter(lhs)); return lhs; });
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
			const auto nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			ss << std::put_time(std::localtime(&nowTime), "%Y-%m-%d-%H-%M-%S.txt");
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to serialize the file time with {}.", ex.what());
		}
		_serialFileName = ss.str();
	}

	/// <summary> Serial safe implementation. </summary>
	/// <remarks> BlueWingTan, 2020/4/19. </remarks>
	/// <param name="contents"> The contents. </param>
	/// <returns> True if it succeeds, false if it fails. </returns>
	bool serial_safe_impl(const std::vector<std::string>& contents) {
		std::lock_guard<std::mutex> lock(_serialLock);
		std::fstream file(GENERATE_PATH + _serialFileName, std::fstream::app | std::fstream::out);

		if (!file) {
			_mainLogger->critical("Failed to open {} to serialize.", GENERATE_PATH + _serialFileName);
			return false;
		} else {
			std::for_each(contents.begin(), contents.end(), [&](auto& content) {file << content << std::endl; });
		}
		return true;
	}

	/// <summary>
	///		<para> Serials the given contents, should be invoked after get_serial_file_name(). </para>
	///		<para> This function is thread-safe by using std::lock_guard. </para>
	///	</summary>
	/// <remarks> BlueWingTan, 2020/4/17. </remarks>
	/// <param name="contents"> The contents. </param>
	/// <returns> True if it succeeds, false if it fails. </returns>
	bool serial(const std::vector<std::string>& contents) {
		// Make sure serial_safe_impl already returned
		// to prevent early extract contents
		return serial_safe_impl(contents);
	}

	/// <summary> Replace from to. </summary>
	/// <remarks> BlueWingTan, 2020/4/19. </remarks>
	/// <param name="from"> Source for the. </param>
	/// <param name="to">   [in,out] to. </param>
	inline void replace_from_to(const std::vector<std::string>& from, std::vector<std::string>& to) {
		to.clear();	// In standard, clear() do not release underlying buffers
		std::copy(from.begin(), from.end(), std::back_inserter(to));
	}

	/// <summary> Estimate total vector capacity. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="formations"> The formations. </param>
	/// <returns> Estimated capacity. </returns>
	std::size_t estimate_capacity(const std::vector<std::string>& formations) const {
		std::vector<std::size_t> estimateSize;
		std::size_t singleFormationCapacity = 1;

		for (const auto& content : formations) {
			singleFormationCapacity *= get_seed_content(content).size();
		}
		estimateSize.emplace_back(singleFormationCapacity);
		return *std::max_element(estimateSize.begin(), estimateSize.end());
	}

	/// <summary> Gets generate formation. </summary>
	/// <remarks> BlueWingTan, 2020/4/17. </remarks>
	/// <returns> The generate formation. </returns>
	std::vector<std::vector<std::string>> get_generate_formation() const {
		std::vector<std::vector<std::string>> multipleFormations;
		try {
			for (const auto& item : _configuration[CONFIG][GENERATE_RULE][FORMATION][CONTENT].get<std::vector<std::string>>()) {
				// Tokenize the formation content
				std::regex delimiter(R"(\s+)");
				auto singleFormation = std::vector<std::string>(
					std::sregex_token_iterator(item.begin(), item.end(), delimiter, -1),
					std::sregex_token_iterator());

				if (_configuration[CONFIG][GENERATE_RULE][FORMATION][KEEP_IN_ORDER].get<bool>()) {
					multipleFormations.emplace_back(singleFormation);
				} else {
					// First we should arrange the formation
					std::vector<std::vector<std::string>> permutation;
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
	bool check_generate_formation(const std::vector<std::vector<std::string>>& formations) const {
		std::vector<std::string> legalSeeds;
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
	std::vector<std::string> get_seed_content(const std::string& formation) const {
		std::vector<std::string> contents;
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
					auto generalSeedContent = generalSeedIndex->get<std::vector<std::string>>();
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
		std::for_each(rules.begin(), rules.end(), [&](const auto& rule) {std::regex_replace(replaced, std::regex(rule.first), rule.second); });
		return replaced;
	}

	/// <summary> Transform password with specified rules, and insert into the original container. </summary>
	/// <remarks> BlueWingTan, 2020/4/18. </remarks>
	/// <param name="passwords"> [in,out] The passwords. </param>
	void password_transform(std::vector<std::string>& passwords) const {
		try {
			const auto& transformConfig = _configuration[CONFIG][GENERATE_RULE][TRANSFORM];
			if (transformConfig[ACTIVE].get<bool>()) {
				// Activated
				const auto& rules = transformConfig[RULES].get<std::map<std::string, std::string>>();
				std::vector<std::string> transformedPasswords;

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
				std::for_each(specialLetters.begin(), specialLetters.end(),
							  [&](const auto& letter) { if (letter.find(ch)) { return true; } return false; }); return false; },
			attributes.optSpecialLetter, false }
		};

		// Now we check it
		std::for_each(password.begin(), password.end(), [&](const auto& ch) {
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
	void password_filter(std::vector<std::string>& passwords) const {
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
	void password_capitalize(std::vector<std::string>& passwords) const {
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
	std::vector<std::string> password_generate(std::vector<std::string>& base, const std::vector<std::string>& addon) {
		std::vector<std::string> generated;
		generated.reserve(std::max(base.size(), std::size_t(1)) * addon.size());

		std::for_each(addon.begin(), addon.end(), [&](const auto& addonSingle) {
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
			for (const auto& additionalDict : _configuration[CONFIG][GENERATE_ADDITIONAL].get<std::vector<std::string>>()) {
				std::fstream file(DIST_PATH + additionalDict);
				if (file) {
					std::vector<std::string> content;
					std::copy(std::istream_iterator<std::string>(file), std::istream_iterator<std::string>(), std::back_inserter(content));
					serial(content);
				}
			}
		} catch (const std::exception& ex) {
			_mainLogger->critical("Failed to parse configuration file for append additional dictionary with {}.", ex.what());
		}
	}
};	// class PasswordMaker
}	// namespace bwt