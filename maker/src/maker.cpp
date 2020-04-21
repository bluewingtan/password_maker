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

CLI11.h
CLI11 1.8 Copyright (c) 2017-2019 University of Cincinnati, developed by Henry
Schreiner under NSF AWARD 1414736. All rights reserved.

Redistribution and use in source and binary forms of CLI11, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
   may be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--*/
#include <version.h>
#include <maker.h>
#include <CLI11.h>

std::string help_content() {
	std::string retval;
	retval += std::string("Password Maker (v") + PASSWORD_MAKER_VERSION + " at " + CMAKE_COMPILE_DATE + " on " + CMAKE_SYSTEM + " " + CMAKE_SYSTEM_VERSION + ")\n";
	retval += std::string("Compiled with CMake ") + CMAKE_VERSION " and " + CMAKE_COMPILER + " " + CMAKE_COMPILER_VERSION + "\n\n";
	retval += std::string("A high-performance, customizable, cross-platform and modern C++ password dictionary generator\n\n");
	return retval;
}

struct ExistingFileDistValidator : public CLI::Validator {
	ExistingFileDistValidator() : Validator("DISTFILE") {
		func_ = [](std::string& filename) {
			std::string distFileName = "./config/" + filename;
			auto path_result = CLI::detail::check_path(distFileName.c_str());
			if (path_result == CLI::detail::path_type::nonexistant) {
				return "File does not exist: " + filename;
			}
			if (path_result == CLI::detail::path_type::directory) {
				return "File is actually a directory: " + filename;
			}
			return std::string();
		};
	}
};

int main(int argc, char** argv) {
	std::cout << help_content();

	CLI::App app{};
	std::string configFileName{ "config.json" };
	std::size_t threadNumber{ std::thread::hardware_concurrency() };
	ExistingFileDistValidator validator;

	app.get_formatter()->column_width(40);
	app.add_option("-c,--config", configFileName, "The configuration filename in ./config path", true)->check(validator);
	app.add_option("-t,--thread", threadNumber, "How many threads should be used to generate the password", true)->check(CLI::Range(1u, std::thread::hardware_concurrency()));

	CLI11_PARSE(app, argc, argv);

	bwt::PasswordMaker maker(configFileName, threadNumber);
	maker.generate();
	return 0;
}
