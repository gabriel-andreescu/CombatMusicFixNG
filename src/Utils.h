#pragma once

namespace inlineUtils
{
	inline void sendConsoleCommand(std::string_view a_command)
	{
		if (auto* scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>()) {
			if (auto* script = scriptFactory->Create()) {
				if (const auto selectedRef = RE::Console::GetSelectedRef()) {
					script->SetCommand(a_command);
					script->CompileAndRun(selectedRef.get());
				}
				delete script;
			}
		}
	}
}
