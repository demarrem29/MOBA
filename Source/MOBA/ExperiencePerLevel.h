#pragma once
#include "Engine/DataTable.h"
#include "ExperiencePerLevel.generated.h"

USTRUCT(BlueprintType)
struct FExperiencePerLevel : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
public:

	FExperiencePerLevel()
		: Level(0)
		, Experience(0)
	{}
	/** Current Level **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Level;
	/** Experience Required to Level Up **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float Experience;
};