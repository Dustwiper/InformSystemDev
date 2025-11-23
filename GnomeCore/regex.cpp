#include "regex.h"

namespace regExpr {
	const std::regex strPattern { R"(^[a-z]+$)" };
	const std::regex arrayPattern { R"(^-?\d+(?:,\s*-?\d+)*$)" };
	const std::regex intPattern{ R"((?:-?\d+))" };
}