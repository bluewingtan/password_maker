#include <maker.h>
#include <CLI11.h>

int main(int argc, char** argv) {
	CLI::App app;

	CLI11_PARSE(app, argc, argv);

	bwt::PasswordMaker maker("config.json");
	maker.generate();
	return 0;
}
