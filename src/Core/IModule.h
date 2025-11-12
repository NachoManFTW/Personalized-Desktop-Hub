#pragma once

class IModule
{
public:
	virtual void Init() = 0;
	virtual void Update(float dt) = 0;
	virtual void Render() = 0;
	virtual void Shutdown() = 0;

	virtual const char* GetName() const = 0;
	virtual ~IModule() = default;
};
