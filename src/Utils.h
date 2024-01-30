#pragma once

namespace inlineUtils
{
	inline void sendConsoleCommand(const std::string& a_command)
	{
		const auto scriptFactory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::Script>();
		if (const auto script = scriptFactory ? scriptFactory->Create() : nullptr) {
			const auto selectedRef = RE::Console::GetSelectedRef();
			script->SetCommand(a_command);
			script->CompileAndRun(selectedRef.get());
			delete script;
		}
	}
}