#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EDroneState : uint8
{
	Air,
	Ground,
	Crash
};